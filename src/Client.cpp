/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 10:01:08 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/29 10:57:33 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Request.hpp"
#include "Get.hpp"
#include "Response.hpp"
#include "Init.hpp"
#include <sys/types.h>
#include "Error.hpp"
#include <sys/socket.h>
#include "Delete.hpp"
#include "Post.hpp"

/* ----- CONSTRUCTORS ----- */

Client::Client() 
	:	fd(-1),
		serverPort(-1),
		lastTime(time(NULL)),
		contentLength(0),
		headersReceived(false),
		requestComplete(false),
		isReadyToWrite(false)
{}

Client::Client(int fd, int port) 
	:	fd(fd),
		serverPort(port),
		lastTime(time(NULL)),
		contentLength(0),
		headersReceived(false),
		requestComplete(false),
		isReadyToWrite(false)
{
	reset();
}

int Client::getServerPort() const 
{
    return this->serverPort;
}

/**
 * @brief Cherche "Content-Length" dans l'en-tête HTTP.
 * Sert à savoir combien d'octets de BODY sont attendus après les headers.
 * @param buffer tout ce qui est reçu pour l'instant
 * @return la taille du body, 0 si pas trouvé ou pas de body (get).
 */
long	Client::getContentLength(const std::string &buffer)
{
	// 1. Recherche de la clé standard HTTP.
	// Si pas trouvé (ex: GET classique sans body), retourne 0.
	size_t	pos = buffer.find("Content-Length: ");
	if (pos == std::string::npos)
		return (0);

	// 2. Placement juste après "Content-Length: "
	size_t	start = pos + 16;
	
	// 3. Recherhce de la fin de la ligne (\r\n) pour isoler le nombre
	size_t	end = buffer.find("\r\n", start);
	if (end == std::string::npos)
		return (0);

	// 4. Découpe de la string et conversion en long
	std::string numStr = buffer.substr(start, end - start);

	return (std::atol(numStr.c_str()));	
}

/**
 * @brief Remet le client à zéro pour traiter une nouvelle requête sur la même connexion. 
 */
void Client::reset()
{
	// 1. Reset des buffers 
	readBuffer.clear();
	writeBuffer.clear();

	// 2. Reset des drapeaux d'état
	headersReceived = false;
	contentLength = 0;
	requestComplete = false;
	isReadyToWrite = false;

	// 3. Reset du timer à 0
	lastTime = time(NULL);

	// Reset de la requête
	this->req = request();

	// Nettoyage manuel de la structure de réponse
	this->res.code = 0;  
	this->res.contentLen = 0;
	this->res.body.clear();
	this->res.response.clear();
	this->res.path.clear();
	this->res.contentType = "text/html"; //valeur par défaut

	// Reset des booléens internes
    this->res.infos.error = false;
    this->res.infos.fileExist = false;
}

void	Client::processRequest(serverT &serverConfig)
{
	std::cout << "[INFO] Request complete. Processing..." << std::endl;

	parsingT	p;
	p.line = readBuffer;

	this->req = request();
	this->res = responseT();

	initMain(this->req, this->res, serverConfig);
	
	if (requestMain(this->req, p) == 1)
	{
		std::cerr << "[ERROR] Parsing failed -> 400  Bad Request" << std::endl;
		errorCode(this->res, serverConfig, 400);
	}
	else
	{
		if (this->req._method == "GET")
			getMain(this->req, this->res, serverConfig);
		else if (this->req._method == "POST")
			postMain(this->req, this->res, serverConfig);
		else if (this->req._method == "DELETE")
		{
			if (deleteMain(this->req, this->res, serverConfig) == 1  && this->res.code == 200)
				errorCode(this->res, serverConfig, 500);
		}
		else
		{
			std::cerr << "[ERROR] Unknown method : " << this->req._method << std::endl;
			errorCode(this->res, serverConfig, 501);
		}
	}
	

	responseMain(this->req, this->res);
	writeBuffer = this->res.response;
	isReadyToWrite = true;

	std::cout << "[INFO] Response generated. Size " << writeBuffer.size() << " bytes." << std::endl;
}

/**
 * @brief Lit les données entrantes (paquets TCP), les accumule dans le buffer de lecture,
 * et vérifie si la requête HTTP et entièrement reçue (Headers + Body) avant de lancer le traitement. 
 */
void	Client::handleRead(serverT &serverConfig)
{

	// 1. Lecture du socket
	char	tmpBuffer[4096];
	int		bytesRead = recv(fd, tmpBuffer, 4096, 0);
	if (bytesRead < 0)
		return;

	if (bytesRead == 0)
		std::runtime_error("Client disconnected");

	// 2. Accumulation
	// Ajout du nouveau morceau à la suite de ce qu'il y a déjà.
	readBuffer.append(tmpBuffer, bytesRead);
	lastTime = time(NULL);

	// 3. Analyse du header
	// Rien ne se passe tant que les en-têtes ne sont pas reçues (\r\n\r\n).
	if (!headersReceived)
	{
		size_t	headerEnd = readBuffer.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			headersReceived = true;
			contentLength = getContentLength(readBuffer);
		}
	}

	// 4. Vérification
	if (headersReceived)
	{
		size_t	headerEnd = readBuffer.find("\r\n\r\n");
		size_t	totalExpecteLength = headerEnd + 4 + contentLength;

		if (readBuffer.size() >= totalExpecteLength)
		{
			requestComplete = true;
			processRequest(serverConfig);
		}
	}
}

void	Client::handleWrite()
{
	if (writeBuffer.empty())
		return ;

	// 1. Envoi
	// Tentative d'envoi du buffer
	// bytesSent contiendra le nombre d'octets réellement acceptés par le réseau.
	int	bytesSent = send(fd, writeBuffer.c_str(), writeBuffer.size(), 0);
	if (bytesSent > 0)
	{
		// 2. Nettoyage partiel
		// Si il y a 1000 octets et que 500 sont envoyés, 
		// on supprime les 500 premiers et on garde le reste pour le prochain tour.
		writeBuffer.erase(0, bytesSent);
		lastTime = time(NULL);
	}
	else if (bytesSent == -1)
		return ;

	if (writeBuffer.empty())
	{
		isReadyToWrite = false;
		std::cout << "[INFO] Response fully sent." << std::endl;
		reset();
	}
}