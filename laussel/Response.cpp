/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 13:27:09 by mlaussel          #+#    #+#             */
/*   Updated: 2025/12/16 14:14:18 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "Request.hpp"

/*

Format : HTTP/<version> <status_code> <reason_phrase>\r\n

Notion clé :
    - HTTP/1.1 → version du protocole.
    - 200 → code d’état (200 = succès).
    - OK → phrase explicative (toujours obligatoire).

Plus tard, tu auras d’autres codes : 404 (Not Found), 500 (Server Error), etc.

*/

int responseMain(request &request, responseT &response)
{
    
    response.response = request._version + " 200 " + "OK\r\n";
    for (std::map<std::string, std::string>::iterator it = request.headers.begin(); it != request.headers.end(); ++it)
        response.response = response.response + it->first + " : " + it->second + "\r\n";
    
    response.response = response.response + "\r\n";

    response.response = response.response + request._body;
    
    return (0);
}