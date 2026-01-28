/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Multipart.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 15:04:45 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/28 15:37:29 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPART_HPP
#define MULTIPART_HPP

#include <iostream>

class request;

struct responseT;
struct serverT;
class Multipart;

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
int extractBundary(request &request);
void splitPart(request &reques, responseT &response, serverT &serverConfig);
void extractName(Multipart &m);
int extractFileName(Multipart &m);
void extractContentType(Multipart &m);
void extractContent(Multipart &m);
int createAndWriteMultipartFile(responseT &response, Multipart &m);

#endif