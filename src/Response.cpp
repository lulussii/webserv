/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:09 by mlaussel          #+#    #+#             */
/*   Updated: 2026/01/19 11:09:07 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include <sstream> //std::stringstream

/**
 * @brief `POST method main`
 *
 *
 * step 1 : status line
 *
 * step 2 : Content Lenght
 *
 * step 3 : Content-Type
 *
 * step 4 : empty line
 *
 * step 5 : body
 *
 */
void responseMain(request &request, responseT &response)
{
    std::stringstream code;
    code << response.code;

    std::stringstream contentL;
    contentL << response.contentLen;
    response.response = request._version + " " + code.str();
    if (response.code == 200)
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
    if (response.code == 201)
        response.response += " Created\r\n";
    if (response.code == 204)
        response.response += " No Content\r\n";
        
    if (request._method == "DELETE")
        response.response += "Content-Length: 0";
    else
        response.response += "Content-Length: " + contentL.str();
    response.response += "\r\n";

    response.response += "Content-Type: " + response.contentType;
    response.response += "\r\n";

    // empty line
    response.response += "\r\n";

    // body
    if (request._method == "GET" || request._method == "POST")
        response.response += response.body;
}