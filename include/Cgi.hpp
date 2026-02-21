/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 11:18:05 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/21 11:59:42 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include "Request.hpp"
#include "Config.hpp"

typedef struct cgi
{
    
    //env part
    std::string method;
    std::string queryString;
    std::string contentType;
    std::string contentLenght;
    std::string gatewayInterface;
    std::string serverProtocol;
    std::string serverName;
    std::string serverPort;
    std::string code;
    std::string scriptPath;
    std::string binaryPath;
    
    std::string headers;
    std::string body;
    
    std::string response;
    
    std::map<std::string, std::string> errorTxt;
    
    // poll part
    bool isCgi;
    int writePipe[2];
    int readPipe[2];
    std::string writeBuffer;
    std::string readBuffer;
    bool writing;
    bool reading;
    pid_t pid;
    int clientFd;
} cgi ;

void cgiMain(request &request, cgi &cgi, serverT &serverConfig, responseT &response);
void handleCgi(request &request, cgi &cgi, serverT &serverConfig, responseT &response, Multipart &m);
int cgiPipe(cgi &cgi);
void parsStdout(cgi &cgi);
void buildCgiResponse(cgi &cgi, responseT &response);
bool isCgi(request &request, cgi &cgi, responseT &response, serverT &serverConfig);
int accessCgi(cgi &cgi);



#endif