/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/11 13:57:44 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/02 10:38:56 by lserodon         ###   ########.fr       */
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
