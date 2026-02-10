/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:24:02 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/10 15:02:27 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h> //read
#include "Request.hpp"
#include "Config.hpp"
#include "Error.hpp"
#include "Response.hpp"
#include "Cgi.hpp"

// static void debug(responseT &response)
// {
//     std::cout << "location : [" << response.location.path << "]" << std::endl;
//     for (size_t i = 0; i < response.location.methods.size(); ++i)
//         std::cout << "  Methods: [" << response.location.methods[i] << "] ";
//     std::cout << std::endl;
//     if (!response.location.index.empty())
//         std::cout << "  Index: [" << response.location.index << "]" << std::endl;

//     if (!response.location.autoindex.empty())
//         std::cout << "  Autoindex: [" << response.location.autoindex << "]" << std::endl;

//     if (!response.location.upload_dir.empty())
//         std::cout << "  Upload dir: [" << response.location.upload_dir << "]" << std::endl;

//     if (!response.location.cgiExtension.empty())
//         std::cout << "  cgiExtension: [" << response.location.cgiExtension << "]" << std::endl;

//     if (!response.location.cgiBinary.empty())
//         std::cout << "  cgiBinary: [" << response.location.cgiBinary << "]" << std::endl;
// }

/**
 * @brief `Firstline extract and parsing`
 *
 * step 1 : extract the first line from p.line, use \r\n
 *  - Handle both '\r\n' and '\n' line endings
 *  - Remove the first line from p.line
 *
 * step 2 : Parse the first line into method, URL, and HTTP version. 
 *  - Split by spaces
 * 
 * step 3 : Extract method, URL, and version safely
 *  - method = substring before first space
 *  - URL = substring between first and second space
 *  - version = substring after second space
 *
 * @return 1 if problem, else 0
 */
static int firstLine(parsingT &p, request &request)
{
    // step 1 : extract the first line
    size_t i = p.line.find("\r\n");

    if (i == std::string::npos)
    {
        i = p.line.find('\n');
        if (i == std::string::npos) // security for find, if doesn't find, find return the bigger size_t
            return (1);
    }
    std::string firstLine = p.line.substr(0, i);
    
    size_t skip = 2;
    if (p.line[i] == '\n') // if end of the line with only \n
        skip = 1;
    p.line = p.line.substr(i + skip); // remove \n or \r\n end first line

    // step 2 : pars the first line
    size_t pos1 = firstLine.find(' ');
    size_t pos2 = firstLine.find(' ', pos1 + 1); // search second space after the first one in pos1

    if (pos1 == std::string::npos || pos2 == std::string::npos)
        return (1);

    // step 3 : extraxt
    request._method = firstLine.substr(0, pos1);
    request._url = "/" + firstLine.substr(pos1 + 2, pos2 - pos1 - 2);
    request._version = firstLine.substr(pos2 + 1);

    return (0);
}

/**
 * @brief `Headers parsing`
 * 
 * step 1 : Loop on each line from p.line (without first line)
 *  - Search end of line '\n' or '\r\n'
 *  - Extract line 
 *  - Delete extract line from p.line
 * 
 * step 2 : Seperate key and value
 *  - Search ':'
 *
 * step 3 : Seperate string key and string value
 * 
 * step 4 : Add in request.headers
 * 
 */
static int headers(parsingT &p, request &request)
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
            return (1);

        std::string key = headerLine.substr(0, posDoubleDot);
        std::string value = headerLine.substr(posDoubleDot + 1);
        request.headers[key] = value;
        // request.headers[headerLine.substr(0, posDoubleDot)] = headerLine.substr(posDoubleDot + 1);
    }
    return (0);
}

/**
 * @brief `Read body if POST`
 * 
 * step 1 : p.line don't have header know. So all p.line is body
 * 
 * step 2 : content length with size
 *
 */
static void postBody(parsingT &p, request &request)
{
    request._body = p.line;
    request.contentLenght = request._body.size();
}

