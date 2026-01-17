/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Error.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 14:47:21 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/17 17:35:51 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Error.hpp"
#include "Get.hpp"

/**
 * @brief `search html body for error`
 *
 * if code is 404 so the new path is root/404.html
 */
void errorCode(responseT &response, serverT &serverConfig, request &request)
{
    (void) request;
    response.infos.error = true;
    response.contentType += "text/html";
    
    std::map<int, std::string>::iterator it =
        serverConfig.errorPage.find(response.code);

    if (it != serverConfig.errorPage.end())
    {
        response.path = serverConfig.root + it->second;

        if (readFile(response) == 0 && !response.body.empty())
        {
            response.contentLen = response.body.size();
            return;
        }
    }
    response.body = "";
}

void error404(responseT &response, serverT &serverConfig, request &request)
{
    response.code = 404;
    errorCode(response, serverConfig, request);
    response.contentLen = response.body.size();
}

void error403(responseT &response, serverT &serverConfig, request &request)
{
    response.code = 403;
    errorCode(response, serverConfig, request);
    response.contentLen = response.body.size();
}

void error405(responseT &response)
{
    response.code = 405;
    response.body = "";
    response.contentLen = response.body.size();
}

void error413(responseT &response)
{
    response.code = 413;
    response.body = "";
    response.contentLen = response.body.size();
}

void error400(responseT &response)
{
    response.code = 400;
    response.body = "";
    response.contentLen = response.body.size();
}

void error500(responseT &response)
{
    response.code = 500;
    response.body = "";
    response.contentLen = response.body.size();
}
