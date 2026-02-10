/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Init.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 08:52:02 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/10 10:39:22 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INIT_HPP
#define INIT_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "Config.hpp"
#include "Cgi.hpp"

void initMain(request &request, responseT &response, cgi &cgi);

#endif