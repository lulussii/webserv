/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:24:02 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/23 16:51:31 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include "Config.hpp"
#include "Error.hpp"
#include "Response.hpp"

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
static int firstLine(request &request)
{
    // step 1 : extract the first line
    size_t i = request.lineRequest.find("\r\n");

    if (i == std::string::npos)
    {
        i = request.lineRequest.find('\n');
        if (i == std::string::npos) // security for find, if doesn't find, find return the bigger size_t
            return (1);
    }
    std::string firstLine = request.lineRequest.substr(0, i);
    
    size_t skip = 2;
    if (request.lineRequest[i] == '\n') // if end of the line with only \n
        skip = 1;
    request.lineRequest = request.lineRequest.substr(i + skip); // remove \n or \r\n end first line

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
 * step 1 : Loop on each line from request.lineRequest (without first line)
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
static int headers(request &request)
{
    while (true)
    {
        size_t end = request.lineRequest.find("\r\n");
        size_t skip = 2; // if \r\n

        if (end == std::string::npos)
        {
            end = request.lineRequest.find('\n');
            skip = 1;
        }
        if (end == std::string::npos)
            break;

        std::string headerLine = request.lineRequest.substr(0, end);
        
        request.lineRequest = request.lineRequest.substr(end + skip); // delete the line already read

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
static void postBody(request &request)
{
    request._body = request.lineRequest;
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
void requestMainNew(request &request, serverT &serverConfig, responseT &response)
{
    std::cout << "[REQUEST] :\n" << request.lineRequest << std::endl;
    
    // step 1 : firstline extract and parsing
    if (firstLine(request) == 1)
    {
       errorCode(response, serverConfig, 400);
       return ;
    }

    // step 2 : headers parsing
    if (headers(request) == 1)
    {
       errorCode(response, serverConfig, 400);
       return ;
    }
   

    // step 3 : if POST method, read body
    if (request._method == "POST")
        postBody(request);
    
    if (foundLocation(request, serverConfig, response) == false)
    {
        errorCode(response, serverConfig, 404);
        return ;
    }
   
    if (checkIs(request, response) == false)
    {
        errorCode(response, serverConfig, 405);
        return ;
    }
}

