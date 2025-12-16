/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/09 15:49:28 by mathildelau       #+#    #+#             */
/*   Updated: 2025/12/15 14:29:51 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>


class request
{
public:
    std::string _method;
    std::string _url;
    std::string _version;
    std::map<std::string, std::string> headers;
    std::string _body;
};

#endif