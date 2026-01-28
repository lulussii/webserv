/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Chunked.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 15:00:55 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/21 15:03:22 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHUNKED_HPP
#define CHUNKED_HPP

#include "Request.hpp"
#include "Response.hpp"

bool isChunked(request &request);
void chunkedParsing(request &request, responseT &response);

#endif