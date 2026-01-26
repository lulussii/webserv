/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Multipart.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 15:04:45 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/26 12:58:32 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPART_HPP
#define MULTIPART_HPP

#include <iostream>
#include "Response.hpp"
class request;

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
void splitPart(request &reques, responseT &response, serverT &serverConfig);
void extractName(Multipart &m);
int extractFileName(Multipart &m);
void extractContentType(Multipart &m);
void extractContent(Multipart &m);
int createAndWriteMultipartFile(responseT &response, Multipart &m);

#endif