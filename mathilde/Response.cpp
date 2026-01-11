/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:09 by mlaussel          #+#    #+#             */
/*   Updated: 2026/01/11 11:53:01 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Request.hpp"
#include <stdio.h> //sprintf
#include <string>  //to_string()

int responseMain(request &request, responseT &response)
{
    // status line :
    response.response = request._version + " " + std::to_string(response.code) + " OK\r\n";

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