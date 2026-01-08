/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 11:32:08 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/08 10:14:55 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdlib>

#define LISTEN_BACKLOG 5 // Définit la taille de la file d'attente

/**
 * @brief Constructeur : Initialise le serveur et vide le tableau de pollfd.
 */
Server::Server(int port) : _port(port), _serverFd(-1) 
{
	for (int i = 0; i <= MAX_CLIENTS; ++i)
	{
		_fds[i].fd = -1;			// -1 signifie que le slot est libre
		_fds[i].events = POLLIN;	// Par défaut, on écoute (lecture)
	}
}

/**
 * @brief Crée et configure le socket principal du serveur (Socket, Bind, Listen).
 * @return Le descripteur de fichier (FD) du serveur ou -1 en cas d'erreur.
 */
int	Server::_createServerSocket(int port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0); // Création du socket 
    if (fd == -1) 
		return -1;
	// Permet de rendre le socket non-bloquant
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "Error : Failed to set non-blocking mode on server socket" << std::endl;
		close (fd);
		return (-1);
	}
	// Évite l'erreur "Address already in use" lors d'un redémarrage rapide
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port); // htons : Convertit le port (format humain) vers le format réseau
	addr.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY : Accepte les connexions de n'importe quelle adresse (localhost, wifi, etc)

	// Réservation de l'adresse pour le socket
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		std::cerr << "Error: Failed to bind to port " << port << std::endl;
		close(fd);
		return (-1);
	}
	// Déclaration d'ouverture de la file d'attente
	if (listen(fd, LISTEN_BACKLOG) == -1)
	{
		std::cerr << "Error: Failed to listen on socket" << std::endl;
		close (fd);
		return (-1);
	}
	return fd;
}

/**
 * @brief Wrapper pour la fonction système accept().
 */
int	Server::_acceptClient(int server_fd)
{
	struct sockaddr_in client_addr;

	socklen_t size = sizeof(client_addr);
	int client_fd = (accept(server_fd, (struct sockaddr *) &client_addr, &size));
	if (client_fd == -1)
		return (-1);
	return (client_fd);
}

/**
 * @brief Point d'entrée pour démarrer le serveur.
 */
void Server::setup()
{
	_serverFd = _createServerSocket(_port);
	if (_serverFd == -1)
		throw std::runtime_error("[ERROR] Failed to create server socket");

	// Le premier slot de poll est toujours réservé au serveur lui-même
	_fds[0].fd = _serverFd;
	_fds[0].events = POLLIN;
	
	std::cout << "[SUCCESS] Server started succesfully" << std::endl;
	std::cout << "[INFO] Listening on port " << _port << std::endl;
}

/**
 * @brief Boucle principale (Heartbeat). Utilise poll() pour surveiller les événements.
 */
void	Server::run()
{
	while(true)
	{
		int ret = poll(_fds, MAX_CLIENTS + 1, 1000);

		if (ret < 0)
			break;

		// Cas 1 : Nouvelle tentative de connexion sur le socket serveur
		if (_fds[0].revents & POLLIN)
			_acceptNewConnection();

		// Cas 2 : Activité sur un socket client déjà existant
		for (int i = 1; i <= MAX_CLIENTS; i++)
		{
			if (_fds[i].fd != -1 && _fds[i].revents != 0)
				_handleClientActivity(i);
		}

		// Gestion du TIMEOUT
		for (int i = 1; i <= MAX_CLIENTS; i++)
		{
			if (_fds[i].fd != -1)
			{
				time_t	now = time(NULL);
				double diff = difftime(now, _clients[_fds[i].fd].lastTime);
				std::cout << "[DEBUG] Client " << _fds[i].fd << " inactive for " << diff << " seconds." << std::endl;
				if (diff > 60)
				{
					std::cout << "[TIMEOUT] Client " << _fds[i].fd << " disconnected (inactive)." << std::endl;
					_closeConnection(i);
				}
			}
		}
	}
}

/**
 * @brief Accepte un nouveau client et lui cherche une place dans le tableau _fds.
 */
void	Server::_acceptNewConnection()
{
	int	client_fd = _acceptClient(_serverFd);
	if (client_fd != -1)
	{
		if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
		{
			std::cerr << "[Error] Failed to set non-blocking mode on cient FD" << std::endl;
			close (client_fd);
			return ;
		}
		for (int i = 1; i <= MAX_CLIENTS; i++)
		{
			if (_fds[i].fd == -1) // Un slot libre est trouvé
			{
				_fds[i].fd = client_fd;
				_clients[client_fd] = Client(client_fd); // Création de l'objet Client
				std::cout << "[CONNEXION] New connection established (FD: " << client_fd << ")" << std::endl;
				return;
			}
		}
		//Si on arrive ici, le serveur est plein
		std::cerr << "[Error] Max clients reached. Connection rejected on FD " << client_fd << std::endl; 
		close(client_fd);
	}
}

