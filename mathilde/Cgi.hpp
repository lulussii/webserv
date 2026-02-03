/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 11:18:05 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/03 09:57:07 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include "Request.hpp"
#include "Config.hpp"

class cgi
{
    public :
        std::string method;
        std::string queryString;
        std::string contentLenght;
        std::string contentType;
        std::string scriptPath;
        std::string serverName;
        std::string serverPort;    
};

int cgiMain(request &request, cgi &cgi, serverT &serverConfig);

#endif