/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 14:54:01 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/19 19:20:52 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Cgi.hpp"
#include "Init.hpp"
#include "Multipart.hpp"

int Server::_createServerSocket(int port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
		return -1;

	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	fcntl(fd, F_SETFL, O_NONBLOCK);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		close(fd);
		return -1;
	}
	if (listen(fd, LISTEN_BACKLOG) == -1)
	{
		close(fd);
		return -1;
	}
	return fd;
}

void Server::_acceptNewConnection(int listeningIndex)
{
	int serverFd = _fds[listeningIndex].fd;
	struct sockaddr_in clientAddr;
	socklen_t size = sizeof(clientAddr);

	int clientFd = accept(serverFd, (struct sockaddr *)&clientAddr, &size);
	if (clientFd < 0)
		return;

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
	{
		close(clientFd);
		return;
	}

	int clientIndex = -1;
	for (int i = _nbListeningSockets; i < MAX_TOTAL_FDS + _nbListeningSockets; i++)
	{
		if (_fds[i].fd == -1)
		{
			clientIndex = i;
			break;
		}
	}

	if (clientIndex != -1)
	{
		_fds[clientIndex].fd = clientFd;
		_fds[clientIndex].events = POLLIN;
		_fds[clientIndex].revents = 0;

		int port = _serverSockets[serverFd];
		_clients[clientFd] = Client(clientFd, port);
		std::cout << "[CONNEXION] FD: " << clientFd << " on Port: " << port << std::endl;
	}
	else
	{
		close(clientFd);
	}
}

void Server::_handleClientActivity(int i)
{
	int fd = _fds[i].fd;
	if (_clients.find(fd) == _clients.end())
		return;

	Client &client = _clients[fd];
	int clientPort = client.getServerPort();
	cgi &cgiClient = client.cgiClient;

	ServerConfig &currentConfig = _configs[0];

	for (size_t j = 0; j < _configs.size(); j++)
	{
		if (_configs[j]._listen[0].port == clientPort)
		{
			currentConfig = _configs[j];
			break;
		}
	}

	try
	{
		request request;
		response response;
		cgi cgi; //to delete + delete in init
		
		// STEP 0 : INIT
		initMain(request, response, cgi);
		
		// STEP 1 : PARSING CONFIG
		serverT mateConf = _convertConfig(currentConfig);

		if (_fds[i].revents & POLLIN) //READ OK
		{
			// STEP 2 : RECUPERE REQUEST REQUEST
			if (!client.requestComplete)
			client.requestLine(request);

			// STEP 3 : REQUEST PARSING (need it to know if CGI)
			if (client.requestComplete)
				requestMainNew(request, mateConf, response);

			// STEP 4 : CGI ?
			if (isCgi(request, cgiClient, response, mateConf) == true)
			{
				std::cout << "[INFO] Is CGI " << std::endl;
				Multipart m; //to delete
				int value = accessCgi(cgiClient); //check access cgi
				if (value == 0)
				{
					handleCgi(request, cgiClient, mateConf, response, m);
					if (cgiClient.pid == -1)
					{
						if (pipe(cgiClient.writePipe) == -1 || pipe(cgiClient.readPipe) == -1)
						{
							// if (cgiClient.writePipe)
							{
								close(cgiClient.writePipe[0]);
								close(cgiClient.writePipe[1]);
							}
							// if (cgiClient.readPipe)
							{
								close(cgiClient.readPipe[0]);
								close(cgiClient.readPipe[1]);
							}
							throw std::runtime_error("Pipe error");
						}

						cgiClient.pid = fork();

						if (cgiClient.pid == -1)
						{
							close(cgiClient.writePipe[0]);
							close(cgiClient.writePipe[1]);
							close(cgiClient.readPipe[0]);
							close(cgiClient.readPipe[1]);
							throw std::runtime_error("Fork error");
						}
					

						else if (cgiClient.pid == 0)
						{
							close(cgiClient.writePipe[1]);
							close(cgiClient.readPipe[0]);
							
							dup2(cgiClient.writePipe[0], STDIN_FILENO);
							dup2(cgiClient.readPipe[1], STDOUT_FILENO);

							close(cgiClient.writePipe[0]);
							close(cgiClient.readPipe[1]);

							char *args[] = {const_cast<char *>(cgiClient.binaryPath.c_str()), const_cast<char *>(cgiClient.scriptPath.c_str()), NULL};

							std::vector<std::string> env;
							env.push_back("REQUEST_METHOD=" + cgiClient.method);
							env.push_back("SCRIPT_FILENAME=" + cgiClient.scriptPath);
							env.push_back("QUERY_STRING=" + cgiClient.queryString);
							env.push_back("CONTENT_TYPE=" + cgiClient.contentType);
							env.push_back("CONTENT_LENGTH=" + cgiClient.contentLenght);
							env.push_back("GATEWAY_INTERFACE=CGI/1.1");
							env.push_back("SERVER_PROTOCOL=" + cgiClient.serverProtocol);
							env.push_back("SERVER_NAME=" + cgiClient.serverName);
							env.push_back("SERVER_PORT=" + cgiClient.serverPort);
							env.push_back("REDIRECT_STATUS=" + cgiClient.code);

							std::vector<char *> envp;
							for (size_t i = 0; i < env.size(); i++)
								envp.push_back(const_cast<char *>(env[i].c_str()));
							envp.push_back(NULL);

							std::string dir = cgiClient.scriptPath; // go in repertory of script
							size_t pos = dir.rfind("/");
							dir = dir.substr(0, pos + 1);
							if (chdir(dir.c_str()) == -1)
							{
								; // ICI GERER LE CAS ECHEC
							}
							// std::cout << dir << std::endl;

							execve(cgiClient.binaryPath.c_str(), args, envp.data());
						}
						else //parent
						{
							close(cgiClient.writePipe[0]);
							close(cgiClient.readPipe[1]);
							cgiClient.writing = true;
							cgiClient.reading = true;
							cgiClient.writeBuffer = cgiClient.body; // corps à envoyer au CGI
							cgiClient.readBuffer.clear();
						}
					}
				}
				if (cgiClient.isCgi && cgiClient.reading) 
				{
					char buffer[4096];
					ssize_t n = read(cgiClient.readPipe[0], buffer, sizeof(buffer));
					if (n > 0)
						cgiClient.readBuffer.append(buffer, n);
					else if (n == 0) // EOF
					{
						cgiClient.reading = false;
						close(cgiClient.readPipe[0]);
						close(cgiClient.writePipe[1]);
						parsStdout(cgiClient);
						buildCgiResponse(cgiClient, response);
						client.writeBuffer = cgiClient.readBuffer;
						client.isReadyToWrite = true;
					}
				}
				else 
				{
					if (value == 404)
						errorCode(response, mateConf, 404);
					if (value == 403)
						errorCode(response, mateConf, 403);
				}
				if (response.infos.error == true)
				{
					responseMain(request, response);
					client.writeBuffer = response.response;
					client.isReadyToWrite = true;
					std::cout << "[INFO] Response generated. Size " << client.writeBuffer.size() << " bytes." << std::endl;
				}
			}
			// STEP 5 : STATIC METHOD : GET POST DELETE
			if (cgiClient.isCgi == false && response.infos.error == false)
				client.handleRead(mateConf, request, response, cgiClient);
			
		}

		if ((_fds[i].revents & POLLOUT) && client.isReadyToWrite) //WRITE OK
		{
			if (cgiClient.isCgi == true && cgiClient.writing == true)
			{
                ssize_t n = write(cgiClient.writePipe[1], cgiClient.writeBuffer.c_str(), std::min(cgiClient.writeBuffer.size(), static_cast<size_t>(4096)));
                if (n > 0)
                {
                    cgiClient.writeBuffer.erase(0, n);
                    if (cgiClient.writeBuffer.empty())
                        cgiClient.writing = false;
                }
            }
			client.handleWrite();
		}
		if (client.isReadyToWrite)
			_fds[i].events = POLLIN | POLLOUT;
		else
			_fds[i].events = POLLIN;
	}
	catch (std::exception &e)
	{
		std::cerr << "[INFO] Client error: " << e.what() << " (FD: " << fd << ")" << std::endl;
		_closeConnection(i);
	}
}

