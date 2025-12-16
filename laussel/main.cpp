/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 15:14:41 by mathildelau       #+#    #+#             */
/*   Updated: 2025/12/16 14:15:04 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream> //cout
#include <unistd.h> //read
#include <fcntl.h>  //open
#include "Request.hpp"
#include "Response.hpp"

void debug1(request &request)
{
    std::cout << "--- REQUEST ---\n";
    std::cout << "Method: " << request._method << "\n";
    std::cout << "URL: " << request._url << "\n";
    std::cout << "Version: " << request._version << "\n";
    std::cout << "Headers:\n";
    for (std::map<std::string, std::string>::iterator it = request.headers.begin(); it != request.headers.end(); ++it)
        std::cout << "  " << it->first << ": " << it->second << "\n";
    if (!request._body.empty())
        std::cout << "Body:\n"
                  << request._body << "\n";
}


void debug2(responseT &response)
{
    std::cout << "\n--- RESPONSE ---\n";
    std::cout << response.response << "\n";
}

int main(int argc, char **argv)
{
    (void) argv; //delete

    
    // step 0 : check number of args
    if (argc != 1)
    {
        std::cout << "Usage : ./webserv \n";
        return (1);
    }
    
    // step 1 : request parsing
    request request;
    parsingT p;
    if (requestMain(request, p) == 1)
        return (1);
    debug1(request);

    //step 2 : response
    responseT response;
    if (responseMain(request, response) == 1)
        return (1);
    debug2(response);

    return (0);
}