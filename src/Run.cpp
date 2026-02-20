/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Run.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 14:53:40 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/20 17:48:41 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Cgi.hpp"

bool server_run = true;

void Server::run()
{
	const int totalFds = MAX_TOTAL_FDS;
	
	while(server_run)
	{
		int ret = poll(_fds, totalFds, 1000);
		
		if (ret < 0 && server_run) break;
		if (ret == 0) continue;

		for (int i = 0; i < _nbListeningSockets; i++)
		{
			if (_fds[i].revents & POLLIN)
				_acceptNewConnection(i);   
		}
	
		for (int i = _nbListeningSockets; i < totalFds; i++)
		{
			int fd = _fds[i].fd;
 
			if (fd == -1)
				continue;

			if (_fds[i].revents == 0)
				continue;
				
			
			if (_fds[i].revents & (POLLERR | POLLHUP))
			{
				removeFdFromPoll(fd);
				close(fd);
				continue;
			}

			// READ PIPE
			if ((_fds[i].revents & POLLIN) && cgiReadMap.count(fd))
			{
				std::cout << "OUIIIIIII\n";
				handleCgiRead(fd);
			}
			
			// WRITE PIPE
			else if ((_fds[i].revents & POLLOUT) && cgiWriteMap.count(fd))
				handleCgiWrite(fd);
				
			else //if (_fds[i].revents != 0)
				_handleClientActivity(i);
		}

		checkCgiProcess();
		_checkTimeouts();
	}
	std::cout << "\n[INFO] Server stopping gracefully..." << std::endl;
}

void Server::_checkTimeouts()
{
	time_t now = time(NULL);
	for (int i = _nbListeningSockets; i < MAX_TOTAL_FDS; i++)
	{
		int fd = _fds[i].fd;
		if (fd >= 0 && _clients.count(fd))
		{
			if (difftime(now, _clients[fd].lastTime) > 60)
			{
				std::cout << "[TIMEOUT] Client " << fd << " inactive." << std::endl;
				_closeConnection(i);
			}
		}
	}
}