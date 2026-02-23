/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 14:47:21 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/23 11:47:14 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Error.hpp"
#include "Get.hpp"

/**
 * @brief `Search HTML body for error`
 * 
 * step 1 : Set the response code and mark the response as an error.
 * 
 * step 2 : Default content type is "text/plain".
 * 
 * step 3 : Search for a custom error page in the server configuration.
 *    - Check if the code exists in serverConfig.errorPage map.
 * 
 * step 4 : If a custom error page exists:
 *    - Build the full path: serverConfig.root + error page file name.
 *    - Set content type to "text/html".
 *    - Read the file into the response body.
 * 
 * step 5 : If the file is successfully read and not empty:
 *    - Set content length to the body size and return.
 * 
 * step 6 : If no custom error page exists or reading failed:
 *    - Set body to empty and content length to 0.
 * 
 */
void errorCode(responseT &response, serverT &serverConfig, int code)
{
    response.code = code;
    
    response.infos.error = true;
    
    response.contentType = "text/plain";
    
    std::map<int, std::string>::iterator it = serverConfig.errorPage.find(response.code);
    if (it != serverConfig.errorPage.end())
    {
        response.path = serverConfig.root + it->second;
        response.contentType = "text/html";

        if (readFile(response) == 0 && !response.body.empty())
        {
            response.contentLen = response.body.size();
            return;
        }
    }
    
    response.body = "";
    
    response.contentLen = 0;
}
