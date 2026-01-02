/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 11:06:16 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/02 12:28:16 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <vector>
#include <map>
#include <poll.h>
#include "Client.hpp"

#define MAX_CLIENTS 10

class Server
{
	private: 
		int						_port;
		int						_serverFd;
		struct pollfd			_fds[MAX_CLIENTS + 1];
		std::map<int, Client>	_clients;

		int		_createServerSocket(int port);
		int		_acceptClient(int server_fd);
		void	_acceptNewConnection();
		void	_handleClientActivity(int i);
		void	_readFromClient(int i, Client &c);
		void	_writeToClient(int i, Client &c);
		void	_closeConnection(int i);

	public:
		Server(int port);
		
		void setup();
		void run();
};

#endif