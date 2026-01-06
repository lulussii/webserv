/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 15:38:59 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/06 15:55:41 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Get.hpp"

bool checkIsGet(request &request, serverT &serverConfig)
{
    for (std::map<std::string, locationsT>::iterator it = serverConfig.locations.begin();
         it != serverConfig.locations.end(); ++it)
    {
        locationsT &location = it->second;
        for (size_t i = 0; i < location.methods.size(); ++i)
            if (location.methods[i] == request._method)
                return (true);
    }
    return (false);
}

int getMain(request &request, responseT &response, serverT &serverConfig)
{
    (void)response;
    if (checkIsGet(request, serverConfig) == 0)
    {
        std::cout << "Error :405 Method Not Allowed\n";
        return (1);
    }
    else
        std::cout << "GOOD\n";
    return (0);
}