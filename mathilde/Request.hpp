/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 15:49:28 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/03 10:55:14 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>
#include <vector>
#include "Multipart.hpp"

class request
{
public:
    std::string _method;
    std::string _url;
    std::string _version;
    std::map<std::string, std::string> headers;
    std::string boundary;
    std::vector<Multipart> party;
    std::string _body;
    size_t contentLenght;
};

typedef struct parsingS
{
    std::string line;
} parsingT;

int requestMain(request &request, parsingT &p, serverT &serverConfig, utilsConfigT &utils, responseT &response);
std::string trim(const std::string &str);

#endif