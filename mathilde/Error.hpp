/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 15:04:14 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/11 15:06:30 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ERROR_HPP
#define ERROR_HPP

void errorCode(responseT &response, serverT &serverConfig, request &request);
void error404(responseT &response, serverT &serverConfig, request &request);
void error403(responseT &response, serverT &serverConfig, request &request);
void error405(responseT &response);

#endif