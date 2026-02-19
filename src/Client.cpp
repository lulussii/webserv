/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 10:01:08 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/19 13:56:54 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Chunked.hpp"
#include <sstream>

Client::Client()
	: fd(-1), serverPort(-1), lastTime(time(NULL)), contentLength(0),
	  headersReceived(false), requestComplete(false), isReadyToWrite(false)
{
}

Client::Client(int fd, int port)
	: fd(fd), serverPort(port), lastTime(time(NULL)), contentLength(0),
	  headersReceived(false), requestComplete(false), isReadyToWrite(false)
{
	reset();
}

int Client::getServerPort() const
{
	return this->serverPort;
}

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
	
	this->res = responseT();
	this->res.code = 0;
	this->res.contentLen = 0;
	this->res.contentType = "text/html";
	this->res.infos.error = false;
}

long Client::getContentLength(const std::string &buffer)
{
	size_t pos = buffer.find("Content-Length: ");
	if (pos == std::string::npos) return 0;
	
	size_t start = pos + 16;
	size_t end = buffer.find("\r\n", start);
	if (end == std::string::npos) return 0;

	return std::atol(buffer.substr(start, end - start).c_str());
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

	// std::cout << "\n[RESPONSE]\n" << res.response;
	writeBuffer = this->res.response;
	isReadyToWrite = true;

	std::cout << "[INFO] Response generated. Size " << writeBuffer.size() << " bytes." << std::endl;
}

void Client::parseHeaders(const std::string& rawHeaders, request &req)
{
	std::istringstream	stream(rawHeaders);
	std::string			line;

	std::getline(stream, line);

	while (std::getline(stream, line) && line != "\r")
	{
		size_t colonPos = line.find(':');
		if (colonPos != std::string::npos)
		{
			std::string key = line.substr(0, colonPos);
			std::string value = line.substr(colonPos + 1);

			if (!value.empty() && value[value.size() - 1] == '\r')
				value.erase(value.size() - 1);
			req.headers[key] = value;
		}
	}
}

void Client::handleRead(serverT &serverConfig)
{
	char tmpBuffer[4096];
	int bytesRead = recv(fd, tmpBuffer, sizeof(tmpBuffer), 0);
	
	if (bytesRead <= 0)
		throw std::runtime_error("Read error or client disconnected");

	readBuffer.append(tmpBuffer, bytesRead);
	lastTime = time(NULL);

	if (!headersReceived)
	{
		size_t	headerEnd = readBuffer.find("\r\n\r\n");
		if (headerEnd != std::string::npos)
		{
			headersReceived = true;
			bodyStartIndex = headerEnd + 4;

			parseHeaders(readBuffer.substr(0, headerEnd), this->req);

			this->isChunkedRequest = isChunked(this->req);
			if (!this->isChunkedRequest)
				this->contentLength = getContentLength(readBuffer);
		}
	}
	if (headersReceived)
	{
		if (this->isChunkedRequest)
		{
			if (readBuffer.find("0\r\n\r\n", bodyStartIndex) != std::string::npos)
			{
				this->req._body = readBuffer.substr(bodyStartIndex);
				chunkedParsing(this->req, this->res);
				requestComplete = true;
			}
		}	
		else
		{
			size_t	currentBodySize = readBuffer.size() - bodyStartIndex;
			if (currentBodySize >= (size_t)contentLength)
			{
				this->req._body = readBuffer.substr(bodyStartIndex, contentLength);
				requestComplete = true;
			}
		}
		if (requestComplete)
			processRequest(serverConfig);
	}
}

void Client::handleWrite()
{
	if (writeBuffer.empty()) return;

	int bytesSent = send(fd, writeBuffer.c_str(), writeBuffer.size(), MSG_NOSIGNAL);
	if (bytesSent > 0)
	{
		writeBuffer.erase(0, bytesSent);
		lastTime = time(NULL);
	}
	else if (bytesSent == -1)
	{
		throw std::runtime_error("Write error (Broken Pipe)");
	}

	if (writeBuffer.empty())
	{
		isReadyToWrite = false;
		reset();
	}
}