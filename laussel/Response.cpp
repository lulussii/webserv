/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:09 by mlaussel          #+#    #+#             */
/*   Updated: 2025/12/22 15:46:19 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Request.hpp"
#include <stdio.h> //sprintf

/*

Format : HTTP/<version> <status_code> <reason_phrase>\r\n

Notion clé :
    - HTTP/1.1 → version du protocole.
    - 200 → code d’état (200 = succès).
    - OK → phrase explicative (toujours obligatoire).

Plus tard, tu auras d’autres codes : 404 (Not Found), 500 (Server Error), etc.

*/

int responseMain(request &request, responseT &response)
{
    // status line : must add a int for 200 and a bool for OK
    response.response = request._version + " 200 " + "OK\r\n";

    // header in response
    response.response += "Content-Length: ";
    size_t bodySize = request._body.size();
    char buf[20];
    //sprintf(buf, "%zu", bodySize); //linux
    snprintf(buf, sizeof(buf), "%zu", bodySize); //macos
    response.response += buf;
    response.response += "\r\n";

    response.response += "Content-Type: text/plain";
    response.response += "\r\n";

    // empty line
    response.response += "\r\n";

    // body
    response.response += request._body;

    return (0);
}