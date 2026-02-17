/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Multipart.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 15:04:09 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/16 12:18:50 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Multipart.hpp"
#include "Request.hpp"
#include "Error.hpp"
#include <ctime>
#include <unistd.h>     //stat() access()
#include <sys/stat.h>   //struct stat
#include <fcntl.h>      //open
#include <unistd.h>     //read
#include <sstream>      //streamstring
#include <ctime>        //time_t

/**
 * @brief `Check if request is multipart/form-data`
 * 
 * step 1 : Look for the "Content-Type" header in the request headers map.
 * 
 * step 2 : If the header exists, check if the string "multipart/form-data" is present.
 * 
 * step 3 : Return true if found, otherwise return false.
 *
 */
bool isMultipart(request &request)
{
    std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");

    if (it != request.headers.end())
    {
        if (it->second.find("multipart/form-data") != std::string::npos)
        {
            return (true);
        }
    }
    return (false);
}

/**
 * @brief `Extract boundary string from multipart/form-data Content-Type`
 * 
 * step 1 : Look for the "Content-Type" header in the request headers map.
 * 
 * step 2 : Search for the substring "boundary=" inside the header value.
 * 
 * step 3 : If "boundary=" is found, extract everything after the '=' as the boundary string.
 * 
 * step 4 : Store the boundary string in request.boundary.
 * 
 * step 5 : Return 0 if successful, 400 if the header or boundary is missing.
 */
int extractBundary(request &request)
{
    std::string s = "boundary=";
    std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");

    if (it != request.headers.end())
    {
        size_t pos = it->second.find("boundary=");
        if (pos == std::string::npos)
            return (400);
        std::string tmp = it->second.substr(pos + s.size());
        request.boundary = tmp;
        return (0);
    }
    else
    {
        return (400);
    }
        // request.boundary = "boundary";
}

/**
 * @brief `split all parts separated by boundary`
 *
 * step 1 : Create a temporary copy of the request body.
 *
 * step 2 : Loop until the end of the multipart (until "--boundary--").
 *
 * step 3 : Remove the first boundary from tmp (including "--" and "\r\n").
 *
 * step 4 : Search for the next boundary in tmp. If not found, search for the ending boundary.
 *
 * step 5 : Extract the part content between boundaries, trimming extra characters.
 *
 * step 6 : Fill the Multipart struct with the extracted part data.
 *
 * step 7 : Generate a unique file path using the filename and timestamp.
 * 
 * step 8 : Write the multipart content to disk, handle errors if writing fails.
 */
void splitPart(request &request, responseT &response, serverT &serverConfig, Multipart &m)
{
    std::string tmp = request._body;
    request.party.clear();
    
    if (tmp.find(request.boundary) == std::string::npos)
        errorCode(response, serverConfig, 400);

    while (tmp.find(request.boundary) != std::string::npos)
    {
        tmp = tmp.substr(2 + request.boundary.size() + 2);

        size_t end = tmp.find("--" + request.boundary);
        if (end == std::string::npos)
        {
            end = tmp.find("--" + request.boundary + "--");
            if (end == std::string::npos)
                break;
        }

        std::string fullPart = tmp.substr(0, end - 1);

        m.fullPart = fullPart;
        extractName(m);
        if (extractFileName(m) == 1)
            errorCode(response, serverConfig, 400);
        extractContentType(m);
        extractContent(m);
        request.party.push_back(m);
        tmp = tmp.substr(end);

        std::stringstream ss;
        time_t now;
        ss << response.location.upload_dir << "/uploads/" << m.filename << "_" << std::time(&now);
        response.post.path = ss.str();

        int errorValue = createAndWriteMultipartFile(response, m);
        {
            if (errorValue == 500)
                errorCode(response, serverConfig, 500);
            else if (errorValue == 403)
                errorCode(response, serverConfig, 403);
        }
        
        response.contentType = "text/plain";
        // std::cout << "DEBUG\n";
        // std::cout << "full part = [" << m.fullPart << "]\n";
        // std::cout << "name = [" << m.name << "]\n";
        // std::cout << "file name = [" << m.filename << "]\n";
        // std::cout << "Content-Type = [" << m.contentType << "]\n";
        // std::cout << "Content = [" << m.content << "]\n";
        // std::cout << "tmp = [" << tmp << "]\n";
    }
}