/**
 * @brief `Find the best matching location for the requested URL.`
 *
 * step 1 : Initialize variables. 
 *  - bestLen : longest matching path length. 
 *  - found : bool if found location or not. 
 * 
 * step 2 : Iterate over all locations configured in the server
 * 
 * step 3 : For each location, check if the request URL is at least as long as the location.path.
 * 
 * step 4 : Check if the request URL starts with the location path. 
 * 
 * step 5 : If it matches and is longer then the previous best match :
 *      - Update bestLen with current location length
 *      - Store the location in the response.location
 *      - Mark found as true
 */
int foundLocation(request &request, serverT &serverConfig, responseT &response)
{
    size_t bestLen = 0;
    bool found = false;

    for (std::map<std::string, locationsT>::iterator it = serverConfig.locations.begin();
         it != serverConfig.locations.end(); ++it)
    {
        const std::string &locPath = it->first;

        if (request._url.size() < locPath.size())
            continue;

        if (request._url.compare(0, locPath.size(), locPath) == 0)
        {
            if (locPath.size() > bestLen)
            {
                bestLen = locPath.size();
                response.location = it->second;
                found = true;
            }
        }
    }
    return (found);
}


/**
 * @brief `Check if the HTTP method from the request is allowed in the matched location`
 *
 * step 1 : Iterate through all allowed methods int the matched location
 * 
 * step 2 : For each method, compare it with the request method
 * 
 * step 3 : If a match is found, method is allowed and we return true
 * 
 * step 4 : Else, method is not allowed and we return false
 *
 * @return true if found, else false
 */
bool checkIs(request &request, responseT &response)
{
    for (size_t i = 0; i < response.location.methods.size(); ++i)
    {
        if (response.location.methods[i] == request._method)
            return (true);
    }
    return (false);
}

/**
 * @brief `Main request`
 *
 * step 1 : Extract and parse the first line of the HTTP request. 
 *
 * step 2 : Parse all headers. 
 *
 * step 3 : Read body for POST requests. 
 * 
 * step 4 : Find the best matching location. 
 * 
 * step 5 : Process CGI if required. 
 * 
 * step 6 : Verify that the request method is allowed in this location. 
 *
 */
