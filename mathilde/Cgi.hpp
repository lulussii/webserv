/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 11:18:05 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/02 12:38:05 by mlaussel         ###   ########.fr       */
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

#endif