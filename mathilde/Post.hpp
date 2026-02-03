/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 13:28:43 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/03 16:04:38 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POST_HPP
#define POST_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"

void postMain(request &request, responseT &response, serverT &serverConfig, cgi &cgi);

#endif