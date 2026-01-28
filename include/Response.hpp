/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:11 by mlaussel          #+#    #+#             */
/*   Updated: 2026/01/28 15:38:08 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include "Config.hpp"
#include "Request.hpp"

class request;

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

struct responseT
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
};

void responseMain(request &request, responseT &response);

#endif