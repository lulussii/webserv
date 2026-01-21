/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Multipart.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/21 15:04:09 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/21 15:34:03 by mathildelau      ###   ########.fr       */
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
        {
            end = tmp.find("--" + request.boundary + "--");
            if (end == std::string::npos)
                break;
        }
            

        std::string fullPart = tmp.substr(0, end - 1);

        Multipart m;
        m.fullPart = fullPart;
        extractName(m);
        extractFileName(m);
        request.party.push_back(m);
        tmp = tmp.substr(end);

        std::cout << "DEBUG\n";
        std::cout << "full part = [" << m.fullPart << "]\n";
        std::cout << "name = [" << m.name << "]\n";
        std::cout << "file name = [" << m.filename << "]\n";
        std::cout << "tmp = [" << tmp << "]\n";
    }
}

/**
 * @brief `find filename`
 *
 * step 1 : search filename= and from filename=
 *
 * step 2 : search /r/n who is at the end of file name
 *
 * step 3 : split before /r/n
 *
 * step 4 : delete "" and filename=
 *
 */
void extractFileName(Multipart &m)
{
    std::string filename;
    std::string tmp = "filename=\"";

    size_t len = m.fullPart.find("filename");
    filename = m.fullPart.substr(len);

    size_t space = filename.find("\r\n");
    filename = filename.substr(0, space);

    filename = filename.substr(tmp.size());
    filename = filename.substr(0, filename.size() - 1);

    m.filename = filename;
}

/**
 * @brief `find filename`
 *
 * step 1 : search name=" and copy from name="
 *
 * step 2 : search ; who is at the end of name
 *
 * step 3 : split before;
 *
 * step 4 : delete "" and name=
 *
 */
void extractName(Multipart &m)
{
    std::string name;
    std::string tmp = "name=\"";

    size_t len = m.fullPart.find("name");
    name = m.fullPart.substr(len);

    size_t space = name.find(";");
    name = name.substr(0, space);

    name = name.substr(tmp.size());
    name = name.substr(0, name.size() - 1);

    m.name = name;
}

