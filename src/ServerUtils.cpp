/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 14:54:01 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/12 14:54:17 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int Server::_createServerSocket(int port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) return -1;

	int opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	fcntl(fd, F_SETFL, O_NONBLOCK);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		close(fd);
		return -1;
	}
	if (listen(fd, LISTEN_BACKLOG) == -1) {
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
	if (clientFd < 0) return;

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) {
		close(clientFd);
		return;
	}

	int clientIndex = -1;
	for (int i = _nbListeningSockets; i < MAX_CLIENTS + _nbListeningSockets; i++) {
		if (_fds[i].fd == -1) {
			clientIndex = i;
			break;
		}
	}

	if (clientIndex != -1) {
		_fds[clientIndex].fd = clientFd;
		_fds[clientIndex].events = POLLIN;
		_fds[clientIndex].revents = 0;
		
		int port = _serverSockets[serverFd];
		_clients[clientFd] = Client(clientFd, port);
		std::cout << "[CONNEXION] FD: " << clientFd << " on Port: " << port << std::endl;
	} else {
		close(clientFd);
	}
}

void Server::_handleClientActivity(int index)
{
	int fd = _fds[index].fd;
	Client &client = _clients[fd];

	try {
		if (_fds[index].revents & POLLIN) {
			client.handleRead(this->_refinedConfigs);
		}

		if ((_fds[index].revents & POLLOUT) && client.isReadyToWrite) {
			client.handleWrite();
		}

		if (client.isReadyToWrite)
			_fds[index].events = POLLIN | POLLOUT;
		else
			_fds[index].events = POLLIN;
	}
	catch (std::exception &e) {
		_closeConnection(index);
	}
}

void Server::_closeConnection(int index)
{
	int fd = _fds[index].fd;
	if (_clients.count(fd)) {
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

	if (!myConfig._listen.empty()) {
		newConfig.listens = myConfig._listen;
		newConfig.listen = myConfig._listen[0].port;
	} else {
		newConfig.listens.push_back(Listen("0.0.0.0", 8080));
		newConfig.listen = 8080;
	}
	
	newConfig.root = myConfig._root;
	newConfig.clientMaxBodySize = myConfig._clientMaxBodySize;
	newConfig.errorPage = myConfig._errorPages;
	newConfig.servernames = myConfig._serverNames;

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

		if (curr._allowGet) newLoc.methods.push_back("GET");
		if (curr._allowPost) newLoc.methods.push_back("POST");
		if (curr._allowDelete) newLoc.methods.push_back("DELETE");

		newConfig.locations[curr._path] = newLoc;
	}
	return newConfig;
}