void requestMain(request &request, parsingT &p, serverT &serverConfig, responseT &response, cgi &cgi)
{
    // -----
    //| GET |
    // ------
    
    // GET /upload
    //p.line = "GET /uploads HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl/8.7.1\r\nAccept: */*\r\n\r\n";

    // GET /
    // p.line = "GET / HTTP/1.1\r\nHost: localhost\r\nUser-Agent: curl/8.7.1\r\nAccept: */*\r\n\r\n";

    // GET error 404
    //p.line = "GET /doesnotexist.html HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // GET error 403 : repo secret (chmod 000) + in config location /secret
    // p.line = "GET /html/secret/ HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // GET error 403 : file secret (chmod 000)
    // p.line = "GET /html/secret.html HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // GET error 403 or 404: url outside root
    //p.line = "GET /../secret.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // GET autoindex : location /test { autoindex on; methods GET;
    // p.line = "GET /test HTTP/1.1\r\nHost: localhost\r\n\r\n";



    // ------
    //| POST |
    // ------
    
    // POST
    // p.line = "POST /login HTTP/1.1\r\nHost: localhost\r\nContent-Length: 24\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\nusername=bob&password=42";

    // POST
    // p.line = "POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Length: 13\r\nContent-Type: text/plain\r\n\r\nHello World!";
    
    // POST error 400 (no content length so bad request)
    // p.line = "POST /upload HTTP/1.1\r\nHost: localhost\r\n\r\n";

    //POST access 500 chmod 000 tmp/uploads
    // p.line = "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 11\r\n\r\nHello World";

    // POST error 413, put an Content-Length more than 100000000

    // POST CHUNKED
    // p.line = "POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\nContent-Type: text/plain\r\n\r\n5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n";

    // POST CHUNKED error 400 : chunk size not good
    // p.line = "POST /upload HTTP/1.1\r\nHost: localhost\r\nTransfer-Encoding: chunked\r\n\r\nZZZ\r\nHello\r\n6\r\n World\r\n0\r\n\r\n";

    // POST BOUNDARY
    // p.line ="POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Length: 138\r\n\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"file\"; filename=\"test.txt\"\r\nContent-Type: text/plain\r\n\r\nHello World\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

    // POST BOUNDARY COMPLEX
    // p.line = "POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Length: 314\r\n\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"file1\"; filename=\"hello.txt\"\r\nContent-Type: text/plain\r\n\r\nHello World!\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"file2\"; filename=\"image.png\"\r\nContent-Type: image/png\r\n\r\nPNGDATA123456\r\n------WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n";

    // POST BOUNDARY error 400
    // p.line = "POST /upload HTTP/1.1\r\nHost: localhost\r\nContent-Type: multipart/form-data\r\nContent-Length: 20\r\n\r\nHello World";


    // --------
    //| DELETE |
    // --------
    
    // DELETE simple
    //p.line = "DELETE /uploads/upload_0 HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // DELETE error 404
    //p.line = "DELETE /upload/nope.txt HTTP/1.1\r\nHost: localhost\r\n\r\n";
    
    // DELETE error 403 or 404 : delete repo
    //p.line = "DELETE /upload/ HTTP/1.1\r\nHost: localhost\r\n\r\n";

    // DELETE error 405 in conf must delete DELETE in / location
    //p.line = "DELETE / HTTP/1.1\r\nHost: localhost\r\n\r\n";

    
    // ------
    //| CGI |
    // ------

    // POST CGI 
    // p.line = "POST /cgi/test.php?name=mlaussel HTTP/1.1\r\nHost: localhost\r\nContent-Length: 13\r\nContent-Type: text/plain\r\n\r\nHello World!";

    // GET SIMPLE CGI
    // p.line = "GET /cgi/test.php HTTP/1.1\r\n""Host: localhost\r\n""\r\n";

    // GET CGI WITH QUERY
    // p.line = "GET /cgi/test.php?name=clarke&city=polis HTTP/1.1\r\n""Host: localhost\r\n""\r\n";

    // POST SIMPLE CGI
    // p.line = "POST /cgi/test.php HTTP/1.1\r\n""Host: localhost\r\n""Content-Type: text/plain\r\n""Content-Length: 11\r\n""\r\n""Hello World";

    // POST CGI with QUERY AND BODY 
    // p.line = "POST /cgi/test.php?x=42 HTTP/1.1\r\n""Host: localhost\r\n""Content-Type: text/plain\r\n""Content-Length: 5\r\n""\r\n""Salut";

    // POST CGI CHUNKED
    // p.line = "POST /cgi/test.php HTTP/1.1\r\n""Host: localhost\r\n""Transfer-Encoding: chunked\r\n""Content-Type: text/plain\r\n""\r\n""5\r\n""Hello\r\n""6\r\n"" World\r\n""0\r\n""\r\n";

    // GET CGI BAD SCRIPT error 404
    // p.line = "GET /doesnotexist.php HTTP/1.1\r\n""Host: localhost\r\n""\r\n";

    // DELETE CGI error 405
    // p.line = "DELETE /test.php HTTP/1.1\r\n""Host: localhost\r\n""\r\n";

    
    //curl "http://localhost:8080/cgi/test.php?name=mlaussel"

    
    //curl -i -X POST 'http://localhost:8080/cgi/test.php?name=mlaussel' \ -H 'Content-Type: text/plain' \ -d 'Hello World!'

    // step 1 : firstline extract and parsing
    if (firstLine(p, request) == 1)
    {
       errorCode(response, serverConfig, 400);
       return ;
    }

    // step 2 : headers parsing
    if (headers(p, request) == 1)
    {
       errorCode(response, serverConfig, 400);
       return ;
    }

    // step 3 : if POST method, read body
    if (request._method == "POST")
        postBody(p, request);

    if (foundLocation(request, serverConfig, response) == false)
    {
        errorCode(response, serverConfig, 404);
        return ;
    }
    
    // step 4 : CGI
    cgiMain(request, cgi, serverConfig, response);
    // debug(response);

    if (checkIs(request, response) == false)
    {
        errorCode(response, serverConfig, 405);
        return ;
    }
}