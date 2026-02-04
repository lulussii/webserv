/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 11:06:16 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/04 15:08:37 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <map>
#include <poll.h>
#include "Client.hpp"
#include "ServerConfig.hpp"
#include "Config.hpp"

#define MAX_CLIENTS 10

class Server
{
	private: 
		//int						_port;
		int						_nbListeningSockets;
		struct pollfd			_fds[MAX_CLIENTS + 1];
		std::map<int, Client>	_clients;
		std::vector<ServerConfig> _configs;

		std::map<int, int>		_serverSockets;

		serverT _convertConfig(const ServerConfig &myConfig);

		int		_createServerSocket(int port);
		int		_acceptClient(int server_fd);
		void	_acceptNewConnection(int serverFd);
		void	_handleClientActivity(int i);
		void	_closeConnection(int i);
		
		void	_enableWriting(int i);
		void	_disableWriting(int i);

	public:
		Server(const std::vector<ServerConfig> &configs);
		
		void setup();
		void run();

		

};

#endif