/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 15:38:43 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/22 19:04:17 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_HPP
#define GET_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"

int getMain(request &request, responseT &response, serverT &serverConfig);
int readFile(responseT &response);

#endif