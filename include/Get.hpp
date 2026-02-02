/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Get.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/06 15:38:43 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/29 10:50:41 by lserodon         ###   ########.fr       */
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