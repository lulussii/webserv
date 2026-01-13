/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Init.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 08:52:02 by mlaussel          #+#    #+#             */
/*   Updated: 2026/01/13 08:52:41 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INIT_HPP
#define INIT_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"

void initMain(request &request, responseT &response, serverT &serverConfig);

#endif