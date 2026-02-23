/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 10:01:08 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/23 09:49:08 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Chunked.hpp"
#include <sstream>
#include <stdlib.h>

Client::Client()
	: fd(-1), serverPort(-1), lastTime(time(NULL)), contentLength(0),
	  headersReceived(false), requestComplete(false), isReadyToWrite(false)
{
}

Client::Client(int fd, int port)
	: fd(fd), serverPort(port), lastTime(time(NULL)), contentLength(0),
	  headersReceived(false), requestComplete(false), isReadyToWrite(false)
{
	cgiClient.contentLenght = "";
    cgiClient.contentType = "";
    cgiClient.method = "";
    cgiClient.queryString = "";
    cgiClient.scriptPath = "";
    cgiClient.binaryPath = "";
    cgiClient.serverName = "";
    cgiClient.serverPort = "";
    cgiClient.gatewayInterface = "";
    cgiClient.serverProtocol = "";
    cgiClient.body = "";
    cgiClient.code = "200";
    cgiClient.isCgi= false;
    cgiClient.writePipe[0] = -1;
    cgiClient.writePipe[1] = -1;
    cgiClient.readPipe[0] = -1;
    cgiClient.readPipe[1] = -1;
    cgiClient.writeBuffer = "";
    cgiClient.readBuffer = "";
    cgiClient.writing = false;
    cgiClient.reading = false;
    cgiClient.pid = -1;

    cgiClient.errorTxt["200"] = "OK";
    cgiClient.errorTxt["201"] = "Created";
    cgiClient.errorTxt["204"] = "No Content";
    cgiClient.errorTxt["400"] = "Bad Request";
    cgiClient.errorTxt["403"] = "Forbidden";
    cgiClient.errorTxt["404"] = "Not Found";
    cgiClient.errorTxt["405"] = "Method Not Allowed";
    cgiClient.errorTxt["413"] = "Content Too Large";
    cgiClient.errorTxt["500"] = "Internal Server Error";
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

void Client::processRequest(serverT &serverConfig, request &request, responseT &response)
{
	// method GET
	if (request._method == "GET" && response.code == 200)
    {
		int errorValue = getMain(request, response, serverConfig);
        if (errorValue == 404)
            errorCode(response, serverConfig, 404);
        else if (errorValue == 403)
            errorCode(response, serverConfig, 403);
		else if (errorValue == 500)
            errorCode(response, serverConfig, 403);
    }
	
	// method POST
    if (request._method == "POST" && response.code == 200)
        postMain(request, response, serverConfig);

    // method DELETE
    if (request._method == "DELETE" && response.code == 200)
        deleteMain(request, response, serverConfig);

    // step response
    if (response.cgi == false || response.infos.error == true)
		responseMain(request, response);

	writeBuffer = response.response;
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

void Client::requestLine(request &request)
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

		request.lineRequest = readBuffer;
	}
}

void Client::handleRead(serverT &serverConfig, request &request, responseT &response)
{
	if (requestComplete)
		processRequest(serverConfig, request, response);
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
