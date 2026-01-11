/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 15:38:43 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/11 13:06:00 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_HPP
#define GET_HPP

#include "Config.hpp"
#include "Response.hpp"

typedef struct getS
{
    locationsT location;    
} getT;

int getMain(request &request, responseT &response, serverT &serverConfig);
int readFile(responseT &response);

#endif