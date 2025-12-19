/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/11 13:57:44 by lserodon          #+#    #+#             */
/*   Updated: 2025/12/19 08:52:51 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>

#define LISTEN_BACKLOG 5
#define MAX_CLIENTS 10

/* struct sockaddr_in {
    sa_family_t    sin_family;   // Type d'adresse (AF_INET pour IPv4)
    in_port_t      sin_port;     // Port (attention : format réseau !)
    struct in_addr sin_addr;     // Adresse IP
    char           sin_zero[8];  // Remplissage pour compatibilité
}; */

int create_server_socket(int port)
{
	int fd;
	struct sockaddr_in addr;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd== -1)
		return (-1);
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
		return (-1);
		
	if (listen(fd, LISTEN_BACKLOG) == -1)
		return (-1);

	return (fd);
}

int accept_client(int server_fd)
{
	struct sockaddr_in client_addr;

	socklen_t size = sizeof(client_addr);
	int client_fd = (accept(server_fd, (struct sockaddr *) &client_addr, &size));
	if (client_fd == -1)
		return (-1);
	return (client_fd);
}

int read_request(int client_fd, char *buffer, int buffer_size)
{
	int bytes_read;

	bytes_read = read(client_fd, buffer, buffer_size - 1);
	if (bytes_read > 0)
		buffer[bytes_read] = '\0';
	std::cout << buffer << std::endl;
	return (bytes_read);
}

void send_response(int client_fd)
{
	const char *response = "HTTP/1.0 200 OK\r\nContent-Length: 13\r\n\r\nHello world!\n";
	write(client_fd, response, strlen(response));
}

int main(void)
{

	std::string buffers[MAX_CLIENTS + 1];
	int server_fd = create_server_socket(8080);
	if (server_fd == -1)
		return (-1);
	
	struct pollfd fds[MAX_CLIENTS + 1];
	fds[0].fd = server_fd;
	fds[0].events = POLLIN;
	for (int i = 1; i <= MAX_CLIENTS; ++i)
	{
		fds[i].events = POLLIN;
		fds[i].fd = -1;
	}
	while (1)
	{
		poll(fds, MAX_CLIENTS + 1, -1);
		if (fds[0].revents & POLLIN)
		{
			int client_fd = accept_client(server_fd);
			if (client_fd == -1)
				return (-1);
			for (int i = 1; i <= MAX_CLIENTS; i++)
			{
				if (fds[i].fd == -1)
				{
					fds[i].fd = client_fd;
					break;					
				}
			}
		}
		char buffer[1024];
		for (int j = 0; j <= MAX_CLIENTS; ++j)
		{		
			if (fds[j].fd != -1 && (fds[j].revents & POLLIN))
    		{
    			int bytes = read(fds[j].fd, buffer, sizeof(buffer));
    			if (bytes > 0)
				{
					buffer[bytes] = '\0';
					buffers[j] += buffer;

					std::cout << "BUFFER CLIENT" << j << ":\n";
					std::cout << buffers[j] << std::endl;

					if (buffers[j].find("\r\n\r\n") != std::string::npos)
					{
						write(fds[j].fd, buffers[j].c_str(), buffers[j].size());
						buffers[j].clear();
						
						std::string response_buffer;
						char tmp[1024];
						int bytes;
						while ((bytes = read(fds[j].fd, tmp, sizeof(tmp))) > 0)
							response_buffer.append(tmp, bytes);

						size_t pos = response_buffer.find("\r\n\r\n");
						if (pos != std::string::npos)
						{
							std::string body = response_buffer.substr(pos + 4);
							std::cout << "BODY RECEIVED:\n" << body << std::endl;
							response_buffer.clear();
						}
						//close(fds[j].fd);
						//fds[j].fd = -1;
					}	
				}
    			else
    			{
    		    	close(fds[j].fd);
    		    	fds[j].fd = -1;
					buffers[j].clear();
    			}
    		}
		}
	}
	for (int i = 0; i <= MAX_CLIENTS; ++i)
	{
		if (fds[i].fd != -1)
			close (fds[i].fd);
	}
	close(server_fd);
    return 0;
}