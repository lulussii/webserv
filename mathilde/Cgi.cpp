/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 11:18:36 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/02 12:46:40 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"


bool isCgi(request &request)
{
    size_t cgi = request._url.find("/cgi/bin/");
    if (cgi != std::string::npos)
        return (true);
    
    cgi = request._url.find(".cgi");
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

