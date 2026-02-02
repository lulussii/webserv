/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 15:01:32 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/02 10:41:06 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunked.hpp"
#include <stdlib.h> //atoi
#include <cstdio> //sscanf

/**
 * @brief `Check if is Chunked`
 *
 */
bool isChunked(request &request)
{
    std::map<std::string, std::string>::iterator it = request.headers.find("Transfer-Encoding");

    if (it != request.headers.end() && it->second == "chunked")
        return (true);
    return (false);
}

/**
 * @brief `Split body part to delete lenght part`
 *
 * Exemple "11\r\nHello World\r\n5\r\n12345\r\n0\r\n\r\n"
 * step 1 : loop on the body
 *
 * step 2 : search \r\n who is a separator between len and string
 *
 * step 3 : in cut, extract lenght part (cut = [11])
 * 
 * step 4 : check if cut is hexa : 
 *
 * step 5 : check if lenght == 0, return if == 0, means end body
 *
 * step 6 : delete len part in body, tmp = \r\nHello World\r\n5\r\n12345\r\n0\r\n\r\n
 * + 2 because we don't want \r\n --> Hello World\r\n5\r\n12345\r\n0\r\n\r\n
 *
 * step 7 : in newBody, put the lenght string ask, here len = 11 so we copy Hello World
 *
 * step 8 : we delete the string part, tmp = 5\r\n12345\r\n0\r\n\r\n
 *
 */
int chunkedParsing(request &request, responseT &response)
{
    std::string tmp = request._body;
    std::string cut;
    std::string newBody = "";

    while (!tmp.empty())
    {
        size_t space = tmp.find("\r\n");

        cut = tmp.substr(0, space);

        unsigned long t;
        if (sscanf(cut.c_str(), "%lx", &t) != 1)
            return (400);
            
        if (static_cast<size_t>(atoi(cut.c_str())) == 0)
            break;

        tmp = tmp.substr(space + 2);

        newBody += tmp.substr(0, static_cast<size_t>(atoi(cut.c_str())));

        tmp = tmp.substr(static_cast<size_t>(atoi(cut.c_str())) + 2);
        
    }
    response.body = newBody;
    return (0);
}