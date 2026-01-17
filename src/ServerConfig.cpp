/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 09:53:30 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/14 12:45:52 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

ServerConfig::ServerConfig()
{
	this->_port = 0;
	this->_host = "127.0.0.1";
	this->_root = "";
	this->_clientMaxBodySize = 1000000;
	this->_serverName = "";
}

std::vector<ServerConfig> ConfigParser::getServers() const
{
	return (this->_servers);
}

responseT response;
    utilsConfigT utils;
    serverT serverConfig;
    request request;
    parsingT p;
    
    // step 0 : init
    initMain(request, response, serverConfig);
    
    // step 1 : request parsing

    if (requestMain(request, p) == 1)
        return;
    // debug1(request);

    // step 2 : config file
    
    if (configMain(serverConfig, utils) == 1)
        return;
    // debug2(serverConfig, utils);

    // step 3 : method GET
    if (request._method == "GET")
    {
        if (getMain(request, response, serverConfig) == 1)
            return;
        // debug3(response, 1);
    }
    

    // step 4 : method POST
    if (request._method == "POST")
    {
        if (postMain(request, response, serverConfig) == 1)
            return;
        // debug4(response, 1);
    }
    
    // step  5 : response
    if (responseMain(request, response) == 1)
	{return ;}

    // debug5(response);
	c.reponse = response.response; */