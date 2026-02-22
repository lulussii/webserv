/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Init.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 08:45:27 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/22 19:08:03 by mathildelau      ###   ########.fr       */
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

void initMain(request &request, responseT &response)
{
    initRequest(request);
    initResponse(response);
}