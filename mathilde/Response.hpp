/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:11 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/03 15:47:56 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include "Config.hpp"
class request;


typedef struct infos
{
    bool error;
    bool get;
    bool loc;
    bool repository;
} infosT;

typedef struct post
{
    std::string path;
} postT;

typedef struct response
{
    std::string response;

    int code;
    size_t contentLen;
    std::string contentType;
    std::string body;

    std::string path;
    std::string repo;
    locationsT location;
    infosT infos;
    postT post;
    std::string filename;
    bool cgi;
} responseT;

void responseMain(request &request, responseT &response);

#endif