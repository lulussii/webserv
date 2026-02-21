/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 08:45:27 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/20 16:47:10 by mathildelau      ###   ########.fr       */
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
    response.errorTxt[413] = "Content Too Large";
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
    request.lineRequest = "";
}

// void initCgi(cgi &cgi)
// {
//     cgi.contentLenght = "";
//     cgi.contentType = "";
//     cgi.method = "";
//     cgi.queryString = "";
//     cgi.scriptPath = "";
//     cgi.binaryPath = "";
//     cgi.serverName = "";
//     cgi.serverPort = "";
//     cgi.gatewayInterface = "";
//     cgi.serverProtocol = "";
//     cgi.body = "";
//     cgi.code = "200";
//     cgi.isCgi = false;
//     cgi.writePipe[0] = -1;
//     cgi.writePipe[1] = -1;
//     cgi.readPipe[0] = -1;
//     cgi.readPipe[1] = -1;
//     cgi.writeBuffer = "";
//     cgi.readBuffer = "";
//     cgi.writing = false;
//     cgi.reading = false;
//     cgi.pid = -1;

//     cgi.errorTxt["200"] = "OK";
//     cgi.errorTxt["201"] = "Created";
//     cgi.errorTxt["204"] = "No Content";
//     cgi.errorTxt["400"] = "Bad Request";
//     cgi.errorTxt["403"] = "Forbidden";
//     cgi.errorTxt["404"] = "Not Found";
//     cgi.errorTxt["405"] = "Method Not Allowed";
//     cgi.errorTxt["413"] = "Content Too Large";
//     cgi.errorTxt["500"] = "Internal Server Error";
// }

void initMain(request &request, responseT &response, cgi &cgi)
{
    initRequest(request);
    initResponse(response);
    (void) cgi;
    // initCgi(cgi);
}