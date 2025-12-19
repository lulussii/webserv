/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/11 13:57:44 by lserodon          #+#    #+#             */
/*   Updated: 2025/12/19 15:17:42 by lserodon         ###   ########.fr       */
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


/**
 * @brief Crée un socket serveur TCP IPv4.
 * 
 * socket() : création du fd
 * bind() : association IP + port
 * listen() : mise en écoute
 * @return le fd du serveur ou -1 en cas d'erreur.
*/
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

/**
 * @brief Accepte une nouvelle connexion entrante sur le socket serveur.
 * - Crée un nouveau fd client avec accept()
 * - Le fd serveur reste ouvert
 * @return le fd du client ou -1 en cas d'erreur.
*/
int accept_client(int server_fd)
{
	struct sockaddr_in client_addr;

	socklen_t size = sizeof(client_addr);
	int client_fd = (accept(server_fd, (struct sockaddr *) &client_addr, &size));
	if (client_fd == -1)
		return (-1);
	return (client_fd);
}

/**
 * @brief Envoie une réponse HTTP minimale au client.
 * 	Utilisée uniquement pour les tests.
*/
void send_response(int client_fd)
{
	const char *response = "HTTP/1.0 200 OK\r\nContent-Length: 13\r\n\r\nHello world!\n";
	write(client_fd, response, strlen(response));
}

/**
* @brief Gère la lecture des données d'un client.
* - Lit les données disponibles (read)
* - Accumule dans un buffer par client
* - Ferme le fd si read <= 0
* - Détecte la fin de requête HTTP (\r\n\r\n)
*/
void handle_client_read(struct pollfd &fd_entry, std::string &buffer)
{
	char tmp[1024];
	int bytes = read(fd_entry.fd, tmp, sizeof(tmp) - 1);
	if (bytes <= 0)
	{
		close(fd_entry.fd);
		fd_entry.fd = -1;
		buffer.clear();
		return ;
	}

	buffer.append(tmp, bytes);
	std::cout << "BUFFER CLIENT " << fd_entry.fd << ":\n" << buffer << std::endl;
	if (buffer.find("\r\n\r\n") != std::string::npos)
	{
		send_response(fd_entry.fd);
		//write(fd_entry.fd, buffer.c_str(), buffer.size());
		buffer.clear();
	}
}


int main(void)
{
	std::string buffers[MAX_CLIENTS + 1];
	int server_fd = create_server_socket(8080);
	if (server_fd == -1)
		return (-1);
	
	struct pollfd fds[MAX_CLIENTS + 1];
	fds[0] = { server_fd, POLLIN, 0};
	
	for (int i = 1; i <= MAX_CLIENTS; ++i)
		fds[i] = { -1, POLLIN, 0};
	
	while (true)
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
		
		for (int j = 1; j <= MAX_CLIENTS; j++)
		{		
			if (fds[j].fd != -1 && (fds[j].revents & POLLIN))
				handle_client_read(fds[j], buffers[j]);
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
