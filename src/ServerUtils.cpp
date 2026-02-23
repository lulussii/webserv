/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 14:54:01 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/23 16:52:02 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Config.hpp"
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
	for (int i = _nbListeningSockets; i < MAX_TOTAL_FDS; i++)
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
	serverT *confPtr = &_refinedConfigs[0];

	bool	found = false;
	for (size_t j = 0; j < _refinedConfigs.size(); j++)
	{
		for(size_t k = 0; (k < _refinedConfigs[j].listens.size()); k++)
		{
			if (_refinedConfigs[j].listens[k].port == clientPort)
			{
				confPtr = &_refinedConfigs[j];
				found = true;
				break;
			}
		}
		if (found)
			break;
	}

	serverT	&currentConfig = *confPtr;
	currentConfig.listen = clientPort;
	
	try
	{
		request request;
		response response;

		// STEP 0 : INIT
		initMain(request, response);

		if (_fds[i].revents & POLLIN) // READ OK
		{
			// STEP 1 : RECUPERE REQUEST REQUEST
			if (!client.requestComplete && response.infos.error == false)
				client.requestLine(request);


			// STEP 2 : REQUEST PARSING (need it to know if CGI)
			if (client.requestComplete && response.infos.error == false)
			{
				std::cout << "[INFO] Request complete. Processing..." << std::endl;
				requestMainNew(request, currentConfig, response);
			}

			// STEP 3 : CGI
			if (client.requestComplete && isCgi(request, cgiClient, response, currentConfig) == true && response.infos.error == false)
			{
				std::cout << "[INFO] Is CGI " << std::endl;
				if (request._method == "DELETE")
					errorCode(response, currentConfig, 405);
				int value = accessCgi(cgiClient);
				if (value == 0 && response.infos.error == false) // check access cgi
				{
					if (cgiClient.pid == -1)
					{
						handleCgi(request, cgiClient, currentConfig, response);
						forkCgi(cgiClient, client);
					}
				}
				else
				{
					if (value == 404)
						errorCode(response, currentConfig, 404);
					if (value == 403)
						errorCode(response, currentConfig, 403);
				}
			}

			// STEP 4 : STATIC METHOD : GET POST DELETE
			else if (cgiClient.isCgi == false && response.infos.error == false)
				client.handleRead(currentConfig, request, response);

			// STEP 5 : build error response
			if (response.infos.error == true)
			{
				responseMain(request, response);
				client.writeBuffer = response.response;
				client.isReadyToWrite = true;
			}
		}

		if ((_fds[i].revents & POLLOUT) && client.isReadyToWrite) // WRITE OK
			client.handleWrite();
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