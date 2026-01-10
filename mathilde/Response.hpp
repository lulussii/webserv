/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:11 by mlaussel          #+#    #+#             */
/*   Updated: 2026/01/10 16:28:48 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Request.hpp"

typedef struct infos
{
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
    std::string path;
    locationsT *location;
    infosT infos;
} responseT;

int responseMain(request &request, responseT &response);