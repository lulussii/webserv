/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 15:04:14 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/19 09:44:19 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERROR_HPP
#define ERROR_HPP

#include "Response.hpp"
#include "Request.hpp"
#include "Config.hpp"

void errorCode(responseT &response, serverT &serverConfig, int code);

#endif