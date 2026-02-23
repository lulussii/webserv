/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 13:27:18 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/23 12:13:22 by mlaussel         ###   ########.fr       */
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
#include <ctime>    //time_t

/**
 * @brief `Check if the request body size respects client_max_body_size.`
 *
 * step 1 : Check if the "Content-Length" header is present. 
 *
 * step 2 : If no "Content-Length" header is found,use the already parsed body size stored in the request. 
 * 
 * step 3 : Compare the body size with serverConfig.clientMaxBodySize. 
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
 * @brief `Check if a POST request contains a body`
 * 
 * step 1 : Verify if the request body is not empty
 * 
 * step 2 : If a body exists, assign it to the response body, for use later. 
 *
 * @return true if a body is present, otherwise return false.
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
 * @brief `Create a unique file path for an uploaded file`
 * 
 * step 1 : Get the current timestamp using std::time. 
 * 
 * step 2 : Convert the timestamp to a string using stringstream. 
 * 
 * step 3 : Build the upload path using upload_dir ans a filename based on the current timestamp. 
 * 
 * step 4 : Put the resulting path in the response post path. 
 */
void createFileName(responseT &response)
{
    std::stringstream ss;
    time_t now;
    ss << response.location.upload_dir << "/upload_" << std::time(&now);
    response.post.path = ss.str();
}

/**
 * @brief `check repository validity and permissions before create file upload`
 *
 * step 1 : Check if the repository exists → Server Error 500
 *
 * step 2 : Check if the path is a directory → Server Error 500
 *
 * step 3 : Check write access permissions on the directory → Forbidden Error 403
 *
 * step 4 : Repository is valid and writable
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

    return (0);
}

/**
 * @brief `Create or remplace a file and write the request body into it`
 *
 * step 1 : check if the target file already exists using stat()
 *  - If the file does not exist : HTTP code 201 Created
 *  - If the file does exist : HTTP code 200 OK
 * 
 * step 2 : Open the file 
 *  - Create the file if it does not exist (O_CREAT)
 *  - Truncate the file if it already exists (O_TRUNC)
 *  - Open the file in write-only mode (O_WRONLY)
 * 
 * step 3 : Write the request body into the file. 
 * 
 * step 4 : Close the file descriptor
 *
 * subject p.9 "You are not required to use poll()
 * (or an equivalent function) for regular disk files;
 * read() and write() on them do not require readiness notifications."
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

    size_t total = 0;
    size_t len = response.body.size();
    const char *data = response.body.c_str();
    while (total < len)
    {
        ssize_t n = write(fd, data + total, len - total);
        if (n <= 0)
        {
            close(fd);
            return (500); 
        }
        total += n;
    }
    
    close(fd);

    return (0);
}

/**
 * @brief `Prepare the HTTP response before sending it to the client`
 *
 * step 1 : If the response is not an error response, clear the response body.
 * (The body will be rebuilt later by the response builder.)
 *
 * step 2 : Check if the request already contains a "Content-Type" header.
 *  If so, reuse this value and stop processing.
 *
 * step 3 : f no Content-Type is provided, determine it from the file extension of the requested resource (index file).
 * 
 * .html/htm -> text/html
 * .css -> text/css
 * .txt -> text/plain
 * .jpeg/jpg -> image/jpeg
 * .png -> image/png
 * .gif -> image/gif
 * 
 * step 4 : if no extension, "application/octet-stream" by default. 
 */
void prepareResponse(responseT &response, request request)
{
    if (response.infos.error == true)
    response.body = "";
    else
    response.body = request._body;
    
    
    std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");
    if (it != request.headers.end())
        response.contentType = it->second;
    else 
    {
        size_t dot = response.location.index.rfind(".");
        if (dot == std::string::npos)
        {
            response.contentType = "application/octet-stream";
            return;
        }
        std::string extension = response.location.index.substr(dot);
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
}

/**
 * @brief `POST method main`
 *
 * step 1 : Check client_max_body_size
 *
 * step 2 : Check if request body exists
 *
 * step 3 : Chunked transfer encoding (if any)
 *
 * step 4 : Boundary 
 *
 * step 5 : CGI 
 *
 * step 6 : Create a unique file path for an uploaded file
 *
 * step 7 : Create or remplace a file and write the request body into it
 * 
 * step 8 : Prepare HTTP response
 *
 */
void postMain(request &request, responseT &response, serverT &serverConfig)
{
    Multipart m;

    if (clientMaxBodySize(serverConfig, request, response) == false)
    {
        errorCode(response, serverConfig, 413);
        return;
    }

    if (bodyExist(request, response) == false)
    {
        errorCode(response, serverConfig, 400);
        return;
    }

    if (isChunked(request) == true)
    {
        if (chunkedParsing(request, response) == 400)
        {
            errorCode(response, serverConfig, 400);
            return;
        }
    }

    if (isMultipart(request) == true)
    {
        std::cout << "[INFO] is Multipart" << std::endl;
        int errorValue = extractBundary(request);
        if (errorValue == 400)
        {
            errorCode(response, serverConfig, 400);
            return;
        }
        if (checkRepo(response, serverConfig) != 0)
            return;
        splitPart(request, response, serverConfig, m);
        return;
    }

    createFileName(response);

    checkRepo(response, serverConfig);

    int errorValue = createAndWriteFile(response);
    if (errorValue == 500 || errorValue == 403)
    {
        if (errorValue == 500)
            errorCode(response, serverConfig, 500);
        else if (errorValue == 403)
            errorCode(response, serverConfig, 403);
        return;
    }
    prepareResponse(response, request);

    return;
}