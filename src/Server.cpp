/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 11:32:08 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/16 13:29:08 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void handle_sigint(int sig)
{
	(void)sig;
	server_run = false;
}

Server::Server(const std::vector<ServerConfig> &configs) : _configs(configs) 
{
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		_fds[i].fd = -1;
		_fds[i].events = POLLIN;
		_fds[i].revents = 0;
	}
}

Server::~Server()
{
	for (int i = 0; i < MAX_CLIENTS + _nbListeningSockets; i++)
	{
		if (_fds[i].fd >= 0)
			close(_fds[i].fd);
	}
	_clients.clear();
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
			for (size_t k = 0; k < openPorts.size(); k++) {
				if (openPorts[k] == currentPort) { alreadyOpen = true; break; }
			}
			if (alreadyOpen) continue;
			
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