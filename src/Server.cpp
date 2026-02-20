/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 11:32:08 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/20 19:04:42 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cstring>
#include "Server.hpp"
#include "Client.hpp"

void handle_sigint(int sig)
{
	(void)sig;
	server_run = false;
}

Server::Server(const std::vector<ServerConfig> &configs) : _nbListeningSockets(0), _configs(configs)
{
	std::memset(_fds, 0, sizeof(_fds));

	for (int i = 0; i < MAX_TOTAL_FDS; i++)
	{
		_fds[i].fd = -1;
	}
}

Server::~Server()
{
	for (int i = 0; i < MAX_TOTAL_FDS; i++)
	{
		if (_fds[i].fd >= 0)
			close(_fds[i].fd);
	}
	_clients.clear();
	std::cout << "[INFO] All resources cleared" << std::endl;
}

void Server::_initRefinedConfigs()
{
	this->_refinedConfigs.clear();
	for (size_t i = 0; i < _configs.size(); i++)
	{
		this->_refinedConfigs.push_back(_convertConfig(_configs[i]));
	}
}

void Server::setup()
{
	_initRefinedConfigs();
	std::vector<int> openPorts;
	int fdsIndex = 0;

	for (size_t i = 0; i < _configs.size(); i++)
	{
		for (size_t j = 0; j < _configs[i]._listen.size(); j++)
		{
			int currentPort = _configs[i]._listen[j].port;

			bool alreadyOpen = false;
			for (size_t k = 0; k < openPorts.size(); k++)
			{
				if (openPorts[k] == currentPort)
				{
					alreadyOpen = true;
					break;
				}
			}
			if (alreadyOpen)
				continue;

			int fd = _createServerSocket(currentPort);
			if (fd == -1)
			{
				std::cerr << "[WARNING] Port " << currentPort << " busy. Skipping." << std::endl;
				continue;
			}

			_fds[fdsIndex].fd = fd;
			_fds[fdsIndex].events = POLLIN;
			_fds[fdsIndex].revents = 0;
			_serverSockets[fd] = currentPort;
			openPorts.push_back(currentPort);
			fdsIndex++;

			std::cout << "[INFO] Listening on port " << currentPort << std::endl;
		}
	}
	_nbListeningSockets = fdsIndex;

	if (_nbListeningSockets == 0)
		throw std::runtime_error("Fatal: No ports available.");
}

// new

// void Server::handleCgiRead(int fd)
// {
// 	responseT response;
//     cgi *cgiClient = cgiReadMap[fd];

//     char buffer[4096];

//     ssize_t n = read(fd, buffer, sizeof(buffer));

//     if (n > 0)
//         cgiClient->response.append(buffer, n);
//     else if (n == 0) //EOF
//     {
//         close(fd);

//         removeFdFromPoll(fd);

//         cgiReadMap.erase(fd);

//         cgiClient->reading = false;

//         parsStdout(*cgiClient);

//         buildCgiResponse(*cgiClient, response);

// 		if (_clients.count(cgiClient->clientFd) == 0)
//     		return;

//         Client &client = _clients[cgiClient->clientFd];

//         client.writeBuffer = response.response;

//         client.isReadyToWrite = true;

// 		for (int i = 0; i < MAX_TOTAL_FDS; i++)
// 		{
// 			if (_fds[i].fd == client.fd)
// 			{
// 				_fds[i].events |= POLLOUT;
// 				break;
// 			}
// 		}
//     }
// 	else if (n == -1 && errno != EAGAIN) // DELETE jsut to test
// 	{
// 		perror("read CGI pipe");
//         close(fd);
//         removeFdFromPoll(fd);
//         cgiReadMap.erase(fd);
//         cgiClient->reading = false;
// 	}
// }

// void Server::handleCgiWrite(int fd)
// {
//     cgi *cgiClient = cgiWriteMap[fd];

//     if (cgiClient->writeBuffer.empty())
//     {
// 		_fds[fd].events &= ~POLLOUT;
//         close(fd);
//         removeFdFromPoll(fd);
//         cgiWriteMap.erase(fd);
//         cgiClient->writing = false;
//         return;
//     }

