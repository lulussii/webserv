/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 15:14:41 by mathildelau       #+#    #+#             */
/*   Updated: 2025/12/15 15:31:55 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream> //cout
#include <unistd.h> //read
#include <fcntl.h>  //open
#include "Request.hpp"
#include "parsing.hpp"

void debug1(request &request)
{
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

int main(int argc, char **argv)
{
    // step 0 : check number of args
    if (argc != 2)
    {
        std::cout << "Usage : ./webserv request.txt\n";
        return (1);
    }

    request request;
    parsingT p;

    // step 1 : read request file and pars it
    if (parsingMain(argv, request, p) == 1)
        return (1);

    // step 2 : minimal HTTP servor 
    
    debug1(request);

    return (0);
}