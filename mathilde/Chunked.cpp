/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 15:01:32 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/21 15:01:57 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Chunked.hpp"
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
 * step 4 : check if lenght == 0, return if == 0, means end body
 *
 * step 5 : delete len part in body, tmp = \r\nHello World\r\n5\r\n12345\r\n0\r\n\r\n
 * + 2 because we don't want \r\n --> Hello World\r\n5\r\n12345\r\n0\r\n\r\n
 *
 * step 6 : in newBody, put the lenght string ask, here len = 11 so we copy Hello World
 *
 * step 7 : we delete the string part, tmp = 5\r\n12345\r\n0\r\n\r\n
 *
 */
void chunkedParsing(request &request, responseT &response)
{
    std::string tmp = request._body;
    std::string cut;
    std::string newBody = "";

    while (!tmp.empty())
    {
        size_t space = tmp.find("\r\n");

        cut = tmp.substr(0, space);
        if (static_cast<size_t>(atoi(cut.c_str())) == 0)
            break;

        tmp = tmp.substr(space + 2);

        newBody += tmp.substr(0, static_cast<size_t>(atoi(cut.c_str())));

        tmp = tmp.substr(static_cast<size_t>(atoi(cut.c_str())) + 2);
    }
    response.body = newBody;
}