/**
 * @brief Dispatch l'activité d'un client vers la lecture ou l'écriture.
 */
void	Server::_handleClientActivity(int i)
{
	Client &c = _clients[_fds[i].fd];
	c.lastTime = time(NULL);

	// Si le client envoie des données
	if (_fds[i].revents & POLLIN)
		_readFromClient(i, c);

	// Si la réponse est prête et que le client est prêt à la recevoir
	if (_fds[i].fd != -1 && (_fds[i].revents & POLLOUT))
		_writeToClient(i, c);
}

/**
 *  @brief Cherche "Content-Length" dans la requête. Retourne 0 si pas trouvé.
 */
long	Server::_getContentLength(std::string &request)
{
	size_t pos = request.find("Content-Length: ");
	if (pos == std::string::npos)
		return (0);
	
	// On se place juste après "Content-Length: "
	size_t	start = pos + 16;

	// On cherche la fin de la ligne (\r\n)
	size_t	end = request.find("\r\n", start);

	// On extrait le nombre
	std::string num = request.substr(start, end - start);
	long	res = atol(num.c_str());

	return (res);
}

/**
 * @brief Lit les données entrantes, les accumule, et appelle le Parser quand la requête est finie.
 */
void	Server::_readFromClient(int i, Client &c)
{
	char 	tmp[1024];
	int		bytes = read(_fds[i].fd, tmp, sizeof(tmp) - 1);

	if (bytes > 0)
	{
		c.readBuffer.append(tmp, bytes);
		
		if (!c.headersReceived)
		{
			if (c.readBuffer.find("\r\n\r\n") != std::string::npos)
			{
				c.headersReceived = true;
				c.contentLength = _getContentLength(c.readBuffer);
				std::cout << "[DEBUG] Headers received. Waiting for body size:" << c.contentLength << std::endl;
			}
		}
		if (c.headersReceived)
		{
			size_t	headerEnd = c.readBuffer.find("\r\n\r\n");
			size_t	totalExcpected = headerEnd + 4 + c.contentLength;
			
			if (c.readBuffer.size() >= totalExcpected)
			{
				std::cout << "[REQ] Full request received (" << c.readBuffer.size() << " bytes)" << std::endl;
				
				parsingT p;
				p.line = c.readBuffer;
				
				// Parsing de la requête 
				if (requestMain(c.req, p) == 0)
				{
					// Génération de la réponse
					responseMain(c.req, c.res);

					//Stockage de la réponse brute et on préient poll qu'on veut écrire
					c.writeBuffer = c.res.response;
					c.isReadyToWrite = true;
					_enableWriting(i);
				}
				c.readBuffer.erase(0, totalExcpected);
				c.headersReceived = false;
				c.contentLength = 0;
			}
		}
	}
	else if (bytes == 0)
		_closeConnection(i);
	else
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
			_closeConnection(i);	
	}
}

/**
 * @brief Envoie la réponse stockée dans writeBuffer vers le client.
 */
void	Server::_writeToClient(int i, Client &c)
{
	if (c.isReadyToWrite && !c.writeBuffer.empty())
	{
		int sent = write(_fds[i].fd, c.writeBuffer.c_str(), c.writeBuffer.size());
		if (sent > 0) 
		{
			std::cout << "[SEND] Response send to FD " << _fds[i].fd << " (" << sent << " bytes)" << std::endl;			
			// On retire de buffer ce qui a déjà été envoyé
			c.writeBuffer.erase(0, sent);

			// Si tout est envoyé, on arrête d'écouter en POLLOUT
			if (c.writeBuffer.empty()) 
			{
				c.isReadyToWrite = false;
				std::cout << "[CLOSE] Closing connection (FD: " << _fds[i].fd << ")" << std::endl;
				_closeConnection(i);
			}
		}
	}
}

/**
 * @brief Nettoie proprement les ressources d'un client déconnecté.
 */
void Server::_closeConnection(int i) 
{
	std::cout << "[DISCONNECTION] Client disconnected (FD: " << _fds[i].fd << ")" << std::endl;
	close(_fds[i].fd);				// Ferme le socket
	_clients.erase(_fds[i].fd);		// Supprime l'objet Client de la Map
	_fds[i].fd = -1;				// Libère le slot pour poll()
}

/**
 * @brief Active le mode Lecture/Écriture pour le socket.
 * Modifie le masque d'événements pour surveiller POLLOUT.
 * Appelé une fois que la réponse HTTP est prête à être envoyée.
 */
void	Server::_enableWriting(int i)
{
	_fds[i].events = POLLIN | POLLOUT;
}

/**
 * @brief Repasse le socket en mode Lecture seule.
 * Réinitialise le masque sur POLLIN uniquement.
 * Évite l'attente active (busy-waiting) une fois l'envoi terminé.
 */
void	Server::_disableWriting(int i)
{
	_fds[i].events = POLLIN;
}