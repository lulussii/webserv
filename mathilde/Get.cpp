/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 15:38:59 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/06 17:49:36 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Get.hpp"

/**
 * @brief `check if the method in the request is in the server`
 *
 * loop on all locations to compare index
 *
 * @return true if found, else false
 */
int foundLocation(request &request, serverT &serverConfig, responseT &response)
{
    for (std::map<std::string, locationsT>::iterator it = serverConfig.locations.begin();
         it != serverConfig.locations.end(); ++it)
    {
        locationsT &location = it->second;
        for (size_t i = 0; i < location.methods.size(); ++i)
        {
            if (location.index == request._url)
            {
                response.location = &it->second;
                response.loc = true;
                return (true);
            }
        }
    }
    return (false);
}

/**
 * @brief `check if the method in the request is in the location we found`
 *
 * loop on method from location to compare if method is ok
 *
 * @return true if found, else false
 */
bool checkIsGet(request &request, responseT &response)
{
    for (size_t i = 0; i < response.location->methods.size(); ++i)
    {
        if(response.location->methods[i] == request._method)
        {
            response.get = true;
            return (true);
        }
    }
    return(false);
}


/**
 * @brief `build the file path`
 *
 */
void pathBuild(responseT &response, serverT &serverConfig)
{
    response.path = serverConfig.root + "/" + response.location->index;
}

/**
 * @brief `GET method main`
 *
 * step 1 : check if the method in the request is in the server
 *
 * step 2 : find the good location
 *
 * @return 1 if problem, else 0
 */
int getMain(request &request, responseT &response, serverT &serverConfig)
{
    (void)response;
    // step 1 : find the good location
    if (foundLocation(request, serverConfig, response) == false)
    {
        std::cout << "Error: no location found\n";
        return (1);
    }
    
    // step 2 : check if method is in server
    if (checkIsGet(request, response) == false)
    {
        std::cout << "Error: 405 Method Not Allowed\n";
        return (1);
    }

    // step 3 : build response path
    pathBuild(response, serverConfig);

    
    return (0);
}