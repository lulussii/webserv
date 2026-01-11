/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:11 by mlaussel          #+#    #+#             */
/*   Updated: 2026/01/11 11:38:37 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
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

typedef struct response
{
    std::string response;
    
    int code;
    size_t contentLen;
    std::string body;
    std::string contentType;
    
    std::string path;
    locationsT *location;
    infosT infos;
} responseT;

int responseMain(request &request, responseT &response);