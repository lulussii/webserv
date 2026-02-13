/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 15:38:59 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/13 09:13:10 by lserodon         ###   ########.fr       */
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

// /**
//  * @brief `check if the file or the repo exist`
//  *
//  * step 1 : check if the file or the repo exist, if not, error 404
//  *
//  * step 2 : if file or repo exist, need to know if it's a file or a repository
//  *
//  * step 3 : S_ISDIR check if it's a classic file (index.html, image.pmg etc...)
//  * --> so it's not a repo (response.infos.repository = false)
//  *
//  * step 4 : S_ISDIR check if it's a repo
//  * --> yes (response.infos.repository = true;)
//  * --> no (error404)
//  *
//  * step 5 : case it's a repo, we check if autoindex is activate (on)
//  * --> yes :
//  *      1) we open the repo (DIR *dir = opendir(response.path.c_str());)
//  *      2) generate html page for autoindex
//  *      3) loop : we read each repo name with (repo = readdir(dir))
//  *      4) repo->d_name it's entry name
//  *      5) ignorate courent repo and parents repo
//  *      6) add end of body html
//  *      7) calculate body size, add content type and code success 200
//  *      8) close repo
//  *
//  */
// static void existFile(responseT &response, serverT &serverConfig, request &request)
// {
//     struct stat test;

//     if (stat(response.path.c_str(), &test) == -1)
//         errorCode(response, serverConfig, 404);
//     else
//     {
//         if (S_ISREG(test.st_mode))
//             response.infos.repository = false;
//         else
//         {
//             if (S_ISDIR(test.st_mode))
//             {
//                 response.infos.repository = true;
//                 if (access(response.path.c_str(), R_OK) == -1)
//                 {
//                     errorCode(response, serverConfig, 403);
//                     return;
//                 }
//                 if (response.location.autoindex == "on")
//                 {
//                     DIR *dir = opendir(response.path.c_str());
//                     if (dir == NULL)
//                         errorCode(response, serverConfig, 404);

//                     struct dirent *repo;
//                     response.body = "<html><head><title>Index of " + request._url + "</title></head><body>\r\n";
//                     response.body += "<h1>Index of " + request._url + "</h1><ul>\r\n";

//                     while ((repo = readdir(dir)) != NULL)
//                     {
//                         std::string filename = repo->d_name;
//                         if (filename != "." && filename != "..")
//                             response.body += "<li><a href='" + filename + "'>" + filename + "</a></li>\r\n";
//                     }

//                     response.body += "\r\n</ul></body></html>";

//                     response.contentLen = response.body.size();
//                     response.contentType = "text/html";
//                     response.code = 200; // maybe delete because init to 200
//                     closedir(dir);
//                 }
//             }
//             else
//                 errorCode(response, serverConfig, 404);
//         }
//     }
// }

/**
 * @brief `Build the file path`
 *
 */
static void pathBuild(responseT &response, serverT &serverConfig, request &request)
{
    if (request._url == "/")
        response.path = serverConfig.root + response.location.index;
        // response.path = serverConfig.root + "/";
    else
        response.path = serverConfig.root + request._url;
    //ICI GERER LE CAS OU IL Y A UNE ROOT DANS UNE LOCATION
}

/**
 * @brief `Check if path exists and determine its type (file or directory)`
 * 
 * step 1 : if path does not exist → 404
 * 
 * step 2 : If regular file → repository = false
 * 
 * step 3 : If directory → repository = true
 */
static int existAndType(responseT &response, serverT &serverConfig)
{
    struct stat test;
    
    if (stat(response.path.c_str(), &test) == -1)
    {
         errorCode(response, serverConfig, 404);
         return (404);
    }
    
    if (S_ISREG(test.st_mode))
    {
        response.infos.repository = false;
        return (0);
    }

    if (S_ISDIR(test.st_mode))
    {
        response.infos.repository = true;
        return (0);
    }
    
    errorCode(response, serverConfig, 404);
    return (404);      
}

