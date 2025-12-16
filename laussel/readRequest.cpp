/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   readRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:08:19 by mlaussel          #+#    #+#             */
/*   Updated: 2025/12/16 13:08:22 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream> //cout
#include <unistd.h> //read
#include <fcntl.h>  //open
#include "Request.hpp"
#include "readRequest.hpp"

#include <stdlib.h> //atoi to delete


/**
 * @brief `open and read file`
 *
 * step 1 : open file
 * 
 * step 2 : read .txt
 * 
 * step 3: convert buffer into std::string to use existing string functions
 * 
 * @return 1 if problem, else 0
 */
static int readFile(char **argv, parsingT &p)
{
    // step 1 : open file
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        std::cout << "Error : cannot open file\n";
        return (1);
    }
    p.fd = fd;
    
    // step 3 : read .txt
    char buffer[1024];
    ssize_t len = 0;
    len = read(fd, buffer, sizeof(buffer) - 1);
    if (len < 0)
    {
        std::cout << "Error : can't read file\n";
        close(fd);
        return (1);
    }
    buffer[len] = '\0';
    
    // step 3: convert buffer into std::string to use existing string functions
    std::string line(buffer);
    p.line = line;
    
    return (0);
}

/**
 * @brief `firstline extract and parsing`
 *
 * step 1 : extract the first line
 * 
 * step 2 : pars the first line
 * 
 * @return 1 if problem, else 0
 */
static int firstLine(parsingT &p, request &request)
{
     // step 1 : extract the first line

    size_t i = p.line.find("\r\n");

    if (i == std::string::npos) // security macos
        i = p.line.find('\n');

    if (i == std::string::npos) // security for find, if doesn't find, find return the bigger size_t
    {
        std::cout << "Invalid HTTP request\n";
        close(p.fd);
        return (1);
    }
    std::string firstLine = p.line.substr(0, i);
    size_t skip = 2;
    if (p.line[i] == '\n') // if end of the line with only \n
        skip = 1;
    p.line = p.line.substr(i + skip); // remove first line

    // step 2 : pars the first line

    size_t pos1 = firstLine.find(' ');
    size_t pos2 = firstLine.find(' ', pos1 + 1); // search second space after the first one in pos1

    if (pos1 == std::string::npos || pos2 == std::string::npos)
    {
        std::cout << "Invalid request line\n";
        close(p.fd);
        return (1);
    }

    request._method = firstLine.substr(0, pos1);
    request._url = firstLine.substr(pos1 + 1, pos2 - pos1 - 1);
    request._version = firstLine.substr(pos2 + 1);

    return (0);
}

/**
 * @brief Removes leading and trailing whitespace (spaces and tabs) from a string.
 *
 * This function takes a std::string and returns a new string with all
 * whitespace characters (' ' and '\t') removed from the beginning and end.
 *
 * @param str The input string to be trimmed.
 * @return A new std::string without leading or trailing whitespace.
 */
static std::string trim(const std::string &str)
{
    size_t start = 0;
    while (start < str.size() && (str[start] == ' ' || str[start] == '\t'))
        ++start;

    size_t end = str.size();
    while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t'))
        --end;

    return str.substr(start, end - start);
}
/**
 * @brief `headers parsing`
 *
 */
static void headers(parsingT &p, request &request)
{
    while (true)
    {
        size_t end = p.line.find("\r\n");
        size_t skip = 2; // if \r\n

        if (end == std::string::npos) // security macos
        {
            end = p.line.find('\n');
            skip = 1;
        }
        if (end == std::string::npos)
            break;

        std::string headerLine = p.line.substr(0, end);
        p.line = p.line.substr(end + skip); // delete the line already read

        if (headerLine.empty())
            break; // end header

        size_t posDoubleDot = headerLine.find(':');
        if (posDoubleDot == std::string::npos)
            continue;

        std::string key = trim(headerLine.substr(0, posDoubleDot));
        std::string value = trim(headerLine.substr(posDoubleDot + 1));
        request.headers[key] = value;
    }
}

/**
 * @brief `Read body if POST`
 *
 */
static void postBody(parsingT &p, request &request)
{
    int content_length = 0;
    if (request.headers.find("Content-Length") != request.headers.end())
        content_length = atoi(request.headers["Content-Length"].c_str()); //chang atoi

    if (content_length > 0 && p.line.size() >= static_cast<size_t>(content_length))
        request._body = p.line.substr(0, content_length);
}

/**
 * @brief `main of the parsing`
 *
 * step 1 : open and read file
 * 
 * step 2 : firstline extract and parsing
 * 
 * step 3 : headers parsing
 * 
 * step 4 : if POST method, read body
 * 
 * step 5 : close file
 * 
 * @return 1 if problem, else 0
 */
int parsingMain(char **argv, request &request, parsingT &p)
{
    // step 1 : open and read file
    if (readFile(argv, p) == 1)
        return (1);

    // step 2 : firstline extract and parsing
    if (firstLine(p, request) == 1)
        return (1);

    // step 3 : headers parsing
    headers(p, request);

    // step 4 : if POST method, read body
    postBody(p, request);

    // step 5 : close file
    close(p.fd);
    
    return (0);
}