/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:24:02 by mlaussel          #+#    #+#             */
/*   Updated: 2026/01/19 12:59:20 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream> //cout
#include <unistd.h> //read
#include <fcntl.h>  //open
#include "Request.hpp"

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

    if (i == std::string::npos)
        i = p.line.find('\n');

    if (i == std::string::npos) // security for find, if doesn't find, find return the bigger size_t
    {
        std::cout << "Invalid HTTP request\n";
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
        return (1);
    }

    request._method = firstLine.substr(0, pos1);
    request._url = "/" + firstLine.substr(pos1 + 2, pos2 - pos1 - 2);
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
std::string trim(const std::string &str)
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

        if (end == std::string::npos)
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
    request._body = p.line;
    request.contentLenght = request._body.size();
}

/**
 * @brief `main of the parsing`
 *
 * step 1 : firstline extract and parsing
 *
 * step 2 : headers parsing
 *
 * step 3 : if POST method, read body
 *
 * @return 1 if problem, else 0
 */
int requestMain(request &request, parsingT &p)
{

    // Simple GET
    // p.line = "GET /index.html HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl/8.7.1\r\nAccept: */*\r\n\r\n";

    // Simple GET 2
    // p.line = "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl/8.7.1\r\nAccept: */*\r\n\r\n";

    // GET inexistant 404
    // p.line = "GET /unfichier_inexistant.html HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // GET access 403 (must chang config file to index.html to chmod 000 secret.html)
    // p.line = "GET /secret.html HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // GET autoindex : location /test { autoindex on; methods GET;
    // p.line = "GET /test HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // POST with body /home/mlaussel/test/www/html/tmp/uploads
    // p.line = "POST /login HTTP/1.1\r\nHost: localhost\r\nContent-Length: 24\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\nusername=bob&password=42";
   
    // POST simple test 
    //p.line = "POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Length: 13\r\nContent-Type: text/plain\r\n\r\nHello World!";

    // POST exist upload_dir /home/mlaussel/test/www/html/tmp/notexist
    
    // POST access 403 chmod 400 tmp/uploads

    // POST complexe test
    p.line = "POST /upload HTTP/1.1\r\n Host: localhost\r\n Transfer-Encoding: chunked\r\n\r\nB\r\nHello World\r\n5\r\n12345\r\n0\r\n\r\n";

    // DELETE
    // p.line = "DELETE /files/test.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // step 1 : firstline extract and parsing
    if (firstLine(p, request) == 1)
        return (1);

    // step 2 : headers parsing
    headers(p, request);

    // step 3 : if POST method, read body
    if (request._method == "POST")
        postBody(p, request);

    return (0);
}