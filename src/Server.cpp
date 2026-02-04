/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 11:32:08 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/04 15:10:34 by lserodon         ###   ########.fr       */
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

#include "Init.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"
#include "Get.hpp"
#include "Post.hpp"
#include "Delete.hpp"

#define LISTEN_BACKLOG 5

serverT Server::_convertConfig(const ServerConfig &myConfig)
{
    serverT newConfig;

	if (!myConfig._listen.empty())
		newConfig.listen = myConfig._listen[0].port;
	else
		newConfig.listen = 8080;
	
	newConfig.root = myConfig._root;
	newConfig.clientMaxBodySize = myConfig._clientMaxBodySize;
    newConfig.errorPage = myConfig._errorPages;

	std::vector<LocationConfig> myLocs = myConfig._locations;

	for (size_t i = 0; i < myLocs.size(); i++)
	{
		locationsT newLoc;
		LocationConfig &curr = myLocs[i];

		newLoc.path = curr._path;
		if (!curr._index.empty())
			newLoc.index = curr._index[0];
		else
			newLoc.index = "";
		newLoc.upload_dir = curr._uploadPath;
		newLoc.methods.clear();
		if (curr._allowGet) 
			newLoc.methods.push_back("GET");
		if (curr._allowPost) 
			newLoc.methods.push_back("POST");
		if (curr._allowDelete) 
			newLoc.methods.push_back("DELETE");
		if (curr._autoIndex == 1)
			newLoc.autoindex = "on";
		else
			newLoc.autoindex = "off";
			
        newConfig.locations[curr._path] = newLoc;
    }

    return newConfig;
}

/**
 * @brief Constructeur : Initialise le serveur et vide le tableau de pollfd.
 */
Server::Server(const std::vector<ServerConfig> &configs) : _configs(configs) 
{
    for (int i = 0; i <= MAX_CLIENTS; ++i)
    {
        _fds[i].fd = -1;         // -1 signifie que le slot est libre
        _fds[i].events = POLLIN; // Par défaut, on écoute (lecture)
    }
}

/**
 * @brief Crée et configure le socket principal du serveur (Socket, Bind, Listen).
 * @return Le descripteur de fichier (FD) du serveur ou -1 en cas d'erreur.
 */
int Server::_createServerSocket(int port)
{
	// 1. Création du socket
	// AF_INET 		: Utilisation de l'IPv4 (Internet Protocol v4)
	// SOCK_STREAM	: Utilisation du protocole TCP
	// 0			: Protocole par défaut
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) 
		return (-1);

	// 2. Mode non-bloquant
	// Par défaut, un socket bloque le programme s'il n'y a rien à lire.
	// C'est ce qui permet au server de gérer 1000 clients avec 1 seul thread.
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "Error : Failed to set non-blocking mode on server socket" << std::endl;
		close (fd);
		return (-1);
	}
	
	// 3. Reuse adress
	// Quand le serveur est coupé, le port reste en état de "TIME-WAIT" pendant quelques minutes.
	// SO_REUSEADDR force la reprise du port 8080 immédiatement sans avoir*
	// l'erreur "Adress already in use".
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// 4. Préparation de l'adresse
	// htons(Host TO Network Short) : Convertit le port dans le format compris par le réseau.
	// INADRR_ANY : Ecoute sur toutes les interfaces (wifi, Ethernet, Localhost, etc)
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	// 5. Bind (assignation)
	// Réservation de l'adresse pour le socket
	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		std::cerr << "Error: Failed to bind to port " << port << std::endl;
		close(fd);
		return (-1);
	}
	
	// 6. Listen
	// Le socket passe en mode "passif".
	// Il ne chechera pas à se connecter, il attendra les connexions.
	// LISTEN_BACKLOG : taille de la file d'attente.
	if (listen(fd, LISTEN_BACKLOG) == -1)
	{
		std::cerr << "Error: Failed to listen on socket" << std::endl;
		close (fd);
		return (-1);
	}

	// Le socket est prêt, on retourne son numéro au serveur
	return fd;
}

/**
 * @brief Wrapper pour la fonction système accept().
 */
