/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 08:45:27 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/04 17:26:16 by mathildelau      ###   ########.fr       */
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

    response.errorTxt[200] = "OK";
    response.errorTxt[201] = "Created";
    response.errorTxt[204] = "No Content";
    response.errorTxt[400] = "Bad Request";
    response.errorTxt[403] = "Forbidden";
    response.errorTxt[404] = "Not Found";
    response.errorTxt[405] = "Method Not Allowed";
    response.errorTxt[413] = "Payload Too Large";
    response.errorTxt[500] = "Internal Server Error";
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
    cgi.gatewayInterface = "";
    cgi.serverProtocol = "";
    cgi.body = "";
    cgi.code = "200";

    cgi.errorTxt["200"] = "OK";
    cgi.errorTxt["201"] = "Created";
    cgi.errorTxt["204"] = "No Content";
    cgi.errorTxt["400"] = "Bad Request";
    cgi.errorTxt["403"] = "Forbidden";
    cgi.errorTxt["404"] = "Not Found";
    cgi.errorTxt["405"] = "Method Not Allowed";
    cgi.errorTxt["413"] = "Payload Too Large";
    cgi.errorTxt["500"] = "Internal Server Error";
}

void initMain(request &request, responseT &response, serverT &serverConfig, cgi &cgi)
{
    initRequest(request);
    initConfig(serverConfig);
    initResponse(response);
    initCgi(cgi);
}