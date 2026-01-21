/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 08:45:27 by mlaussel          #+#    #+#             */
/*   Updated: 2026/01/20 12:29:28 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Request.hpp"
#include "Response.hpp"

void initResponse(responseT &response)
{
    response.response = "";
    response.code = 200;
    response.contentLen = 0;
    response.contentType = "";
    response.body = "";

    response.path = "";
    response.repo = "";

    response.infos.error = false;
    response.infos.get = false;
    response.infos.loc = false;
    response.infos.fileExist = false;
    response.infos.file = false;
    response.infos.repository = false;
    response.infos.read = false;

    response.post.count = 0;
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

void initMain(request &request, responseT &response, serverT &serverConfig)
{
    initRequest(request);
    initConfig(serverConfig);
    initResponse(response);
}