int Server::_acceptClient(int server_fd)
{
	// 1. "Carte d'identité" du client.
	// Contient l'IP du client et son port source 
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
	std::vector<int> openPorts;
	int				fdsIndex = 0;

	for (size_t i = 0; i < _configs.size(); i++)
	{
		int	currentPort = _configs[i]._listen[0].port;
		bool	portExists = false;

		for (size_t j = 0;j < openPorts.size(); j++)
		{
			if (openPorts[j] == currentPort)
			{
				portExists = true;
				break;
			}
		}
		if (portExists)
			continue;
		int fd = _createServerSocket(currentPort);
		if (fd == -1)
		{
			std::cerr << "[ERROR] Failed to create socket for port" << currentPort << std::endl;
			continue;
		}

		_fds[fdsIndex].fd = fd;
		_fds[fdsIndex].events = POLLIN;
		_fds[fdsIndex].revents = 0;
		_serverSockets[fd] = currentPort;
		openPorts.push_back(currentPort);
		fdsIndex++;

		std::cout << "[INFO] Listening on port " << currentPort << std::endl;
	}

	_nbListeningSockets = fdsIndex;

	std::cout << "[SUCCESS] Server setup complete. Listening on " 
	<< _nbListeningSockets << " ports." << std::endl;
}

/**
 * @brief Boucle principale (Heartbeat). Utilise poll() pour surveiller les événements.
 */
void Server::run()
{
	while(true)
	{
		int ret = poll(_fds, MAX_CLIENTS + _nbListeningSockets, 1000);
		if (ret < 0)
			break;
			
		for (int i = 0; i < _nbListeningSockets; i++)
		{
			if (_fds[i].revents & POLLIN)
				_acceptNewConnection(_fds[i].fd);	
		}
	
		for (int i = _nbListeningSockets; i <= MAX_CLIENTS + _nbListeningSockets; i++)
		{
			if (_fds[i].fd != -1 && _fds[i].revents != 0)
				_handleClientActivity(i);
		}
		
		for (int i = _nbListeningSockets; i <= MAX_CLIENTS + _nbListeningSockets; i++)
		{
			if (_fds[i].fd >= 0)
			{
				time_t	now = time(NULL);
				double diff = difftime(now, _clients[_fds[i].fd].lastTime);

				if (diff > 60)
				{
					if (i < 0)
						std::cout << "[TIMEOUT] Client " << _fds[i].fd << " disconnected (inactive)." << std::endl;
					_closeConnection(i);
				}
			}
		}
	}
}

void Server::_acceptNewConnection(int serverFd)
{
	struct sockaddr_in  clientAddr;
	socklen_t           clientLen = sizeof(clientAddr);

	int clientFd = accept(serverFd, (struct sockaddr *)&clientAddr, &clientLen);
    
	if (clientFd < 0) {
		std::cerr << "[ERROR] Accept failed" << std::endl;
		return;
	}

	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1) 
	{
		std::cerr << "[ERROR] Failed to set non-blocking mode on client FD" << std::endl;
		close(clientFd);
		return;
	}

    for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (_fds[i].fd == -1)
		{
			_fds[i].fd = clientFd;
			_fds[i].events = POLLIN;
			_fds[i].revents = 0;

            int portConnecte = _serverSockets[serverFd];
			
            _clients[clientFd] = Client(clientFd, portConnecte);

			std::cout << "[CONNEXION] New client connected on port " << portConnecte << " (FD: " << clientFd << ")" << std::endl;
			return;
		}
	}

	std::cerr << "[ERROR] Server full. Connection rejected." << std::endl;
	close(clientFd);
}

void Server::_handleClientActivity(int i)
{
	int fd = _fds[i].fd;
	if (_clients.find(fd) == _clients.end())
		return;
    
	Client &client = _clients[fd];
	int clientPort = client.getServerPort();
	
	ServerConfig &currentConfig = _configs[0]; 

	for (size_t j = 0; j < _configs.size(); j++)
	{
		if (_configs[j]._listen[0].port == clientPort)
		{
			currentConfig = _configs[j];
			break;
		}
	}
	
	try 
	{
		if (_fds[i].revents & POLLIN)
		{
			serverT mateConf = _convertConfig(currentConfig);
			client.handleRead(mateConf);
		}

		if ((_fds[i].revents & POLLOUT) && client.isReadyToWrite)
		{
			client.handleWrite();
		}

		if (client.isReadyToWrite)
			_fds[i].events = POLLIN | POLLOUT;
		else
			_fds[i].events = POLLIN;
	}
	catch (std::exception &e)
	{
		std::cerr << "[INFO] Client error: " << e.what() << " (FD: " << fd << ")" << std::endl;
		_closeConnection(i);
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
void Server::_enableWriting(int i)
{
	_fds[i].events = POLLIN | POLLOUT;
}

/**
 * @brief Repasse le socket en mode Lecture seule.
 * Réinitialise le masque sur POLLIN uniquement.
 * Évite l'attente active (busy-waiting) une fois l'envoi terminé.
 */
void Server::_disableWriting(int i)
{
    _fds[i].events = POLLIN;
}