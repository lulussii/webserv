/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 15:04:14 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/12 16:16:24 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERROR_HPP
#define ERROR_HPP

#include "Response.hpp"
#include "Request.hpp"
#include "Config.hpp"

void errorCode(responseT &response, serverT &serverConfig, request &request);
void error404(responseT &response, serverT &serverConfig, request &request);
void error403(responseT &response, serverT &serverConfig, request &request);
void error405(responseT &response);
void error413(responseT &response);
void error400(responseT &response);
void error500(responseT &response);

#endif