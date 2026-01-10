/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 15:14:41 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/10 12:37:02 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream> //cout
#include <unistd.h> //read
#include <fcntl.h>  //open
#include "Request.hpp"
#include "Config.hpp"
#include "Get.hpp"

void debug1(request &request)
{
    std::cout << "--- [REQUEST] ---\n";
    std::cout << "Method: [" << request._method << "] " << "\n";
    std::cout << "URL: [" << request._url << "] " << "\n";
    std::cout << "Version: [" << request._version << "] " << "\n";
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

void debug3(serverT &serverConfig, utilsConfigT &utils)
{
    (void)utils;
    std::cout << "\n--- [CONFIG] ---\n";
    // std::cout << "server : " << utils.server << std::endl;
    // std::cout << "\nlocations : " << utils.location << std::endl;

    std::cout << "  --Server--";
    std::cout << "\nlisten : " << serverConfig.listen;
    std::cout << "\nroot : " << serverConfig.root;
    std::cout << "\nerror_page : \n";
    for (std::map<int, std::string>::iterator it = serverConfig.errorPage.begin();
         it != serverConfig.errorPage.end(); it++)
    {
        std::cout << "      Code : " << it->first << "| Page : " << it->second << std::endl;
    }
    std::cout << "clientMaxBodySize : " << serverConfig.clientMaxBodySize << std::endl;

    std::cout << "  \n--Locations--\n";
    for (std::map<std::string, locationsT>::iterator it = serverConfig.locations.begin();
         it != serverConfig.locations.end(); ++it)
    {
        locationsT &location = it->second;
        std::cout << "location : [" << it->first << "]" << std::endl;
        for (size_t i = 0; i < location.methods.size(); ++i)
            std::cout << "  Methods: [" << location.methods[i] << "] ";
        std::cout << std::endl;
        if (!location.index.empty())
            std::cout << "  Index: [" << location.index << "]" << std::endl;

        if (!location.autoindex.empty())
            std::cout << "  Autoindex: [" << location.autoindex << "]" << std::endl;

        if (!location.upload_dir.empty())
            std::cout << "  Upload dir: [" << location.upload_dir << "]" << std::endl;
    }
}

void debug4(responseT &response)
{
    std::cout << "\n--- [GET] ---\n";
    if (response.infos.loc == true)
        std::cout << "GOOD location\n";
    if (response.infos.get == true)
        std::cout << "GOOD method\n";
    std::cout << "file path : [" << response.path << "]\n";
    if (response.infos.fileExist == false)
        std::cout << "File doesn't exist\n";
    else
    {
        if (response.infos.file == true)
            std::cout << "File exist\n";
        else
            std::cout << "It's a repo\n";
    }
    if (response.infos.read == false)
        std::cout << "new response : HTTP/1.1 403 Forbidden\r\n";
    else
        std::cout << "read ok";
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

    // step 2 : simple response
    responseT response;
    if (responseMain(request, response) == 1)
        return (1);
    debug2(response);

    // step 3 : config file
    utilsConfigT utils;
    serverT serverConfig;
    if (configMain(serverConfig, utils) == 1)
        return (1);
    // debug3(serverConfig, utils);

    // method GET
    if (getMain(request, response, serverConfig) == 1)
        return (1);
    debug4(response);
    return (0);
}