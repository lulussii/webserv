/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 15:14:41 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/02 13:45:40 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Init.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"
#include "Cgi.hpp"
#include "Get.hpp"
#include "Post.hpp"
#include "Delete.hpp"
#include "Error.hpp"

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

void debug2(serverT &serverConfig, utilsConfigT &utils)
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

void debug3(responseT &response, int i)
{
    std::cout << "\n--- [GET] ---\n";
    if (i == 1)
    {
        std::cout << "location : [" << response.location.path << "]" << std::endl;
        for (size_t i = 0; i < response.location.methods.size(); ++i)
            std::cout << "  Methods: [" << response.location.methods[i] << "] ";
        std::cout << std::endl;
        if (!response.location.index.empty())
            std::cout << "  Index: [" << response.location.index << "]" << std::endl;

        if (!response.location.autoindex.empty())
            std::cout << "  Autoindex: [" << response.location.autoindex << "]" << std::endl;

        if (!response.location.upload_dir.empty())
            std::cout << "  Upload dir: [" << response.location.upload_dir << "]" << std::endl;
    }
    std::cout << "file path : [" << response.path << "]\n";
}

void debug4(responseT &response, int i)
{
    std::cout << "\n--- [POST] ---\n";
    if (i == 1)
    {
        std::cout << "location : [" << response.location.path << "]" << std::endl;
        for (size_t i = 0; i < response.location.methods.size(); ++i)
            std::cout << "  Methods: [" << response.location.methods[i] << "] ";
        std::cout << std::endl;
        if (!response.location.index.empty())
            std::cout << "  Index: [" << response.location.index << "]" << std::endl;

        if (!response.location.autoindex.empty())
            std::cout << "  Autoindex: [" << response.location.autoindex << "]" << std::endl;

        if (!response.location.upload_dir.empty())
            std::cout << "  Upload dir: [" << response.location.upload_dir << "]" << std::endl;
    }
    std::cout << "path new file : [" << response.post.path << "]\n";
    if (response.infos.repository == true)
        std::cout << "REPO OK\n";
    std::cout << "Body : \n[" << response.body << "]\n";
    // std::cout << "Content Lenght : " << response.contentLen << std::endl;
    // std::cout << "Content Type : " << response.contentType << std::endl;
}

void debug5(responseT &response, int i)
{
    std::cout << "\n--- [DELETE] ---\n";
    if (i == 1)
    {
        std::cout << "location : [" << response.location.path << "]" << std::endl;
        for (size_t i = 0; i < response.location.methods.size(); ++i)
            std::cout << "  Methods: [" << response.location.methods[i] << "] ";
        std::cout << std::endl;
        if (!response.location.index.empty())
            std::cout << "  Index: [" << response.location.index << "]" << std::endl;

        if (!response.location.autoindex.empty())
            std::cout << "  Autoindex: [" << response.location.autoindex << "]" << std::endl;

        if (!response.location.upload_dir.empty())
            std::cout << "  Upload dir: [" << response.location.upload_dir << "]" << std::endl;
    }
    std::cout << "file path : [" << response.path << "]\n";
}

void debug6(responseT &response)
{
    std::cout << "\n--- [RESPONSE] ---\n";
    std::cout << response.response << "\n";
}

int main(void)
{
    responseT response;
    utilsConfigT utils;
    serverT serverConfig;
    request request;
    parsingT p;
    
    // step 0 : init
    initMain(request, response, serverConfig);
    
    // step 1 : request parsing

    if (requestMain(request, p) == 1)
        return (1);
    // debug1(request);

    // step 2 : config file
    
    if (configMain(serverConfig, utils) == 500)
    {
        errorCode(response, serverConfig, 500);
        return (1);
    }
    // debug2(serverConfig, utils);

   


    // step : CGI
    
    


    
    // step 3 : method GET
    if (request._method == "GET" && response.code == 200)
    {
        int errorValue = getMain(request, response, serverConfig);
        if (errorValue == 1)
            return (1);
        else if (errorValue == 404)
        {
            errorCode(response, serverConfig, 404);
        }
        else if (errorValue == 403)
        {
            errorCode(response, serverConfig, 403);
        }
        // debug3(response, 1);
    }
    

    // step 4 : method POST
    if (request._method == "POST" && response.code == 200)
    {
        postMain(request, response, serverConfig);
        // debug4(response, 1);
    }

    
    // step 5 : method DELETE
    if (request._method == "DELETE" && response.code == 200)
    {
        if (deleteMain(request, response, serverConfig) == 1)
            return (1);
        // debug5(response, 1);
    }
    
    // step  6 : response
    responseMain(request, response);
    debug6(response);
    
    return (0);
}