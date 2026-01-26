/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Delete.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 11:55:23 by mlaussel          #+#    #+#             */
/*   Updated: 2026/01/26 09:45:38 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Delete.hpp"
#include "Error.hpp"
#include <unistd.h>   //stat() access()
#include <sys/stat.h> //struct stat
#include <unistd.h> //unlink()

/**
 * @brief `Find the best matching location for the requested URL.`
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
int foundLocationDelete(request &request, serverT &serverConfig, responseT &response)
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
                response.location = it->second;
                response.infos.loc = true;
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
bool checkIsDelete(request &request, responseT &response)
{
    for (size_t i = 0; i < response.location.methods.size(); ++i)
    {
        if (response.location.methods[i] == request._method)
        {
            response.infos.get = true;
            return (true);
        }
    }
    return (false);
}

/**
 * @brief `build the file path`
 *
 */
void pathBuildDelete(responseT &response, serverT &serverConfig, request &request)
{
    (void)serverConfig;
    if (response.location.upload_dir != "")
        response.path = response.location.upload_dir + request._url;
}

/**
 * @brief `check if the file exist and if we can access before delete with unlink`
 *
 */
void existFileDelete(responseT &response, serverT &serverConfig)
{
    struct stat test;
    if (stat(response.path.c_str(), &test) == -1)
        errorCode(response, serverConfig, 404);
    else
    {
        if (access(response.path.c_str(), R_OK) == -1)
        {
            errorCode(response, serverConfig, 403);
            return;
        }
        if (unlink(response.path.c_str()) != 0)
        {
            errorCode(response, serverConfig, 500);
            return;
        }
        response.code = 204;
    }
}

/**
 * @brief `DELETE method main`
 *
 *
 * step 1 : find the good location
 *
 * step 2 : check if method is in server
 *
 * step 3 : build file path
 *
 * step 4 : check if the file exist and if we can access before delete
 *
 * @return 1 if problem, else 0
 */
int deleteMain(request &request, responseT &response, serverT &serverConfig)
{
    // step 1 : find the good location
    if (foundLocationDelete(request, serverConfig, response) == false)
    {
        std::cout << "Error: no location found\n";
        return (1);
    }

    // step 2 : check if method is in server
    if (checkIsDelete(request, response) == false)
    {
         errorCode(response, serverConfig, 405);
        return (0);
    }

    // step 3 : build file path
    pathBuildDelete(response, serverConfig, request);

    //step 4 : check if the file exist and if we can access before delete
    existFileDelete(response, serverConfig);

   return (0);

}