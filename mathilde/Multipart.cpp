/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Multipart.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 15:04:09 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/21 15:17:26 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Multipart.hpp"
#include "Request.hpp"
/**
 * @brief `Check if is multipart/form-data`
 *
 */
bool isMultipart(request &request)
{
    std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");

    if (it != request.headers.end())
    {
        if (it->second.find("multipart/form-data;") != std::string::npos)
            return (true);
    }
    return (false);
}

/**
 * @brief `extraxt bundary name`
 *
 * step 1 : search in Content-Type "bundary="
 *
 * step 2 : extract what is after =
 */
void extractBundary(request &request)
{
    std::string s = "boundary=";
    std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");

    if (it != request.headers.end())
    {
        size_t pos = it->second.find("boundary=");
        std::string tmp = it->second.substr(pos + s.size());
        request.boundary = tmp;
    }
    else
        request.boundary = "boundary";
}

/**
 * @brief `split all part separate by bundary`
 *
 * step 1 : use a copy of body in tmp
 *
 * step 2 : loop while we are not at the end (--bundary--)
 *
 * step 3 : delete first bundary part in tmp (--bundary) 2 for --, and for \r\n
 *
 * step 4 : search --bundary in tmp, if not, search end --bundary--
 *
 * step 5 : create new string without --bundary or end, - 1 for space
 *
 * step 6 : add full part part to class Multipart
 */
void splitPart(request &request)
{
    std::string tmp = request._body;
    request.party.clear();

    while (tmp.find(request.boundary) != std::string::npos)
    {
        tmp = tmp.substr(2 + request.boundary.size() + 2);

        size_t end = tmp.find("--" + request.boundary);
        if (end == std::string::npos)
            end = tmp.find("--" + request.boundary + "--");

        std::string fullPart = tmp.substr(0, end - 1);

        Multipart m;
        m.fullPart = fullPart;
        request.party.push_back(m);
        tmp = tmp.substr(end);
        std::cout << "full part = [" << m.fullPart << "]\n";
        std::cout << "tmp = [" << tmp << "]\n";
    }
}

/**
 * @brief `find filename`
 *
 * step 1 : extract from filename
 *
 * step 2 : search /r/n who is at the end of file name
 *
 * step 3 : split before /r/n
 *
 * step 4 : delete "" and filename=
 *
 */
void extractFileName(request &request)
{
    std::string filename;
    std::string tmp = "filename=\"";

    size_t len = request._body.find("filename");
    filename = request._body.substr(len);

    size_t space = filename.find("\r\n");
    filename = filename.substr(0, space);

    filename = filename.substr(tmp.size());
    filename = filename.substr(0, filename.size() - 1);

    // std::cout << "filename [" << filename << "]\n";
}