/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:09 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/04 17:27:18 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Request.hpp"
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

    std::map<int, std::string>::iterator it = response.errorTxt.find(response.code);
    if (it != response.errorTxt.end())
        response.response += " " + it->second;
    // if (response.code == 200)
    //     response.response += " OK\r\n";
    // if (response.code == 404)
    //     response.response += " Not Found\r\n";
    // if (response.code == 403)
    //     response.response += " Forbidden\r\n";
    // if (response.code == 405)
    //     response.response += " Method Not Allowed\r\n";
    // if (response.code == 413)
    //     response.response += " Payload Too Large\r\n";
    // if (response.code == 400)
    //     response.response += " Bad Request\r\n";
    // if (response.code == 500)
    //     response.response += " Server Error\r\n";
    // if (response.code == 201)
    //     response.response += " Created\r\n";
    // if (response.code == 204)
    //     response.response += " No Content\r\n";

    if (request._method == "DELETE" || response.code == 413)
        response.response += "Content-Length: 0";
    else
        response.response += "Content-Length: " + contentL.str();
    response.response += "\r\n";

    if (response.contentType.empty())
        response.contentType = "text/plain";

    response.response += "Content-Type: " + response.contentType;
    response.response += "\r\n";

    // empty line
    response.response += "\r\n";

    // body
    if (response.infos.error == true || request._method == "GET")
        response.response += response.body;
}