/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 08:45:27 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/03 15:48:12 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Cgi.hpp"

void initResponse(responseT &response)
{
    response.response = "";
    response.code = 200;
    response.contentLen = 0;
    response.contentType = "";
    response.body = "";
    response.cgi = false;

    response.path = "";
    response.repo = "";

    response.infos.error = false;
    response.infos.get = false;
    response.infos.loc = false;
    response.infos.repository = false;

    response.post.path = "";

    response.location.path = "";
    response.location.index = "";
    response.location.autoindex = "";
    response.location.upload_dir = "";
}

void initRequest(request &request)
{
    request._method = "";
    request._url = "";
    request._version = "";
    request.boundary = "";
    request._body = "";
    request.contentLenght = 0;
}

void initConfig(serverT &serverConfigS)
{
    serverConfigS.listen = 0;
    serverConfigS.root = "";
    serverConfigS.clientMaxBodySize = 0;
}

void initCgi(cgi &cgi)
{
    cgi.contentLenght = "";
    cgi.contentType = "";
    cgi.method = "";
    cgi.queryString = "";
    cgi.scriptPath = "";
    cgi.binaryPath = "";
    cgi.serverName = "";
    cgi.serverPort = "";
}
void initMain(request &request, responseT &response, serverT &serverConfig, cgi &cgi)
{
    initRequest(request);
    initConfig(serverConfig);
    initResponse(response);
    initCgi(cgi);
}