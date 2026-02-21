/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserLocation.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 15:23:21 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/21 11:55:36 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

void ConfigParser::parseMethods(std::string &args, LocationConfig &loc)
{
	if (loc._allowMethodsDefined)
		_throwError("Config Error: Duplicate methods directive");
	loc._allowMethodsDefined = true;

	std::stringstream ss(args);
	std::string method;
	bool end = false;

	while (ss >> method)
	{
		if (method[method.size() - 1] == ';')
		{
			method.erase(method.size() - 1);
			end = true;
		}
		if (method.empty())
		{
			if (end)
				break;
			continue;
		}
		if (method == "GET" && !loc._allowGet) 
			loc._allowGet = true;
		else if (method == "POST" && !loc._allowPost) 
			loc._allowPost = true;
		else if (method == "DELETE" && !loc._allowDelete) 
			loc._allowDelete = true;
		else
			_throwError("Config Error: Invalid or duplicate method: " + method);

		if (end) 
			break;
	}
	if (!end)
		_throwError("Syntax Error: Missing semicolon at end of methods");

	_checkExtraArgs(ss);
}

void ConfigParser::parseReturn(std::string &args, LocationConfig &loc)
{
	std::stringstream ss(args);
	int code;
	std::string url;

	if (!(ss >> code >> url))
		_throwError("Syntax Error: Invalid return directive");

	if (url[url.size() - 1] != ';')
		_throwError("Syntax Error: Missing semicolon");
	url.erase(url.size() - 1);

	if (code < 300 || code > 399)
		_throwError("Config Error: return code must be 3xx");

	loc._returnCode = code;
	loc._returnPath = url;
}

void ConfigParser::parseUpload(std::string &args, LocationConfig &loc)
{
	std::stringstream ss(args);
	std::string path;

	if (!(ss >> path))
		_throwError("Syntax Error: Upload path missing");

	if (path[path.size() - 1] != ';')
		_throwError("Syntax Error: Missing semicolon");
	path.erase(path.size() - 1);

	loc._uploadPath = path;
}

void ConfigParser::parseCgi(std::string &args, LocationConfig &loc)
{
	std::stringstream ss(args);
	std::string ext, path;

	if (!(ss >> ext >> path))
		_throwError("Config Error: cgi_setup requires extension and path");

	if (path[path.size() - 1] != ';')
		_throwError("Syntax Error: Missing semicolon");
	path.erase(path.size() - 1);

	if (ext[0] != '.')
		_throwError("Config Error: Extension must start with '.'");
	
	if (access(path.c_str(), X_OK) == -1)
		_throwError("Config Error: CGI binary not found or not executable");

	loc._cgiExtension = ext;
	loc._cgiBinary = path;
}

void ConfigParser::parseLocation(std::ifstream &file, ServerConfig &server, std::string &path)
{
	LocationConfig loc;
	loc._path = path;
	std::string line;

	while (std::getline(file, line))
	{
		_lineNumber++;

		_stripComments(line);
		std::stringstream ss(line);
		std::string key;
		ss >> key;

		if (key.empty()) 
			continue;

		if (key == "}")
		{
			server._locations.push_back(loc);
			return;
		}

		std::string args;
		std::getline(ss, args);

		if (key == "autoindex")
			parseAutoIndex(args, loc);
		else if (key == "index")
			parseIndex(args, loc);
		else if (key == "methods")
			parseMethods(args, loc);
		else if (key == "return")
			parseReturn(args, loc);
		else if (key == "upload_dir")
			parseUpload(args, loc);
		else if (key == "cgi_setup")
			parseCgi(args, loc);
		else 
			_throwError("Syntax Error: Unknown directive in location: " + key);
	}
	_throwError("Syntax Error: Location block not closed with '}'");
}