/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 15:01:32 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/23 12:13:26 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunked.hpp"
#include <stdlib.h> //atoi
#include <cstdio> //sscanf

/**
 * @brief `Check if the request uses chunked transfer encoding`
 * 
 * step 1 : Look for the "Transfer-Encoding" header in the request headers map.
 * 
 * step 2 : If the header exists and its value is exactly "chunked", the request body must be decoded as chunked.
 *
 */
bool isChunked(request &request)
{
    std::map<std::string, std::string>::iterator it = request.headers.find("Transfer-Encoding");

    if (it != request.headers.end() && it->second == " chunked")
    {
        return (true);
    }
    return (false);
}

/**
 * @brief `Parse a HTTP body using "Transfer-Encoding: chunked" and rebuild the raw body.`
 *
 * Exemple "11\r\nHello World\r\n5\r\n12345\r\n0\r\n\r\n"
 * Expected : "Hello World12345"
 * 
 * step 1 : Initialize a temporary buffer containing the raw chunked body.
 *
 * step 2 : Loop while the temporary buffer is not empty.
 *
 * step 3 : Find the ("\r\n" or "\n") separator that ends the chunk size line.
 * 
 * step 4 : Extract the chunk size substring (hexadecimal value) from the beginning of the buffer until the separator.
 *
 * step 5 : Convert the extracted chunk size from hexadecimal to an unsigned long.
 * If conversion fails, return HTTP 400 (Bad Request).
 *
 * step 6 : If the chunk size is equal to 0, stop parsing.
 * This indicates the end of the chunked body.
 *
 * step 7 : Remove the chunk size line and its ("\r\n" or "\n") from the temporary buffer.
 *
 * step 8 : Copy exactly `chunkSize` bytes from the temporary buffer into the new body.
 * 
 * step 9 : Remove the copied chunk data and the following CRLF from the buffer.
 *
 */
int chunkedParsing(request &request, responseT &response)
{
    std::string tmp = request._body;
    std::string cut;
    std::string newBody = "";
    int flag = 0;

    while (!tmp.empty())
    {
        if (flag == 0)
        {
            size_t pos = tmp.find("\r\n");
            size_t spaceSize = 2;
            if (pos == std::string::npos)
            {
                pos = tmp.find("\n");
                if (pos == std::string::npos)
                    return (400);
                spaceSize = 1;
            }

            tmp = tmp.substr(pos + spaceSize);
        }
        flag = 1;

        size_t pos = tmp.find("\r\n");
        size_t spaceSize = 2;
        if (pos == std::string::npos)
        {
            pos = tmp.find("\n");
            if (pos == std::string::npos)
                return (400);
            spaceSize = 1;
        }

        cut = tmp.substr(0, pos);

        unsigned long chunkSize;
        if (sscanf(cut.c_str(), "%lx", &chunkSize) != 1)
            return (400);
        
        if (chunkSize == 0)
            break;

        tmp = tmp.substr(pos + spaceSize);

        newBody += tmp.substr(0, chunkSize);

        tmp = tmp.substr(chunkSize + spaceSize);
    }
    response.body = newBody;
    return (0);
}