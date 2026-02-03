/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 11:18:36 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/03 11:03:49 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"

bool isCgi(request &request, cgi &cgi)
{
    size_t pos = request._url.find("?");
    if (pos != std::string::npos)
    {
        cgi.queryString = request._url.substr(pos + 1);
        request._url = request._url.substr(0, pos);
    }
    std::cout << "request url : " << request._url << std::endl;
    std::cout << "query : " << cgi.queryString << std::endl;

    
    pos = request._url.find(".php");
    if (pos != std::string::npos)
        return (true);

    return (false);
}

void handleCgi(request &request, cgi &cgi, serverT &serverConfig)
{
    cgi.method = request._method;

    if (cgi.method != "GET")
        cgi.queryString = "";

    if (cgi.method == "POST")
    {
        cgi.contentLenght = request.contentLenght;

        std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");
        if (it != request.headers.end())
            cgi.contentType = it->second;
    }

    cgi.scriptPath = serverConfig.root;

    cgi.serverName = "localhost";

    cgi.serverPort = serverConfig.listen;
}

int cgiMain(request &request, cgi &cgi, serverT &serverConfig)
{
    
    if (isCgi(request, cgi) == false)
    {
        std::cout << "FALSE " << std::endl;
        return (0);
    }

    handleCgi(request, cgi, serverConfig);

    return (0);
}