/**
 * @brief `Generate an autoindex page if enabled and path is a directory`
 * 
 * step 1 : Open directory (DIR *dir = opendir(response.path.c_str());)
 * 
 * step 2 : Generate HTML header
 * 
 * step 3 loop : we read each directory name with (directory = readdir(dir))
 * 
 * step 4 directory->d_name it's entry name
 * 
 * step 5 ignorate courent directory and parents directory
 * 
 * step 6 add end of body html
 * 
 * step 7 calculate body size, add content type and code success 200
 * 
 * step 8 close directory
 */
static void autoindex(responseT &response, serverT &serverConfig, request &request)
{
    if (response.location.autoindex == "on" && response.infos.repository == true)
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

/**
 * @brief `Check access permissions for the directory`
 * 
 * step 1 : Check if the request target is identified as a repository.
 * 
 * step 2 : If it is a repository, verify read access permissions using access() and R_OK.
 * 
 * step 3 : If access is denied, generate a 403 Forbidden response and stop processing.
 * 
 * step 4 : If access is allowed or the target is not a repository, continue normally.
 */
int accessRepo(responseT &response, serverT &serverConfig)
{
    if (response.infos.repository == true)
    {
        if (access(response.path.c_str(), R_OK) == -1)
        {
            errorCode(response, serverConfig, 403);
            return (403);
        }
    }
    return (0);
}

/**
 * @brief `Check file access permissions`
 *
 * step 1 : Use access() to check read permissions (R_OK) on the file path.
 * 
 * step 2 : If access is denied or the file is not readable, return a 403 Forbidden error.
 */
void accessFile(responseT &response, serverT &serverConfig)
{
    if (access(response.path.c_str(), R_OK) == -1)
        errorCode(response, serverConfig, 403);
}

/**
 * @brief `Open and read a file to found the response body and content length`
 *
 * step 1 : Initialize the response body and content length.
 *
 * step 2 : Open the file in read-only mode.
 * If open fails:
 *  - Return 404 if file does not exist (ENOENT).
 *  - Return 403 for other permission errors.
 *
 * step 3 : Read the file content in a loop.
 *  - Use a buffer (1024 bytes).
 *  - Append read bytes to response.body.
 *  - Stop when read returns 0 (EOF).
 *  - If read fails, close the file and return 500 (server error).
 *
 * step 4 : Set response.contentLen with the size of response.body.
 * 
 * step 5 : Close the file descriptor.
 *
 * @return 0 if success, 500 if read error, 403/404 for permission or missing file
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
            return (500);
        }
        response.body.append(buffer, len);
    }

    response.contentLen = response.body.size();

    close(fd);
    return (0);
}

/**
 * @brief `Determine the Content-Type header based on the file extension`
 * 
 * step 1 : Find the last dot in the filename to determine the extension. 
 * 
 * step 2 : Extract the substring form the dot to the end as the extension. 
 * 
 * step 3 : Handle query strings. 
 * 
 * step 4 : Match common extensions to their types 
 *
 *      .html/htm -> text/html
 *
 *      .css -> text/css
 *
 *      .txt -> text/plain
 *
 *      .jpeg/jpg -> image/jpeg
 *
 *      .png -> image/png
 *
 *      .gif -> image/gif
 * 
 * step 5 : If the extension is not recognized, default to "application/octet-stream"
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
 * step 1 : Build file path. 
 *
 * step 2 : If CGI, do CGI part. 
 *
 * step 3 : Check if the file exists and determine its type. 
 *
 * step 4 : Check access permission for the directory. 
 *
 * step 5 : Autoindex if enabled and directory. 
 * 
 * step 6 : Check access permission for the directory. 
 * 
 * step 7 : Open and read a file to found the response body and content length.
 * 
 * step 8 : Determine content type. 
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
    
    // existFile(response, serverConfig, request);
    
    if (existAndType(response, serverConfig) != 0)
        return (0);

    if (accessRepo(response, serverConfig) != 0)
        return (0);
    
    autoindex(response, serverConfig, request);

    if (response.infos.error == false && response.infos.repository == false)
        accessFile(response, serverConfig);

    if (response.infos.error == false && response.infos.repository == false)
    {
        int errorValue = readFile(response);
        if (errorValue != 0)
            return (errorValue);
    }

    if (response.infos.repository == false)
        contentType(response);

    return (0);
}