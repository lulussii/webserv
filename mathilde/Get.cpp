/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 15:38:59 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/06 16:03:00 by mathildelau      ###   ########.fr       */
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
    // step 1 : check if method is in server
    if (checkIsGet(request, serverConfig) == 0)
    {
        std::cout << "Error :405 Method Not Allowed\n";
        return (1);
    }
    else // delete later
        std::cout << "GOOD\n";

    // step 2 : find the good location

    return (0);
}