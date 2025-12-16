/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 15:49:28 by mathildelau       #+#    #+#             */
/*   Updated: 2025/12/16 13:22:26 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>

class request
{
public:
    std::string _method;
    std::string _url;
    std::string _version;
    std::map<std::string, std::string> headers;
    std::string _body;
    std::string headersPart;
    std::string bodyPart;
};

typedef struct parsingS
{
    std::string line;
} parsingT;

int requestMain(request &request, parsingT &p);

#endif