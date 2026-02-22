/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 11:18:36 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/22 19:11:43 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"
#include "Error.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include <unistd.h>   //stat() access() pipe() fork()
#include <sys/stat.h> //struct stat
#include <sstream>    //std::stringstream
#include <sys/wait.h> //waitpid
#include <vector>

/**
 * @brief `Check if the request should be handled by a CGI script`
 *
 * step 1 : verify that the location allows CGI (cgiBinary and cgiExtension are set)
 *
 * step 2 : parse URL to separate query string from script path
 *
 * step 3 : build the absolute script path using server root and URL (/Users/mathildelaussel/webserv/mathilde/server_files/test.php)
 *
 * step 4 : check if file exists and is a regular file
 *
 * step 5 : verify that the file extension matches the configured CGI extension
 *
 * step 6 : set CGI paths and response flag if CGI
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
        path = serverConfig.root + url.substr(1); // delete /

    struct stat test;
    if (stat(path.c_str(), &test) != -1 && S_ISREG(test.st_mode))
    {
        pos = path.rfind(response.location.cgiExtension);
        if (pos != std::string::npos)
        {
            cgi.binaryPath = response.location.cgiBinary;
            cgi.scriptPath = path;
            response.cgi = true;
            cgi.isCgi = true;
            return (true);
        }
    }
    return (false);
}

/**
 * @brief `Check file permissions for CGI execution`
 *
 * step 1 : check if the script exists (F_OK)
 *
 * step 2 : check if the script is readable (R_OK)
 *
 * step 3 : check if the CGI binary exists (F_OK)
 *
 * step 4 : check if the CGI binary is executable (X_OK)
 */
int accessCgi(cgi &cgi)
{
    if (access(cgi.scriptPath.c_str(), F_OK) == -1)
        return (404);
    if (access(cgi.scriptPath.c_str(), R_OK) == -1)
        return (403);
    if (access(cgi.binaryPath.c_str(), F_OK) == -1)
        return (404);
    if (access(cgi.binaryPath.c_str(), X_OK) == -1)
        return (403);
    return (0);
}

/**
 * @brief `Prepare CGI environment variables and request body`
 *
 * step 1 : set the request method (GET, POST, etc.)
 * step 2 : get Content-Type from request headers
 * step 3 : get Host from request headers
 * step 4 : set server port
 * step 5 : set gateway interface (CGI/1.1)
 * step 6 : set server protocol (HTTP version)
 * step 7 : set request body (either multipart content or raw body)
 * step 8 : calculate Content-Length
 * step 9 : set status code for CGI environment (REDIRECT_STATUS)
 *
 */
void handleCgi(request &request, cgi &cgi, serverT &serverConfig, responseT &response)
{
    if (request._method == "GET" || request._method == "POST")
        cgi.method = request._method;

    std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");
    if (it != request.headers.end())
        cgi.contentType = it->second;

    it = request.headers.find("Host");
    if (it != request.headers.end())
        cgi.serverName = it->second;

    std::stringstream convert;
    convert << serverConfig.listen;
    cgi.serverPort = convert.str();
    cgi.gatewayInterface = "CGI/1.1";

    cgi.serverProtocol = request._version;

    cgi.body = request._body;

    if (request._method != "GET")
    {
        std::stringstream length;
        length << cgi.body.size();
        cgi.contentLenght = length.str();
    }
    else
        cgi.contentLenght = "0";

    std::stringstream code;
    code << response.code;
    cgi.code = code.str();
}


/**
 * @brief `Parse CGI stdout into headers and body`
 *
 * step 1 : search for header/body separator (\r\n\r\n or \n\n)
 * step 2 : split response into headers and body
 * step 3 : extract Status: header if present, default to 200
 * step 4 : extract Content-Type header if present, default to text/plain
 */

void parsStdout(cgi &cgi)
{
    int space = 4;
    cgi.headers.clear();
    cgi.body.clear();

    size_t pos = cgi.response.find("\r\n\r\n");
    if (pos == std::string::npos)
    {
        space = 2;
        pos = cgi.response.find("\n\n");
    }

    if (pos != std::string::npos)
    {
        cgi.headers = cgi.response.substr(0, pos);
        cgi.body = cgi.response.substr(pos + space);
    }
    else
        cgi.body = cgi.response;
    pos = cgi.headers.find("Status:");
    if (pos != std::string::npos)
    {
        std::string tmp = cgi.headers.substr(pos);
        cgi.code = tmp.substr(8, 3);
    }
    else
        cgi.code = "200";

    pos = cgi.headers.find("Content-Type:");
    if (pos != std::string::npos)
    {
        std::string tmp = cgi.headers.substr(pos);
        size_t end = tmp.find("\r\n");
        if (end == std::string::npos)
            end = tmp.find("\n");
        cgi.contentType = tmp.substr(14, end - 14);
    }
    else
        cgi.contentType = "text/plain";
}

/**
 * @brief `Build the HTTP response from CGI output`
 *
 * step 1 : start response with HTTP version and CGI status code
 *
 * step 2 : add status text if available
 *
 * step 3 : determine if Content-Length should be 0 (DELETE, 413) or size of CGI body
 *
 * step 4 : add Content-Type header
 *
 * step 5 : add empty line to separate headers from body
 *
 * step 6 : append CGI body if applicable
 */

void buildCgiResponse(cgi &cgi, responseT &response)
{
    response.response.clear();

    response.response += cgi.serverProtocol + " " + cgi.code;

    std::map<std::string, std::string>::iterator it = cgi.errorTxt.find(cgi.code);
    if (it != cgi.errorTxt.end())
        response.response += " " + it->second;

    if (cgi.method == "DELETE" || cgi.code == "413")
    {
        response.response += "\r\n";
        response.response += "Content-Length: 0";
    }
    else
    {
        response.response += "\r\n";
        std::stringstream length;
        length << cgi.response.size();
        // length << cgi.body.size();
        response.response += "Content-Length: " + length.str();
    }
    response.response += "\r\n";

    response.response += "Content-Type: " + cgi.contentType;
    response.response += "\r\n";

    response.response += "\r\n";

    // response.response += cgi.body;
    response.response += cgi.response;
    
}

