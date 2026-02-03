/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 11:18:36 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/03 09:51:59 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"

bool isCgi(request &request)
{
    std::cout << request._url << std::endl;
    
    size_t cgi = request._url.find(".php");
    if (cgi != std::string::npos)
        return (true);

    return (false);
}

void addEnv(request &request, cgi &cgi, serverT &serverConfig)
{
    cgi.method = request._method;

    if (cgi.method == "GET")
    {
        size_t pos = request._url.find("?");
        if (pos != std::string::npos)
            cgi.queryString = request._url.substr(pos);
    }

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
