/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 13:27:18 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/02 13:47:44 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Post.hpp"
#include "Chunked.hpp"
#include "Multipart.hpp"
#include "Error.hpp"
#include <unistd.h>   //stat() access()
#include <sys/stat.h> //struct stat
#include <fcntl.h>    //open
#include <unistd.h>   //read
#include <dirent.h>
#include <sstream>  //std::stringstream
#include <stdlib.h> //atoi
#include <ctime>        //time_t

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
bool checkIsPost(request &request, responseT &response)
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
 * @brief `check if the content lenght < clientMaxBodySize, if it's not to big`
 *
 * step 1 : check if already a content lenght
 * 
 * step 2 : else take content lenght from body 
 * 
 * @return true if ok, else false
 */
bool clientMaxBodySize(serverT &serverConfig, request &request, responseT &response)
{
    std::map<std::string, std::string>::iterator it = request.headers.find("Content-Length");
    if (it != request.headers.end())
    {
        response.contentLen = atoi(it->second.c_str());
        if (response.contentLen <= static_cast<size_t>(serverConfig.clientMaxBodySize))
            return (true);
        else
            return (false);
    }
    
    if (request.contentLenght <= static_cast<size_t>(serverConfig.clientMaxBodySize))
        return (true);
    return (false);
}

/**
 * @brief `check if the body of POST exist`
 *
 * @return true if ok, else false
 */
bool bodyExist(request &request, responseT &response)
{
    if (request._body.size() > 0)
    {
        response.body = request._body;
        return (true);
    }
    return (false);
}

/**
 * @brief `create random file with path in location`
 *
 * stringstream use to convert an int into a string
 * 
 * add "/uploads/" to path because we wants to uploads inside this repo
 * 
 * add 
 */
void createFileName(responseT &response)
{
    std::stringstream ss;
    time_t now;
    ss << response.location.upload_dir << "/uploads/" << "upload_" << std::time(&now);
    response.post.path = ss.str();
    //response.post.path = response.location.upload_dir + "/uploads/" + "upload_";
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
int checkRepo(responseT &response, serverT &serverConfig)
{
    struct stat test;

    if (stat(response.location.upload_dir.c_str(), &test) == -1)
    {
        errorCode(response, serverConfig, 500);
        return (500);
    }
    else if (!S_ISDIR(test.st_mode))
    {
        errorCode(response, serverConfig, 500);
        return (500);
    }
    else if (access(response.location.upload_dir.c_str(), W_OK) == -1)
    {
        errorCode(response, serverConfig, 403);
        return (403);
    }
    else
        response.infos.repository = true;

    return (0);
}

/**
 * @brief `create a file and write in`
 *
 * step 1 : check is file exist, yes code 200 else code 201
 *
 * step 2 : open file and create it if it's not
 *
 * step 3 : write body in file
 *
 *
 * step 4 : close fd
 *
 */
int createAndWriteFile(responseT &response)
{
    struct stat test;
    int fd;

    if (stat(response.post.path.c_str(), &test) == -1)
    {
        response.code = 201;
        fd = open(response.post.path.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (fd < 0)
            return (500);
    }
    else
    {
        response.code = 200;
        fd = open(response.post.path.c_str(), O_TRUNC | O_WRONLY, 0644);
        if (fd < 0)
            return (403);
    }

    write(fd, response.body.c_str(), response.body.size());

    close(fd);

    return (0);
}

/**
 * @brief `prepare response`
 *
 * step 1 : if not error, delete body
 * 
 * step 2 : content type already exist in header
 *
 * step 3 : content type if not content type
 * .html/htm -> text/html
 *
 * .css -> text/css
 *
 * .txt -> text/plain
 *
 * .jpeg/jpg -> image/jpeg
 *
 * .png -> image/png
 *
 * .gif -> image/gif
 *
 * 
 *
 */
void prepareResponse(responseT &response, request request)
{
    if (response.infos.error == false)
        response.body = "";

    //check if already a content type
    std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");
    if (it != request.headers.end())
    {
        response.contentType = it->second;
        return;
    }
    
    size_t dot = response.location.index.rfind(".");
    if (dot == std::string::npos)
    {
        response.contentType = "application/octet-stream";
        return;
    }
    std::string extension = response.location.index.substr(dot);

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
 * @brief `POST method main`
 *
 *
 * step 1 : find the good location
 *
 * step 2 : check if method is in server
 *
 * step 3 : check client_max_body_size
 *
 * step 4 : check if body exist
 *
 * step 5 : create name file
 *
 * step 6 : check repo
 *
 * step 7 : create and write on file
 *
 * step 8 : prepare response
 *
 * @return 1 if problem, else 0
 */
void postMain(request &request, responseT &response, serverT &serverConfig)
{

    // step 1 : find the good location
    if (foundLocationPost(request, serverConfig, response) == false)
    {
        errorCode(response, serverConfig, 404);
        return ;
    }

    // step 2 : check if method is in server
    if (checkIsPost(request, response) == false)
    {
        errorCode(response, serverConfig, 405);
        return ;
    }
    // step 3 : check client_max_body_size
    if (clientMaxBodySize(serverConfig, request, response) == false)
    {
        errorCode(response, serverConfig, 413);
        return ;
    }

    // step 4 : check if body exist
    if (bodyExist(request, response) == false)
    {
        errorCode(response, serverConfig, 400);
        return ;
    }

    // step : chuncked
    if (isChunked(request) == true)
    {
        if (chunkedParsing(request, response) == 400)
        {
            errorCode(response, serverConfig, 400);
            return ;
        }
    }

    // step : multipart/form-data
    bool boolValue = isMultipart(request);
    if (boolValue == true)
    {
        int errroValue = extractBundary(request);
        if (errroValue == 400)
        {
            errorCode(response, serverConfig, 400);
            return ;
        }
        if (checkRepo(response, serverConfig) != 0)
            return ;
        splitPart(request, response, serverConfig);
        return ;
    }

    // step 5 : create name file
    createFileName(response);

    // step 6 : check repo
    checkRepo(response, serverConfig);

    // step 7 : create and write on file
    int errorValue = createAndWriteFile(response);
    if (errorValue == 500 || errorValue == 403)
    {  
        if (errorValue == 500)
            errorCode(response, serverConfig, 500);
        else if (errorValue == 403)
            errorCode(response, serverConfig, 403);
         return ;
    }

    // step 8 : prepare response
    prepareResponse(response, request);

    return ;
}