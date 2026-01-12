/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:09 by mlaussel          #+#    #+#             */
/*   Updated: 2026/01/12 16:16:04 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include <string> //to_string()

int responseMain(request &request, responseT &response)
{
    // status line :
    response.response = request._version + " " + std::to_string(response.code);
    if (response.infos.error == false)
        response.response += " OK\r\n";
    if (response.code == 404)
        response.response += " Not Found\r\n";
    if (response.code == 403)
        response.response += " Forbidden\r\n";
    if (response.code == 405)
        response.response += " Method Not Allowed\r\n";
    if (response.code == 413)
        response.response += " Payload Too Large\r\n";
    if (response.code == 400)
        response.response += " Bad Request\r\n";
    if (response.code == 500)
        response.response += " Server Error\r\n";
    // header in response
    response.response += "Content-Length: " + std::to_string(response.contentLen);
    response.response += "\r\n";

    response.response += "Content-Type: " + response.contentType;
    response.response += "\r\n";

    // empty line
    response.response += "\r\n";

    // body
    response.response += response.body;

    return (0);
}