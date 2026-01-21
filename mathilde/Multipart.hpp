/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Multipart.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 15:04:45 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/21 15:11:41 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPART_HPP
#define MULTIPART_HPP

class request;
#include <iostream>

class Multipart
{
public:
    std::string fullPart;
    std::string filename;
    std::string name;
    std::string contentType;
    std::string content;
};

bool isMultipart(request &request);
void extractBundary(request &request);
void splitPart(request &request);
void extractFileName(request &request);

#endif