//     ssize_t n = write(fd, cgiClient->writeBuffer.c_str(), cgiClient->writeBuffer.size());
//     if (n > 0)
// 	{
//         cgiClient->writeBuffer.erase(0, n);
// 		if (!cgiClient->writeBuffer.empty())
//         {
//             // Rester dans POLLOUT pour le prochain poll
//             for (int i = 0; i < MAX_TOTAL_FDS; i++)
//             {
//                 if (_fds[i].fd == fd)
//                 {
//                     _fds[i].events |= POLLOUT;
//                     break;
//                 }
//             }
//         }
//         else
//         {
//             // Tout envoyé
//             //_fds[fd].events &= ~POLLOUT;
// 			close(fd);
// 			removeFdFromPoll(fd);
// 			cgiWriteMap.erase(fd);
//             cgiClient->writing = false;
//         }
// 	}

// 	else if (n == -1 && errno != EAGAIN) // DELETE errno but to test
// 	{
// 		close(fd);
// 		removeFdFromPoll(fd);
// 		cgiWriteMap.erase(fd);
// 		cgiClient->writing = false;
// 	}
// }

void Server::handleCgiRead(int fd)
{
	cgi *cgiClient = cgiReadMap[fd];
	char buffer[4096];

	ssize_t n = read(fd, buffer, sizeof(buffer));

	if (n > 0)
	{
		cgiClient->response.append(buffer, n);
	}
	else if (n == 0) // EOF : le CGI a fini sa sortie
	{
		close(fd);
		removeFdFromPoll(fd);
		cgiReadMap.erase(fd);
		cgiClient->reading = false;

		// Parser la sortie CGI et construire la réponse HTTP
		parsStdout(*cgiClient);
		responseT response;
		buildCgiResponse(*cgiClient, response);

		if (_clients.count(cgiClient->clientFd))
		{
			Client &client = _clients[cgiClient->clientFd];
			client.writeBuffer = response.response;
			client.isReadyToWrite = true;

			addFdToPoll(client.fd, POLLOUT); //ICI
			// Activer POLLOUT pour le client
			// for (int i = 0; i < MAX_TOTAL_FDS; i++)
			// {
			// 	if (_fds[i].fd == client.fd)
			// 	{
			// 		_fds[i].events |= POLLOUT;
			// 		break;
			// 	}
			// }
		}

		// Maintenant que le CGI a fini, on peut fermer son pipe d’écriture si fini
		if (!cgiClient->writing && cgiClient->writePipe[1] >= 0)
		{
			// close(cgiClient->writePipe[1]);
			removeFdFromPoll(cgiClient->writePipe[1]);
			cgiWriteMap.erase(cgiClient->writePipe[1]);
			cgiClient->writePipe[1] = -1;
		}
	}
	else if (n == -1 && errno != EAGAIN)
	{
		perror("read CGI pipe");
		close(fd);
		removeFdFromPoll(fd);
		cgiReadMap.erase(fd);
		cgiClient->reading = false;
	}
	// Si errno == EAGAIN, on attend le prochain poll
}

void Server::handleCgiWrite(int fd)
{
	cgi *cgiClient = cgiWriteMap[fd];

	if (cgiClient->writeBuffer.empty())
	{
		// Plus rien à écrire pour l'instant : désactiver POLLOUT
		for (int i = 0; i < MAX_TOTAL_FDS; i++)
		{
			if (_fds[i].fd == fd)
			{
				_fds[i].events &= ~POLLOUT;
				break;
			}
		}
		// cgiClient->writing = false;
		return;
	}

	ssize_t n = write(fd, cgiClient->writeBuffer.c_str(), cgiClient->writeBuffer.size());

	if (n > 0)
	{
		cgiClient->writeBuffer.erase(0, n);

		if (!cgiClient->writeBuffer.empty())
		{
			// Rester dans POLLOUT pour le prochain poll
			for (int i = 0; i < MAX_TOTAL_FDS; i++)
			{
				if (_fds[i].fd == fd)
				{
					_fds[i].events |= POLLOUT;
					break;
				}
			}
		}
		else
		{
			// Tout envoyé, désactiver POLLOUT, mais ne pas fermer le fd ici
			for (int i = 0; i < MAX_TOTAL_FDS; i++)
			{
				if (_fds[i].fd == fd)
				{
					_fds[i].events &= ~POLLOUT;
					break;
				}
			}
			cgiClient->writing = false;
		}
	}
	else if (n == -1 && errno != EAGAIN)
	{
		perror("write CGI pipe");
		close(fd);
		removeFdFromPoll(fd);
		cgiWriteMap.erase(fd);
		cgiClient->writing = false;
	}
	// Sinon errno == EAGAIN : rien à faire, on attend le prochain poll
}








