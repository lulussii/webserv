/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 15:49:28 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/25 15:42:55 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>
#include <vector>
#include "Multipart.hpp"

struct cgi;

typedef struct request
{
    std::string lineRequest;
    std::string _method;
    std::string _url;
    std::string _version;
    std::map<std::string, std::string> headers;
    std::string boundary;
    std::vector<Multipart> party;
    std::string _body;
    size_t contentLenght;
} request ;

typedef struct parsingS
{
    std::string line;
} parsingT;


void requestMain(request &request, serverT &serverConfig, responseT &response);


#endif