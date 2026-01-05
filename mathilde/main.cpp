/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 15:14:41 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/05 13:34:25 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream> //cout
#include <unistd.h> //read
#include <fcntl.h>  //open
#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"

void debug1(request &request)
{
    std::cout << "--- [REQUEST] ---\n";
    std::cout << "Method: " << request._method << "\n";
    std::cout << "URL: " << request._url << "\n";
    std::cout << "Version: " << request._version << "\n";
    std::cout << "Headers:\n";
    for (std::map<std::string, std::string>::iterator it = request.headers.begin(); it != request.headers.end(); ++it)
        std::cout << "  " << it->first << ": " << it->second << "\n";
    if (!request._body.empty())
        std::cout << "Body:\n"
                  << request._body << "\n";
}

void debug2(responseT &response)
{
    std::cout << "\n--- [RESPONSE] ---\n";
    std::cout << response.response << "\n";
}

void debug3(serverT &serverConfig, locationsT &locationsConfig, utilsConfigT &utils)
{
    (void)locationsConfig;
    (void)utils;
    std::cout << "\n--- [CONFIG] ---\n";
    // std::cout << "server : " << utils.server << std::endl;
    // std::cout << "\nlocations : " << utils.location << std::endl;
    
    std::cout << "  --Server--";
    std::cout << "\nlisten : " << serverConfig.listen;
    std::cout << "\nroot : " << serverConfig.root;
    std::cout << "\nerror_page : \n";
    for (std::map<int, std::string>::iterator it = serverConfig.errorPage.begin() ;
        it != serverConfig.errorPage.end(); it++)
    {
        std::cout << "      Code : " << it->first << "| Page : " << it->second << std::endl;
    }
    std::cout << "clientMaxBodySize : " << serverConfig.clientMaxBodySize << std::endl;

    std::cout << "  \n--Locations--";
    // for (std::map<std::string, locationsT>::iterator it = serverConfig.locations.begin() ;
    // it != serverConfig.locations.end(); it++)
    // {
    //     std::cout << "      Name : " << it->first << "| path : " << it->second << std::endl;
    // }
}

int main(int argc, char **argv)
{
    (void)argv; // delete

    // step 0 : check number of args
    if (argc != 1)
    {
        std::cout << "Usage : ./webserv \n";
        return (1);
    }

    // step 1 : request parsing
    request request;
    parsingT p;
    if (requestMain(request, p) == 1)
        return (1);
    // debug1(request);

    // step 2 : response
    responseT response;
    if (responseMain(request, response) == 1)
        return (1);
    // debug2(response);

    // step 3 : config file
    utilsConfigT utils;
    serverT serverConfig;
    locationsT locationsConfig;
    if (configMain(serverConfig, locationsConfig, utils) == 1)
        return (1);
    debug3(serverConfig, locationsConfig, utils);
    return (0);
}