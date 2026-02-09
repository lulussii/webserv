/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 15:38:59 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/09 12:44:28 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Get.hpp"
#include "Error.hpp"
#include "Cgi.hpp"
#include <unistd.h>   //stat() access()
#include <sys/stat.h> //struct stat
#include <fcntl.h>    //open
#include <unistd.h>   //read
#include <dirent.h>
#include <errno.h> //errno

/**
 * @brief `build the file path`
 *
 */
void pathBuild(responseT &response, serverT &serverConfig, request &request)
{
    if (request._url == "/")
        response.path = serverConfig.root + response.location.index;
    else
        response.path = serverConfig.root + request._url;
}

/**
 * @brief `check if the file or the repo exist`
 *
 * step 1 : check if the file or the repo exist, if not, error 404
 *
 * step 2 : if file or repo exist, need to know if it's a file or a repository
 *
 * step 3 : S_ISDIR check if it's a classic file (index.html, image.pmg etc...)
 * --> so it's not a repo (response.infos.repository = false)
 *
 * step 4 : S_ISDIR check if it's a repo
 * --> yes (response.infos.repository = true;)
 * --> no (error404)
 *
 * step 5 : case it's a repo, we check if autoindex is activate (on)
 * --> yes :
 *      1) we open the repo (DIR *dir = opendir(response.path.c_str());)
 *      2) generate html page for autoindex
 *      3) loop : we read each repo name with (repo = readdir(dir))
 *      4) repo->d_name it's entry name
 *      5) ignorate courent repo and parents repo
 *      6) add end of body html
 *      7) calculate body size, add content type and code success 200
 *      8) close repo
 *
 */
void existFile(responseT &response, serverT &serverConfig, request &request)
{
    struct stat test;

    if (stat(response.path.c_str(), &test) == -1)
        errorCode(response, serverConfig, 404);
    else
    {
        if (S_ISREG(test.st_mode))
            response.infos.repository = false;
        else
        {
            if (S_ISDIR(test.st_mode))
            {
                response.infos.repository = true;
                if (access(response.path.c_str(), R_OK) == -1)
                {
                    errorCode(response, serverConfig, 403);
                    return;
                }
                if (response.location.autoindex == "on")
                {
                    DIR *dir = opendir(response.path.c_str());
                    if (dir == NULL)
                        errorCode(response, serverConfig, 404);

                    struct dirent *repo;
                    response.body = "<html><head><title>Index of " + request._url + "</title></head><body>\r\n";
                    response.body += "<h1>Index of " + request._url + "</h1><ul>\r\n";

                    while ((repo = readdir(dir)) != NULL)
                    {
                        std::string filename = repo->d_name;
                        if (filename != "." && filename != "..")
                            response.body += "<li><a href='" + filename + "'>" + filename + "</a></li>\r\n";
                    }

                    response.body += "\r\n</ul></body></html>";

                    response.contentLen = response.body.size();
                    response.contentType = "text/html";
                    response.code = 200; // maybe delete because init to 200
                    closedir(dir);
                }
            }
            else
                errorCode(response, serverConfig, 404);
        }
    }
}

/**
 * @brief `check the access of the file`
 *
 * if not, error 403
 */
void accessFile(responseT &response, serverT &serverConfig)
{
    if (access(response.path.c_str(), R_OK) == -1)
        errorCode(response, serverConfig, 403);
}

/**
 * @brief `open and read file to found body and content lenght`
 *
 * step 1 : open
 *
 * step 2 : read
 *
 * step 3 : search-content len
 *
 * step 4 : close fd
 *
 * @return 1 if error else 0
 */
int readFile(responseT &response)
{
    response.body.clear();
    response.contentLen = 0;

    int fd;
    fd = open(response.path.c_str(), O_RDONLY);
    if (fd < 0)
    {
        if (errno == ENOENT)
            return (404);
        else
            return (403);
    }

    ssize_t len = 1;
    while (len > 0)
    {
        char buffer[1024];
        len = read(fd, buffer, sizeof(buffer));
        if (len < 0)
        {
            close(fd);
            return (1);
        }
        response.body.append(buffer, len);
    }

    response.contentLen = response.body.size();

    close(fd);
    return (0);
}

/**
 * @brief `search content type with the extension`
 *
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
void contentType(responseT &response)
{
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
 * @brief `GET method main`
 *
 * step 1 : build file path
 *
 * step 2 : check is the file exist
 *
 * step 3 : access to the file
 *
 * step 4 : read file who exist and have access to build body
 *
 * step 5 : search content type of the file
 *
 * @return 1 if problem, else 0
 */
int getMain(request &request, responseT &response, serverT &serverConfig, cgi &cgi)
{
    pathBuild(response, serverConfig, request);

    if (response.cgi == true)
    {
        Multipart m;
        handleCgi(request, cgi, serverConfig, response, m);
        if (cgiPipe(cgi) == 500)
            errorCode(response, serverConfig, 500);
        parsStdout(cgi);
        buildCgiResponse(cgi, response);
        return (0);
    }
    
    existFile(response, serverConfig, request);

    if (response.infos.error == false && response.infos.repository == false)
        accessFile(response, serverConfig);

    if (response.infos.error == false || response.infos.repository == false)
    {
        int errorValue = readFile(response);
        if (errorValue != 0)
            return (errorValue);
    }

    if (response.infos.repository == false)
        contentType(response);

    return (0);
}