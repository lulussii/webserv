/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:11 by mlaussel          #+#    #+#             */
/*   Updated: 2026/01/19 10:30:33 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include "Config.hpp"
#include "Request.hpp"

typedef struct infos
{
    bool error;
    bool get;
    bool loc;
    bool fileExist;
    bool file;
    bool repository;
    bool read;
} infosT;

typedef struct post
{
    int count;
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
} responseT;

void responseMain(request &request, responseT &response);

#endif