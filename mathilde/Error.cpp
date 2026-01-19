/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 14:47:21 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/19 10:25:24 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Error.hpp"
#include "Get.hpp"

/**
 * @brief `search html body for error`
 *
 * if code is 404 so the new path is root/404.html
 * 
 * step 1 : add content type who is an html (404.html etc)
 * 
 * step 2 : search an error page in my config
 * 
 * step 3 : if there is the error page in my config 
 * (it != serverConfig.errorPage.end() because end it's the end of the map)
 * So if it is different from nothing, the end, it's that I found an error page
 * 
 * step 4 : if ok --> build response path to search error page html 
 * Exemple : serverConfig.root = /Users/mathildelaussel/server/ + it->second = 404.html
 * 
 * step  5 : read file to add the body html
 * 
 */
void errorCode(responseT &response, serverT &serverConfig, int code)
{
    response.code = code;
    response.infos.error = true;
    response.contentType += "text/html";
    
    std::map<int, std::string>::iterator it = serverConfig.errorPage.find(response.code);

    if (it != serverConfig.errorPage.end())
    {
        response.path = serverConfig.root + it->second;

        if (readFile(response) == 0 && !response.body.empty())
        {
            response.contentLen = response.body.size();
            return;
        }
    }
    response.body = "";
}
