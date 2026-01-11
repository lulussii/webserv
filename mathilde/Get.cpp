/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 15:38:59 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/11 11:42:44 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Get.hpp"
#include <unistd.h> //stat() access()
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>   //to_string()
#include <fcntl.h>  //open
#include <unistd.h> //read

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
bool checkIsGet(request &request, responseT &response)
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
void existFile(responseT &response, request &request)
{
    struct stat test;
    if (stat(response.path.c_str(), &test) == -1)
    {
        response.infos.fileExist = false;
        response.code = 404;
        response.response = "HTTP/1.1 " + std::to_string(response.code) + " Forbidden\r\n";
        response.response += "Content-Length: " + std::to_string(request._body.size());
        response.response += "Content-Type: text/html";
        response.response += "\r\n";
        response.response += "\r\n";
        request._body = "<html><body>404 Forbidden</body></html>";
        response.response += request._body;
    }
    else
    {
        response.infos.fileExist = true;
        if (S_ISREG(test.st_mode))
        {
            response.infos.file = true;
            response.infos.repository = false;
        }
        else
        {
            response.infos.file = false;
            response.infos.repository = true;
        }
    }
}

/**
 * @brief `check the access of the file`
 *
 * if not, error 403
 */
void accessFile(responseT &response, request &request)
{
    response.code = 200;
    if (access(response.path.c_str(), R_OK) == -1)
    {
        response.infos.read = false;
        response.code = 403;
        response.response = "HTTP/1.1 " + std::to_string(response.code) + " Forbidden\r\n";
        response.response += "Content-Length: " + std::to_string(request._body.size());
        response.response += "Content-Type: text/html";
        response.response += "\r\n";
        response.response += "\r\n";
        request._body = "<html><body>403 Forbidden</body></html>";
        response.response += request._body;
    }
    else
        response.infos.read = true;
}

/**
 * @brief `open and read file to found body and content lenght`
 *
 * @return 1 if error else 0
 */
int readFile(responseT &response)
{
    response.body.clear();
    response.contentLen = 0;

    // step 1 : open
    int fd;
    fd = open(response.path.c_str(), O_RDONLY);
    if (fd < 0)
    {
        std::cout << "Error : cannot open file\n";
        return (1);
    }

    // step 2 : read
    ssize_t len = 1;
    while (len > 0)
    {
        char buffer[1024];
        len = read(fd, buffer, sizeof(buffer));
        if (len < 0)
        {
            std::cout << "Error : can't read file\n";
            close(fd);
            return (1);
        }
        response.body.append(buffer, len);
    }

    // step 3 : search-content len
    response.contentLen = response.body.size();

    // step 4 : close
    close(fd);
    return (0);
}

/**
 * @brief `search content type with the extension`
 *
 * .html -> text/html
 *
 * .css -> text/css
 *
 * .txt -> text/plain
 *
 * .jpeg -> image/jpeg
 */
void contentType(responseT &response, request &request)
{
    size_t dot = request._url.rfind(".");
    if (dot == std::string::npos)
    {
        response.contentType = "application/octet-stream";
        return;
    }
    std::string extension = request._url.substr(dot);

    // case index.html?user=42
    if (extension.find("?") != std::string::npos)
    {
        size_t end = extension.find("?");
        extension = extension.substr(0, end);
    }

    if (extension == ".html" || extension == ".htm")
        response.contentType = "text/html";
    else if (extension == ".css")
        response.contentType = "text/css";
    else if (extension == ".txt")
        response.contentType = "text/plain";
    else if (extension == ".jpeg" || extension == ".jpg")
        response.contentType = "image/jpeg";
    else if (extension == ".png")
        response.contentType = "image/png";
    else if (extension == ".gif")
        response.contentType = "image/gif";
    else
        response.contentType = "application/octet-stream";
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
    existFile(response, request);

    // step 5 : access to the file
    accessFile(response, request);

    // step 6 : read file who exist and have access
    if (readFile(response) == 1)
        return (1);

    // step 7 : search content type of the file
    contentType(response, request);
    return (0);
}