void Server::_closeConnection(int index)
{
	int fd = _fds[index].fd;
	if (_clients.count(fd))
	{
		_clients.erase(fd);
	}
	close(fd);
	_fds[index].fd = -1;
	_fds[index].events = 0;
	_fds[index].revents = 0;
}

serverT Server::_convertConfig(const ServerConfig &myConfig)
{
	serverT newConfig;

	if (!myConfig._listen.empty())
	{
		newConfig.listens = myConfig._listen;
		newConfig.listen = myConfig._listen[0].port;
	}
	else
	{
		newConfig.listens.push_back(Listen("0.0.0.0", 8080));
		newConfig.listen = 8080;
	}

	newConfig.root = myConfig._root;
	newConfig.clientMaxBodySize = myConfig._clientMaxBodySize;
	newConfig.errorPage = myConfig._errorPages;

	for (size_t i = 0; i < myConfig._locations.size(); i++)
	{
		locationsT newLoc;
		const LocationConfig &curr = myConfig._locations[i];

		newLoc.path = curr._path;
		newLoc.upload_dir = curr._uploadPath;
		newLoc.index = (!curr._index.empty()) ? curr._index[0] : "";
		newLoc.autoindex = (curr._autoIndex == 1) ? "on" : "off";
		newLoc.cgiBinary = curr._cgiBinary;
		newLoc.cgiExtension = curr._cgiExtension;

		if (curr._allowGet)
			newLoc.methods.push_back("GET");
		if (curr._allowPost)
			newLoc.methods.push_back("POST");
		if (curr._allowDelete)
			newLoc.methods.push_back("DELETE");

		newConfig.locations[curr._path] = newLoc;
	}
	return newConfig;
}