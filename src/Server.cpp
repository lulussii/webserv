/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 11:32:08 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/02 12:42:24 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

#define LISTEN_BACKLOG 5

Server::Server(int port) : _port(port), _serverFd(-1) 
{
	for (int i = 0; i <= MAX_CLIENTS; ++i)
	{
		_fds[i].fd = -1;
		_fds[i].events = POLLIN;
	}
}

int	Server::_createServerSocket(int port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) return -1;
    
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) 
		return -1;
    if (listen(fd, LISTEN_BACKLOG) == -1) 
		return -1;
    return fd;
}

int	Server::_acceptClient(int server_fd)
{
	struct sockaddr_in client_addr;

	socklen_t size = sizeof(client_addr);
	int client_fd = (accept(server_fd, (struct sockaddr *) &client_addr, &size));
	if (client_fd == -1)
		return (-1);
	return (client_fd);
}

void Server::setup()
{
	_serverFd = _createServerSocket(_port);
	if (_serverFd == -1)
		throw std::runtime_error("Error: Impossible de créer le socket serveur");
	_fds[0].fd = _serverFd;
	_fds[0].events = POLLIN;
	std::cout << "--- SERVEUR WEBSERV LANCÉ SUR LE PORT " << _port << " ---" << std::endl;
}

void	Server::run()
{
	while(true)
	{
		if (poll(_fds, MAX_CLIENTS + 1, -1) < 0)
			break;
		if (_fds[0].revents & POLLIN)
			_acceptNewConnection();
		for (int i = 1; i <= MAX_CLIENTS; i++)
		{
			if (_fds[i].fd != -1)
				_handleClientActivity(i);
		}
	}
}

void	Server::_acceptNewConnection()
{
	int	client_fd = _acceptClient(_serverFd);
	if (client_fd != -1)
	{
		for (int i = 1; i <= MAX_CLIENTS; i++)
		{
			if (_fds[i].fd == -1)
			{
				_fds[i].fd = client_fd;
				_fds[i].events = POLLIN;
				_clients[client_fd] = Client(client_fd);
				std::cout << "[Connecté] FD: " << client_fd << std::endl;
				return;
			}
		}
	}
}

void	Server::_handleClientActivity(int i)
{
	Client &c = _clients[_fds[i].fd];

	if (_fds[i].revents & POLLIN)
		_readFromClient(i, c);
	if (_fds[i].fd != -1 && (_fds[i].revents & POLLOUT))
		_writeToClient(i, c);
}

void	Server::_readFromClient(int i, Client &c)
{
	char 	tmp[1024];
	int		bytes = read(_fds[i].fd, tmp, sizeof(tmp) - 1);

	if (bytes <= 0)
		_closeConnection(i);
	else
	{
		c.readBuffer.append(tmp, bytes);
		if (c.readBuffer.find("\r\n\r\n") != std::string::npos)
		{
			parsingT p;
			p.line = c.readBuffer;
			if (requestMain(c.req, p) == 0)
			{
				responseMain(c.req, c.res);
				c.writeBuffer = c.res.response;
				c.isReadyToWrite = true;
				_fds[i].events |= POLLOUT;
			}
			c.readBuffer.clear();
		}
	}
}

void	Server::_writeToClient(int i, Client &c)
{
	if (c.isReadyToWrite && !c.writeBuffer.empty())
	{
		int sent = write(_fds[i].fd, c.writeBuffer.c_str(), c.writeBuffer.size());
		if (sent > 0) 
		{
			c.writeBuffer.erase(0, sent);
			if (c.writeBuffer.empty()) 
			{
				c.isReadyToWrite = false;
				_fds[i].events &= ~POLLOUT;
			}
		}
	}
}

void Server::_closeConnection(int i) 
{
	std::cout << "[Déconnexion] FD: " << _fds[i].fd << std::endl;
	close(_fds[i].fd);
	_clients.erase(_fds[i].fd);
	_fds[i].fd = -1;
}