/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 11:06:16 by lserodon          #+#    #+#             */
/*   Updated: 2026/03/02 10:33:26 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <map>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <vector>

#include "Client.hpp"
#include "Config.hpp"
#include "Delete.hpp"
#include "Get.hpp"
#include "Init.hpp"
#include "Post.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerConfig.hpp"

#define MAX_TOTAL_FDS 1024
#define LISTEN_BACKLOG 5

extern bool server_run;

class Server
{
	private: 

		int						_nbListeningSockets;
		struct pollfd			_fds[MAX_TOTAL_FDS];
		std::map<int, Client>	_clients;
		std::vector<ServerConfig> _configs;
		std::vector<serverT> 	_refinedConfigs;

		std::map<int, int>		_serverSockets;

		serverT _convertConfig(const ServerConfig &myConfig);

		int		_createServerSocket(int port);
		void	_acceptNewConnection(int serverFd);
		int	_handleClientActivity(int i);
		void	_closeConnection(int i);
		void	_checkTimeouts();
		
   		void 	_initRefinedConfigs();

	public:
		Server(const std::vector<ServerConfig> &configs);
		~Server();

		void setup();
		void run();
		
		ServerConfig &getConfig(int port, std::string hostHeader);

		/*CGI PART*/
		std::map<int, cgi*> cgiReadMap;
		std::map<int, cgi*> cgiWriteMap;
		std::map<pid_t, cgi*> cgiPidMap;

		void handleCgiRead(int fd);
		void handleCgiWrite(int fd);
		void checkCgiProcess();

		int forkCgi(cgi &cgiClient, Client &client);
		/*END CGI PART*/

		void addFdToPoll(int fd, short events);
		void removeFdFromPoll(int fd);
	};
	

void handle_sigint(int sig);

#endif