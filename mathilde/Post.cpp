/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 13:27:18 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/12 19:51:46 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Post.hpp"
#include "Error.hpp"
#include <string>     //to_string()
#include <unistd.h>   //stat() access()
#include <sys/stat.h> //struct stat
#include <dirent.h>

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
int foundLocationPost(request &request, serverT &serverConfig, responseT &response)
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
bool checkIsPost(request &request, responseT &response)
{
    for (size_t i = 0; i < response.location->methods.size(); ++i)
    {
        if (response.location->methods[i] == request._method)
        {
            response.infos.get = true;
            return (true);
        }
    }
    return (false);
}

/**
 * @brief `check if the content lenght < clientMaxBodySize, if it's not to big`
 *
 * @return true if ok, else false
 */
bool clientMaxBodySize(serverT &serverConfig, request &request)
{

    if (request.contentLenght <= static_cast<size_t>(serverConfig.clientMaxBodySize))
        return (true);
    return (false);
}

/**
 * @brief `check if the body of POST exist`
 *
 * @return true if ok, else false
 */
bool bodyExist(request &request)
{
    if (request._body.size() > 0)
        return (true);
    return (false);
}

/**
 * @brief `create random file with path in location`
 */
void createFileName(responseT &response)
{
    response.post->path = response.location->upload_dir + "/" + "upload_" + std::to_string(response.post->count) + ".txt";
}

/**
 * @brief `check repo`
 *
 * step 1 : exist else error 500
 * 
 * step 2 : is repo else error 500
 * 
 * step 3 : can access else error 403
 * 
 * step 4 : everything is OK
 */
void checkRepo(request &request, responseT &response, serverT &serverConfig)
{
    struct stat test;

    if (stat(response.location->upload_dir.c_str(), &test) == -1)
    {
        error500(response);
        return;
    }
    else if (!S_ISDIR(test.st_mode))
    {
        error500(response);
        return;
    }
    else if (access(response.location->upload_dir.c_str(), W_OK) == -1)
    {
        error403(response, serverConfig, request);
    }
    else
        response.infos.repository = true;
}

int postMain(request &request, responseT &response, serverT &serverConfig)
{
    // step 1 : find the good location
    if (foundLocationPost(request, serverConfig, response) == false)
    {
        std::cout << "Error: no location found\n";
        return (1);
    }

    // step 2 : check if method is in server
    if (checkIsPost(request, response) == false)
    {
        error405(response);
        return (1);
    }

    // step 3 : check client_max_body_size
    if (clientMaxBodySize(serverConfig, request) == false)
    {
        error413(response);
        return (1);
    }

    // step 4 : check if body exist
    if (bodyExist(request) == false)
    {
        error400(response);
        return (1);
    }
    
    // step 5 : create name file
    createFileName(response);

    // step 6 : check repo
    checkRepo(request, response, serverConfig);

    return (0);
}