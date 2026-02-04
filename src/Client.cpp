/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 10:01:08 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/04 15:29:51 by lserodon         ###   ########.fr       */
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
	: fd(-1),
	  serverPort(-1),
	  lastTime(time(NULL)),
	  contentLength(0),
	  headersReceived(false),
	  requestComplete(false),
	  isReadyToWrite(false)
{
}

Client::Client(int fd, int port)
	: fd(fd),
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
long Client::getContentLength(const std::string &buffer)
{
	size_t pos = buffer.find("Content-Length: ");
	if (pos == std::string::npos)
		return (0);
	size_t start = pos + 16;
	size_t end = buffer.find("\r\n", start);
	if (end == std::string::npos)
		return (0);
	std::string numStr = buffer.substr(start, end - start);

	return (std::atol(numStr.c_str()));
}

/**
 * @brief Remet le client à zéro pour traiter une nouvelle requête sur la même connexion.
 */
void Client::reset()
{
	readBuffer.clear();
	writeBuffer.clear();

	headersReceived = false;
	contentLength = 0;
	requestComplete = false;
	isReadyToWrite = false;

	lastTime = time(NULL);

	this->req = request();

	this->res.code = 0;
	this->res.contentLen = 0;
	this->res.body.clear();
	this->res.response.clear();
	this->res.path.clear();
	this->res.contentType = "text/html";

	this->res.infos.error = false;
	this->res.infos.fileExist = false;
}

void Client::processRequest(serverT &serverConfig)
{
	std::cout << "[INFO] Request complete. Processing..." << std::endl;

	parsingT p;
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
			if (deleteMain(this->req, this->res, serverConfig) == 1 && this->res.code == 200)
				errorCode(this->res, serverConfig, 500);
		}
	}

	responseMain(this->req, this->res);

	std::cout << "\n\nREPONSE\n"
			  << res.response;

	writeBuffer = this->res.response;
	isReadyToWrite = true;

	std::cout << "[INFO] Response generated. Size " << writeBuffer.size() << " bytes." << std::endl;
}

/**
 * @brief Lit les données entrantes (paquets TCP), les accumule dans le buffer de lecture,
 * et vérifie si la requête HTTP et entièrement reçue (Headers + Body) avant de lancer le traitement.
 */
void Client::handleRead(serverT &serverConfig)
{

	char tmpBuffer[4096];
	int bytesRead = recv(fd, tmpBuffer, 4096, 0);
	if (bytesRead < 0)
		return;

	if (bytesRead == 0)
		std::runtime_error("Client disconnected");
		
	readBuffer.append(tmpBuffer, bytesRead);
	lastTime = time(NULL);
	
	if (!headersReceived)
	{
		size_t headerEnd = readBuffer.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			headersReceived = true;
			contentLength = getContentLength(readBuffer);
		}
	}
	if (headersReceived)
	{
		size_t headerEnd = readBuffer.find("\r\n\r\n");
		size_t totalExpecteLength = headerEnd + 4 + contentLength;

		if (readBuffer.size() >= totalExpecteLength)
		{
			requestComplete = true;
			processRequest(serverConfig);
		}
	}
}

void Client::handleWrite()
{
	if (writeBuffer.empty())
		return;

	int bytesSent = send(fd, writeBuffer.c_str(), writeBuffer.size(), 0);
	if (bytesSent > 0)
	{
		writeBuffer.erase(0, bytesSent);
		lastTime = time(NULL);
	}
	else if (bytesSent == -1)
		return;

	if (writeBuffer.empty())
	{
		isReadyToWrite = false;
		std::cout << "[INFO] Response fully sent." << std::endl;
		reset();
	}
}