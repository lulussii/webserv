/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 15:00:55 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/02 10:39:58 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNKED_HPP
#define CHUNKED_HPP

#include "Request.hpp"
#include "Response.hpp"

bool isChunked(request &request);
int chunkedParsing(request &request, responseT &response);

#endif