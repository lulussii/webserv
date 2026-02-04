/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 11:18:05 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/04 17:00:30 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include "Request.hpp"
#include "Config.hpp"

class cgi
{
public:
    std::string method;
    std::string queryString;
    std::string contentLenght;
    std::string contentType;
    std::string scriptPath;
    std::string binaryPath;
    std::string serverName;
    std::string serverPort;
    std::string gatewayInterface;
    std::string serverProtocol;
    std::string headers;
    std::string body;
    std::string code;
    std::string response;
    std::map<std::string, std::string> errorTxt;
};

int cgiMain(request &request, cgi &cgi, serverT &serverConfig, responseT &response);
void handleCgi(request &request, cgi &cgi, serverT &serverConfig, responseT &response, Multipart &m);
int cgiPipe(cgi &cgi, responseT &response);
void parsStdout(cgi &cgi);
void buildCgiResponse(cgi &cgi, responseT &response);

#endif