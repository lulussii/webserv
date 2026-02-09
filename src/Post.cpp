/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 13:27:18 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/09 10:50:45 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Post.hpp"
#include "Chunked.hpp"
#include "Multipart.hpp"
#include "Error.hpp"
#include "Cgi.hpp"
#include <unistd.h>   //stat() access()
#include <sys/stat.h> //struct stat
#include <fcntl.h>    //open
#include <unistd.h>   //read
#include <dirent.h>
#include <sstream>  //std::stringstream
#include <stdlib.h> //atoi
#include <ctime>    //time_t

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
 * step 2 : if content type already exist in header
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
 */
void prepareResponse(responseT &response, request request)
{
    if (response.infos.error == false)
        response.body = "";

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
 * step 1 : check client_max_body_size
 *
 * step 2 : check if body exist
 *
 * step 3 : create name file
 *
 * step 4 : check repo
 *
 * step 5 : create and write on file
 *
 * step 6 : prepare response
 *
 */
void postMain(request &request, responseT &response, serverT &serverConfig, cgi &cgi)
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
        int errroValue = extractBundary(request);
        if (errroValue == 400)
        {
            errorCode(response, serverConfig, 400);
            return;
        }
        if (checkRepo(response, serverConfig) != 0)
            return;
        splitPart(request, response, serverConfig, m);
        return;
    }

    if (response.cgi == true)
    {
        handleCgi(request, cgi, serverConfig, response, m);
        if (cgiPipe(cgi) == 500)
            errorCode(response, serverConfig, 500);
        parsStdout(cgi);
        buildCgiResponse(cgi, response);
        return ;
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