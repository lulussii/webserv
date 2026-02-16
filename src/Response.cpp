/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:09 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/16 10:12:07 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Request.hpp"
#include <sstream> //std::stringstream

/**
 * @brief `Build full HTTP response`
 *
 * step 1 : Start with the status line: "<HTTP version> <status code> <reason phrase>".
 * 
 * step 2 : Add the "Content-Length" header:
 *    - 0 if DELETE method or code 413.
 *    - Otherwise, use the size of the body.
 * 
 * step 3 : Add the "Content-Type" header:
 *    - Default to "text/plain" if not set.
 * 
 * step 4 : Add an empty line to separate headers from the body.
 * 
 * step 5 : Append the body if:
 *    - Response is an error.
 *    - Or request method is GET.
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
    response.response += "\r\n";

    if (request._method == "DELETE" || response.code == 413)
        response.response += "Content-Length: 0";
    else
        response.response += "Content-Length: " + contentL.str();
    response.response += "\r\n";

    if (response.contentType.empty())
        response.contentType = "text/plain";
    response.response += "Content-Type: " + response.contentType;
    response.response += "\r\n";

    response.response += "\r\n";

    // if (response.infos.error == true || request._method == "GET")
        response.response += response.body;
    response.response += "\r\n";
}