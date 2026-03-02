/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Run.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 14:53:40 by lserodon          #+#    #+#             */
/*   Updated: 2026/03/02 10:37:27 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Cgi.hpp"

bool server_run = true;

/**
 * @brief Main event loop of the server using poll().
 *
 * This function continuously runs while the global flag `server_run` is true.
 * It handles new client connections, client requests, CGI pipes, and timeouts.
 *
 * Steps performed in the loop:
 *
 * 1. Polling: Calls `poll(_fds, MAX_TOTAL_FDS, 1000)` to wait for events on all monitored file descriptors.
 *
 * 2. Accepting new connections:
 *    - Iterates over the first `_nbListeningSockets` fds which are the listening sockets.
 *    - If the fd has `POLLIN`, it means there is a new incoming connection.
 *    - Calls `_acceptNewConnection(i)` to accept the new client and set up its fd for polling.
 *
 * 3. Handling existing fds:
 *    - Iterates over the remaining fds (`_nbListeningSockets` to `MAX_TOTAL_FDS`).
 *    - Skips fd == -1 (unused).
 *    - Skips if `revents == 0` (no events on this fd).
 *
 * 4. Error and hangup handling:
 *    - If `revents` has `POLLERR` the fd is closed and removed from poll.
 * 	  - If `revents` has `POLLHUP`(EOF) 
 * 		- Calls `handleCgiRead(fd)` to read CGI output, parse it, and build HTTP responses.
 *
 * 5. CGI pipes handling:
 *    - If the fd has `POLLIN` and exists in `cgiReadMap`, it is a CGI output pipe ready to read.
 *      - Calls `handleCgiRead(fd)` to read CGI output, parse it, and build HTTP responses.
 *    - If the fd has `POLLOUT` and exists in `cgiWriteMap`, it is a CGI input pipe ready to write.
 *      - Calls `handleCgiWrite(fd)` to write data to the CGI process.
 *
 * 6. Client activity:
 *    - If the fd exists in `_clients`, it is a normal client socket.
 *    - Calls `_handleClientActivity(i)` to read request data, generate responses, and handle client write events.
 *
 * 7. CGI process cleanup:
 *    - Calls `checkCgiProcess()` to reap finished CGI child processes using `waitpid(WNOHANG)`.
 *
 * 8. Timeout management:
 *    - Calls `_checkTimeouts()` to close client connections that have timed out.
 *
 * 9. Loop continues until `server_run` is false.
 *    - When the server stops, it prints "[INFO] Server stopping gracefully...".
 *
 * @note Multiplexed IO using poll():
 *    - POLLIN: data is available to read (new connection, client data, CGI output)
 *    - POLLOUT: fd is ready to write (send client response, write to CGI stdin)
 *    - POLLERR: error occurred on fd (socket error, broken pipe)
 *    - POLLHUP: peer has closed connection (EOF on pipe or socket closed)
 */
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

			if (_fds[i].revents & POLLERR)
			{
				removeFdFromPoll(fd);
				close(fd);
				continue;
			}

			if (_fds[i].revents & POLLHUP)
			{
				if (cgiReadMap.count(fd))
					handleCgiRead(fd);

				else
				{
					removeFdFromPoll(fd);
					close(fd);
				}
				continue;
			}

			// READ PIPE
			if ((_fds[i].revents & POLLIN) && cgiReadMap.count(fd))
				handleCgiRead(fd);
			
			// WRITE PIPE
			if ((_fds[i].revents & POLLOUT) && cgiWriteMap.count(fd))
				handleCgiWrite(fd);
				
			if (_clients.count(fd))
			{
				if (_handleClientActivity(i) == -1)
				{
					removeFdFromPoll(fd);
					close(fd);
				}
			}
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