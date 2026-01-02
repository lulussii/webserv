/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 09:47:55 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/02 12:26:55 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <map>

#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <map>
#include "Client.hpp" // Assure-toi que ce fichier contient ta classe Client
#include "Server.hpp"

#define MAX_CLIENTS 10
#define LISTEN_BACKLOG 5

/* 
int main(void) {
    std::map<int, Client> clients;
    int server_fd = create_server_socket(8080);
    if (server_fd == -1) {
        std::cerr << "Erreur fatale : impossible de créer le socket serveur." << std::endl;
        return 1;
    }

    struct pollfd fds[MAX_CLIENTS + 1];
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;

    for (int i = 1; i <= MAX_CLIENTS; ++i) fds[i].fd = -1;

    std::cout << "--- SERVEUR WEBSERV LANCÉ SUR LE PORT 8080 ---" << std::endl;

    while (true) {
        if (poll(fds, MAX_CLIENTS + 1, -1) < 0) break;

        // --- 1. ACCEPTATION DES NOUVEAUX CLIENTS ---
        if (fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t size = sizeof(client_addr);
            int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &size);
            
            if (client_fd != -1) {
                for (int i = 1; i <= MAX_CLIENTS; i++) {
                    if (fds[i].fd == -1) {
                        fds[i].fd = client_fd;
                        fds[i].events = POLLIN; // On commence par écouter le client
                        clients[client_fd] = Client(client_fd);
                        std::cout << "[Connexion] Nouveau client sur FD: " << client_fd << std::endl;
                        break;
                    }
                }
            }
        }

        // --- 2. GESTION DES REQUÊTES ET RÉPONSES ---
        for (int i = 1; i <= MAX_CLIENTS; i++) {
            if (fds[i].fd == -1) continue;

            // A. LECTURE (POLLIN)
            if (fds[i].revents & POLLIN) {
                Client &c = clients[fds[i].fd];
                char tmp[1024];
                int bytes = read(fds[i].fd, tmp, sizeof(tmp) - 1);

                if (bytes <= 0) {
                    std::cout << "[Déconnexion] FD: " << fds[i].fd << std::endl;
                    close(fds[i].fd);
                    clients.erase(fds[i].fd);
                    fds[i].fd = -1;
                } else {
                    c.readBuffer.append(tmp, bytes);
                    
                    // Si on détecte la fin d'une requête HTTP (\r\n\r\n)
                    if (c.readBuffer.find("\r\n\r\n") != std::string::npos) {
                        parsingT p;
                        p.line = c.readBuffer;

                        // Appel du code de ta partenaire
                        if (requestMain(c.req, p) == 0) {
                            responseMain(c.req, c.res); // Elle prépare la réponse
                            c.writeBuffer = c.res.response;
                            c.isReadyToWrite = true;
                            
                            // On active le mode écriture pour ce client dans poll
                            fds[i].events |= POLLOUT; 
                        }
                        c.readBuffer.clear(); // On vide pour la prochaine requête
                    }
                }
            }

            // B. ÉCRITURE (POLLOUT)
            if (fds[i].fd != -1 && (fds[i].revents & POLLOUT)) {
                Client &c = clients[fds[i].fd];
                if (c.isReadyToWrite && !c.writeBuffer.empty()) {
                    int sent = write(fds[i].fd, c.writeBuffer.c_str(), c.writeBuffer.size());
                    if (sent > 0) {
                        c.writeBuffer.erase(0, sent);
                        // Si on a fini d'envoyer toute la réponse
                        if (c.writeBuffer.empty()) {
                            c.isReadyToWrite = false;
                            fds[i].events &= ~POLLOUT; // On revient en mode lecture seule
                        }
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
} */

int main() 
{
	Server webServ(8080);

	try {
		webServ.setup();
		webServ.run();
	} catch (const std::exception &e) {
		std::cerr << "Critical error: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}
