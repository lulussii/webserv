/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:11 by mlaussel          #+#    #+#             */
/*   Updated: 2026/01/10 12:30:20 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Request.hpp"

typedef struct response
{
    std::string response;
    std::string path;
    locationsT *location;
    bool get;
    bool loc;
    bool fileExist;
    bool file;
    bool repository;
    bool read;
    
} responseT;

int responseMain(request &request, responseT &response);