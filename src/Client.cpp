/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 10:01:08 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/16 10:28:05 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include "Init.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"
#include "Cgi.hpp"
#include "Get.hpp"
#include "Post.hpp"
#include "Delete.hpp"
#include "Error.hpp"

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
}

void Client::processRequest(serverT &serverConfig)
{
	std::cout << "[INFO] Request complete. Processing..." << std::endl;

	parsingT p;
	p.line = readBuffer;

	this->req = request();
	this->res = responseT();
	cgi cgi;

	initMain(this->req, this->res, cgi);

	// method GET
	requestMain(this->req, p, serverConfig, this->res, cgi);
	if (this->req._method == "GET" && this->res.code == 200)
    {
		int errorValue = getMain(this->req, this->res, serverConfig, cgi);
        if (errorValue == 404)
            errorCode(this->res, serverConfig, 404);
        else if (errorValue == 403)
            errorCode(this->res, serverConfig, 403);
		else if (errorValue == 500)
            errorCode(this->res, serverConfig, 403);
    }
	
	// step 4 : method POST
    if (this->req._method == "POST" && this->res.code == 200)
        postMain(this->req, this->res, serverConfig, cgi);

    // step 5 : method DELETE
    if (this->req._method == "DELETE" && this->res.code == 200)
        deleteMain(this->req, this->res, serverConfig);

    // step  6 : response
    if (this->res.cgi == false || this->res.infos.error == true)
	{
        responseMain(this->req, this->res);
	}

	std::cout << "\n[RESPONSE]\n" << res.response;

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
	if (bytesRead <= 0)
		throw std::runtime_error("Read error or client disconnected");

	readBuffer.append(tmpBuffer, bytesRead);
	lastTime = time(NULL);
	
	if (!headersReceived)
	{
		if (readBuffer.size() > 8192)
		{
			std::cout << "[SECURITY] Headers too large (" << readBuffer.size() << "). Closing." << std::endl;
			throw std::runtime_error("431 Request Header Fields Too Large");
		}
		size_t headerEnd = readBuffer.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			headersReceived = true;
			contentLength = getContentLength(readBuffer);
			bodyStartIndex = headerEnd + 4;
		}
	}
	if (headersReceived)
	{
		size_t currentBodySize = readBuffer.size() - bodyStartIndex;
		if (currentBodySize >= (size_t)contentLength)
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

	int bytesSent = send(fd, writeBuffer.c_str(), writeBuffer.size(), MSG_NOSIGNAL);
	if (bytesSent > 0)
	{
		writeBuffer.erase(0, bytesSent);
		lastTime = time(NULL);
	}
	else if (bytesSent == -1)
		throw std::runtime_error("Write error (Broken Pipe)");

	if (writeBuffer.empty())
	{
		isReadyToWrite = false;
		std::cout << "[INFO] Response fully sent." << std::endl;
		reset();
	}
}