void Server::checkCgiProcess()
{
	for (std::map<pid_t, cgi *>::iterator it = cgiPidMap.begin(); it != cgiPidMap.end();)
	{
		int status;

		pid_t result = waitpid(it->first, &status, WNOHANG);

		if (result > 0)
			it = cgiPidMap.erase(it);
		else
			++it;
	}
}

// new
void Server::addFdToPoll(int fd, short events)
{
	// check if already exist
    for (int i = 0; i < MAX_TOTAL_FDS; i++)
    {
        if (_fds[i].fd == fd)
        {
            _fds[i].events |= events; // add without delete other
            return;
        }
    }
	//add if not exist
	for (int i = _nbListeningSockets; i < MAX_TOTAL_FDS; i++)
	{
		if (_fds[i].fd == -1)
		{
			_fds[i].fd = fd;
			_fds[i].events = events;
			_fds[i].revents = 0;
			return;
		}
	}
}

void Server::removeFdFromPoll(int fd)
{
	for (int i = 0; i < MAX_TOTAL_FDS; i++)
	{
		if (_fds[i].fd == fd)
		{
			_fds[i].fd = -1;
			return;
		}
	}
}




void Server::forkCgi(cgi &cgiClient, Client &client)
{
	if (cgiClient.pid != -1)
		return;

	if (cgiClient.pid == -1)
	{
		if (pipe(cgiClient.writePipe) == -1 || pipe(cgiClient.readPipe) == -1)
		{
			close(cgiClient.writePipe[0]);
			close(cgiClient.writePipe[1]);
			close(cgiClient.readPipe[0]);
			close(cgiClient.readPipe[1]);
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

		else if (cgiClient.pid == 0) // child
		{
			close(cgiClient.writePipe[1]);
			close(cgiClient.readPipe[0]);

			dup2(cgiClient.writePipe[0], STDIN_FILENO);
			dup2(cgiClient.readPipe[1], STDOUT_FILENO);
			// dup2(cgiClient.writePipe[1], STDIN_FILENO);  // écrit dans le CGI
			// dup2(cgiClient.readPipe[0], STDOUT_FILENO);  // lit la sortie du CGI

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

			write(STDOUT_FILENO, "HELLO CGI\n", 10);

			execve(cgiClient.binaryPath.c_str(), args, envp.data());
		}
		else // parent
		{
			cgiClient.clientFd = client.fd;
			close(cgiClient.writePipe[0]);
			close(cgiClient.readPipe[1]);

			fcntl(cgiClient.readPipe[0], F_SETFL, O_NONBLOCK);
			if (!cgiClient.body.empty())
				fcntl(cgiClient.writePipe[1], F_SETFL, O_NONBLOCK);

			cgiClient.reading = true;
			cgiClient.writing = !cgiClient.body.empty();
				
			cgiClient.writeBuffer = cgiClient.body; // corps à envoyer au CGI
			
			cgiClient.readBuffer.clear();

			// add in poll
			addFdToPoll(cgiClient.readPipe[0], POLLIN);
			if (!cgiClient.body.empty())
				addFdToPoll(cgiClient.writePipe[1], POLLOUT);
			
			// addFdToPoll(cgiClient.writePipe[1], POLLIN);
			// addFdToPoll(cgiClient.readPipe[0], POLLOUT);

			cgiPidMap[cgiClient.pid] = &cgiClient;
			cgiReadMap[cgiClient.readPipe[0]] = &cgiClient;
			cgiWriteMap[cgiClient.writePipe[1]] = &cgiClient;

			client.isReadyToWrite = false;
		}
	}
}