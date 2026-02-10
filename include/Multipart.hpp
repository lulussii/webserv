/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Multipart.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 15:04:45 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/10 10:23:59 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPART_HPP
#define MULTIPART_HPP

#include <iostream>
#include "Response.hpp"
struct request;

typedef struct Multipart
{
    std::string fullPart;
    std::string filename;
    std::string name;
    std::string contentType;
    std::string content;
} Multipart;

bool isMultipart(request &request);
int extractBundary(request &request);
void splitPart(request &request, responseT &response, serverT &serverConfig, Multipart &m);
void extractName(Multipart &m);
int extractFileName(Multipart &m);
void extractContentType(Multipart &m);
void extractContent(Multipart &m);
int createAndWriteMultipartFile(responseT &response, Multipart &m);

#endif