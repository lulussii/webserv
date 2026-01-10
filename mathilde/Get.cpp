/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 15:38:59 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/10 12:31:11 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Get.hpp"
#include <unistd.h> //stat() access()
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Find the best matching location for the requested URL.
 *
 * This function iterates over all configured locations of the server and
 * tries to find which location matches the request URL.
 *
 * A location matches if the request URL starts with the location path.
 * If multiple locations match, the most specific one is selected,
 * meaning the location with the longest path.
 *
 * Example:
 *  - URL: /upload/file.txt
 *  - Matching locations: "/" and "/upload"
 *  - Selected location: "/upload" (longest match)
 *
 * If a matching location is found, a pointer to this location is stored
 * in the response structure and a flag is set to indicate success.
 *
 * This function only determines the correct location.
 *
 * @param request The parsed HTTP request containing the requested URL.
 * @param serverConfig The server configuration holding all locations.
 * @param response The response structure where the matched location is stored.
 *
 * @return true if a matching location is found, false otherwise.
 */
int foundLocation(request &request, serverT &serverConfig, responseT &response)
{
    size_t bestLen = 0;
    bool found = false;

    for (std::map<std::string, locationsT>::iterator it = serverConfig.locations.begin();
         it != serverConfig.locations.end(); ++it)
    {
        const std::string &locPath = it->first;

        // URL must be at least as long as location path
        if (request._url.size() < locPath.size())
            continue;

        // Check if URL starts with location path
        if (request._url.compare(0, locPath.size(), locPath) == 0)
        {
            // Keep the most specific (longest) match
            if (locPath.size() > bestLen)
            {
                bestLen = locPath.size();
                response.location = &it->second;
                response.loc = true;
                found = true;
            }
        }
    }
    return (found);
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
        if (response.location->methods[i] == request._method)
        {
            response.get = true;
            return (true);
        }
    }
    return (false);
}

/**
 * @brief `build the file path`
 *
 */
void pathBuild(responseT &response, serverT &serverConfig, request &request)
{
    if (request._url == "/")
        response.path = serverConfig.root + response.location->index;
    else
        // response.path = serverConfig.root + response.location->index;
        response.path = serverConfig.root + request._url;
}

/**
 * @brief `check if the file exist and if it's a file or something else`
 *
 */
void existFile(responseT &response)
{
    struct stat test;
    if (stat(response.path.c_str(), &test) == -1)
        response.fileExist = false;
    else
    {
        response.fileExist = true;
        if (S_ISREG(test.st_mode))
        {
            response.file = true;
            response.repository = false;
        }
        else
        {
            response.file = false;
            response.repository = true;
        }
    }
}

/**
 * @brief `check the access of the file`
 *
 */
void    accessFile(responseT &response)
{
    if (access(response.path.c_str(), R_OK) == -1)
    {
        response.read = false;
    }
    else 
        response.read = true;
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
    pathBuild(response, serverConfig, request);

    // step 4 : check is the file exist
    existFile(response);

    //step 5 : access to the file
    accessFile(response);

    return (0);
}