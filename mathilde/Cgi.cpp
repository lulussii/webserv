/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 11:18:36 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/03 16:39:51 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"
#include "Multipart.hpp"
#include "Error.hpp"
#include <unistd.h>   //stat() access()
#include <sys/stat.h> //struct stat
#include <sstream> //std::stringstream

/**
 * @brief `check if cgi exist and allowed`
 *
 * step 1 : check if location allowed cgi
 *
 * step 2 : pars url to have test.php without query
 *
 * step 3 : build path (/Users/mathildelaussel/webserv/mathilde/server_files/test.php)
 *
 * step 4 : check if regular file, if yes, search if cgi extension is in path
 */
bool isCgi(request &request, cgi &cgi, responseT &response, serverT &serverConfig)
{
    std::string url = request._url;
    if (response.location.cgiBinary.empty() || response.location.cgiExtension.empty())
        return (false);

    size_t pos = url.find("?");
    if (pos != std::string::npos)
    {
        cgi.queryString = url.substr(pos + 1);
        url = url.substr(0, pos);
    }
    std::string path;
    if (url == "/")
        path = serverConfig.root + response.location.index;
    else
        path = serverConfig.root + url;

    struct stat test;
    if (stat(path.c_str(), &test) != -1 && S_ISREG(test.st_mode))
    {
        pos = path.find(response.location.cgiExtension);
        if (pos != std::string::npos)
        {
            cgi.binaryPath = response.location.cgiBinary;
            cgi.scriptPath = path;
            response.cgi = true;
            return (true);
        }
    }
    return (false);
}

/**
 * @brief `check binary and extension`
 *
 * step 1 : check can execute binary (X_OK) who is /Users/mathildelaussel/webserv/mathilde/server_files/cgi/php-cgi
 *
 * step 2 : check can read script (R_OK) who is /Users/mathildelaussel/webserv/mathilde/server_files/cgi/test.php
 */
int accessCgi(cgi &cgi)
{
    if (access(cgi.scriptPath.c_str(), R_OK) == -1)
        return (404);
    if (access(cgi.binaryPath.c_str(), X_OK) == -1)
        return (404);
    return (0);
}

/**
 * @brief `handle cgi env``
 *
 * step 1 - cgi.method : add method GET POST or DELETE
 *
 * step 3 - cgi.contentLenght
 *
 * step 4 - cgi.contentType
 *
 * step 5 - cgi.serverName
 *
 * step 6 - cgi.serverPort : listen in server (8080)
 *
 * step 7 - cgi.gatewayInterface
 *
 * step 8 - cgi.serverProtocol : request._version (HTTP/1.1)
 *
 */
void handleCgi(request &request, cgi &cgi, serverT &serverConfig, responseT &response, Multipart &m)
{
    if (request._method == "GET" || request._method == "POST")
        cgi.method = request._method;

    std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");
    if (it != request.headers.end())
        cgi.contentType = it->second;

    if (m.content.empty())
    {
        std::stringstream convert;
        convert << response.contentLen;
        cgi.contentLenght = convert.str();
    }
    else
    {
        std::stringstream convert;
        convert << m.content.size();
        cgi.contentLenght = convert.str();
    }

    it = request.headers.find("Host");
    if (it != request.headers.end())
        cgi.serverName = it->second;
        

    std::stringstream convert;
    convert << serverConfig.listen;
    cgi.serverPort = convert.str();

    cgi.gatewayInterface = "CGI/1.1";

    cgi.serverProtocol = request._version;

    std::cout << "CGI METHOD : " << cgi.method << std::endl;
    std::cout << "CGI QURRY : " << cgi.queryString << std::endl;
    std::cout << "CGI CONTENT LENGHT : " << cgi.contentLenght << std::endl;
    std::cout << "CGI CONTENT TYPE : " << cgi.contentType << std::endl;
    std::cout << "CGI SCRIPT PATH : " << cgi.scriptPath << std::endl;
    std::cout << "CGI BINARY PATH : " << cgi.binaryPath << std::endl;
    std::cout << "CGI SERVER NAME : " << cgi.serverName << std::endl;
    std::cout << "CGI SERVER PORT : " << cgi.serverPort << std::endl;
    std::cout << "CGI GATE WAY : " << cgi.gatewayInterface << std::endl;
    std::cout << "CGI SERVER PROTOCOL : " << cgi.serverProtocol << std::endl;
}

int cgiMain(request &request, cgi &cgi, serverT &serverConfig, responseT &response)
{
    if (isCgi(request, cgi, response, serverConfig) == false)
    {
        std::cout << "FALSE " << std::endl;
        return (0);
    }

    if (accessCgi(cgi) == 404)
    {
        errorCode(response, serverConfig, 404);
        return (0);
    }

    return (0);
}