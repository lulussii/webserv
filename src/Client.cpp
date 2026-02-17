/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 10:01:08 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/17 13:48:36 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

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

serverT* Client::_selectServerConfig(std::vector<serverT> &allConfigs)
{
	serverT *defaultConfig = NULL;

	for (size_t i = 0; i < allConfigs.size(); i++)
	{
		for (size_t p = 0; p < allConfigs[i].listens.size(); p++)
		{
			if (allConfigs[i].listens[p].port == this->serverPort)
			{
				if (defaultConfig == NULL) defaultConfig = &allConfigs[i];

				if (this->req.headers.count("Host") > 0)
				{
					std::string hostName = this->req.headers["Host"];
					
					if (!hostName.empty() && hostName[0] == ' ') hostName.erase(0, 1);
					size_t pos = hostName.find(':');
					if (pos != std::string::npos) hostName = hostName.substr(0, pos);
					if (!hostName.empty() && hostName[hostName.length() - 1] == '\r')
						hostName.erase(hostName.length() - 1);

					for (size_t k = 0; k < allConfigs[i].servernames.size(); k++)
					{
						if (allConfigs[i].servernames[k] == hostName)
						{
							return &allConfigs[i];
						}
					}
				}
			}
		}
	}
	if (defaultConfig) return defaultConfig;
	return &allConfigs[0];
}

void Client::_dispatchMethod(serverT &config, cgi &cgiInstance)
{
	if (this->res.code != 200) return;

	int errorValue = 0;

	if (this->req._method == "GET")
	{
		errorValue = getMain(this->req, this->res, config, cgiInstance);
		if (errorValue == 404 || errorValue == 403)
			errorCode(this->res, config, errorValue);
		else if (errorValue == 500)
			errorCode(this->res, config, 403);
	}
	else if (this->req._method == "POST")
	{
		postMain(this->req, this->res, config, cgiInstance);
	}
	else if (this->req._method == "DELETE")
	{
		deleteMain(this->req, this->res, config);
	}
}

void Client::processRequest(std::vector<serverT> &allConfigs)
{
	parsingT p;
	p.line = readBuffer;
	this->req = request();
	this->res = responseT();
	cgi cgiObj;

	initMain(this->req, this->res, cgiObj);

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
}

void Client::handleRead(std::vector<serverT> &allConfigs)
{
	char tmpBuffer[4096];
	int bytesRead = recv(fd, tmpBuffer, sizeof(tmpBuffer), 0);
	
	if (bytesRead <= 0)
		throw std::runtime_error("Read error or client disconnected");

	readBuffer.append(tmpBuffer, bytesRead);
	lastTime = time(NULL);

	if (!headersReceived)
	{
		if (readBuffer.size() > 8192)
			throw std::runtime_error("431 Request Header Fields Too Large");

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
			processRequest(allConfigs);
		}
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