/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Delete.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 12:06:02 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/03 11:21:33 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DELETE_HPP
#define DELETE_HPP

#include "Request.hpp"
#include "Config.hpp"
#include "Response.hpp"

void deleteMain(request &request, responseT &response, serverT &serverConfig);

#endif