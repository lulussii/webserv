/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 11:18:05 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/03 16:25:43 by mathildelau      ###   ########.fr       */
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
};

int cgiMain(request &request, cgi &cgi, serverT &serverConfig, responseT &response);
void handleCgi(request &request, cgi &cgi, serverT &serverConfig, responseT &response, Multipart &m);

#endif