/**
 * @brief `extract the name from a multipart part`
 *
 * step 1 : Search for "name=" in the full part string.
 *
 * step 2 : Extract substring starting from "name=".
 *
 * step 3 : Find the end of the name field using ";" delimiter.
 *
 * step 4 : Remove "name=\"" prefix and trailing quote.
 *
 */
void extractName(Multipart &m)
{
    std::string name;
    std::string tmp = "name=\"";

    size_t len = m.fullPart.find("name");
    
    name = m.fullPart.substr(len);

    size_t space = name.find(";");
    name = name.substr(0, space);

    name = name.substr(tmp.size());
    name = name.substr(0, name.size() - 1);

    m.name = name;
}

/**
 * @brief `extract the filename from a multipart part`
 *
 * step 1 : Search for "filename=" in the full part string.
 *
 * step 2 : Extract substring starting from "filename=".
 *
 * step 3 : Find the end of the filename using "\r\n".
 *
 * step 4 : Remove "filename=\"" prefix and trailing quote.
 * 
 * step 5 : Assign the cleaned filename to the Multipart struct.
 *
 */
int extractFileName(Multipart &m)
{
    std::string filename;
    std::string tmp = "filename=\"";

    size_t len = m.fullPart.find("filename");
    if (len == std::string::npos)
    {
        filename = "";
        return (1);
    }
    
    filename = m.fullPart.substr(len);

    size_t space = filename.find("\r\n");
    filename = filename.substr(0, space);

    filename = filename.substr(tmp.size());
    filename = filename.substr(0, filename.size() - 1);

    m.filename = filename;

    return (0);
}

/**
 * @brief `extract Content-Type from a multipart part`
 *
 * step 1 : Search for "Content-Type:" in the full part string.
 * 
 * step 2 : Extract substring starting from "Content-Type:".
 * 
 * step 3 : Find the end of the Content-Type line using "\r\n".
 * 
 * step 4 : Remove the "Content-Type: " prefix.
 * 
 * step 5 : Assign the content type to the Multipart struct.
 */
void extractContentType(Multipart &m)
{
    std::string contentType;
    std::string tmp = "Content-Type: ";

    size_t len = m.fullPart.find("Content-Type:");
    
    contentType = m.fullPart.substr(len);

    size_t space = contentType.find("\r\n");
    contentType = contentType.substr(0, space);

    contentType = contentType.substr(tmp.size());
    contentType = contentType.substr(0, contentType.size());

    m.contentType = contentType;
}

/**
 * @brief `extract the content data from a multipart part`
 *
 * step 1 : Search for the delimiter "\r\n\r\n" that separates headers from content.
 * 
 * step 2 : Extract the substring after the delimiter.
 * 
 * step 3 : Assign the extracted content to the Multipart struct.
 */
void extractContent(Multipart &m)
{
    std::string content;
    std::string tmp = "\r\n\r\n";

    size_t len = m.fullPart.find("\r\n\r\n");

    content = m.fullPart.substr(len + tmp.size());

    m.content = content;
}

/**
 * @brief `create a file and write content from multipart`
 *
 * step 1 : Check if the file exists. 
 *   - If it does not exist, set response code to 201 (Created)
 *   - If it exists, set response code to 200 (OK)
 *
 * step 2 : Open the file. 
 *   - Use O_CREAT | O_TRUNC | O_WRONLY if file does not exist
 *   - Use O_TRUNC | O_WRONLY if file exists
 *   - Handle errors (return 500 or 403 if open fails)
 *
 * step 3 : Write the content of the multipart to the file. 
 *
 * step 4 : Close the file descriptor
 *
 * subject p.9 "You are not required to use poll()
 * (or an equivalent function) for regular disk files;
 * read() and write() on them do not require readiness notifications."
 */
int createAndWriteMultipartFile(responseT &response, Multipart &m)
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

    // write(fd, m.content.data(), m.content.size());
    size_t total = 0;
    size_t len = m.content.size();
    const char *data = m.content.data();
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