/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserLocation.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 15:23:21 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/04 15:24:26 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

void ConfigParser::parseMethods(std::string &args, LocationConfig &loc)
{
	if (loc._allowMethodsDefined)
		_throwError("Config Error: Duplicate methods directive");
	loc._allowMethodsDefined = true;

	std::stringstream ss(args);
	std::string value;
	bool foundSemicolon = false;

	while (ss >> value)
	{
		if (value[0] == '#')
			break;

		bool isLast = false;
		if (value == ";")
		{
			foundSemicolon = true;
			break;
		}
		else if (value[value.size() - 1] == ';')
		{
			value.erase(value.size() - 1);
			foundSemicolon = true;
			isLast = true;
		}

		if (value == "GET")
		{
			if (loc._allowGet)
				_throwError("Config Error: Duplicate method GET");
			loc._allowGet = true;
		}
		else if (value == "POST")
		{
			if (loc._allowPost)
				_throwError("Config Error: Duplicate method POST");
			loc._allowPost = true;
		}
		else if (value == "DELETE")
		{
			if (loc._allowDelete)
				_throwError("Config Error: Duplicate method DELETE");
			loc._allowDelete = true;
		}
		else
			_throwError("Config Error: Unknown method: " + value);

		if (isLast)
			break;
	}
	if (!foundSemicolon)
		_throwError("Syntax Error: Allow_methods must end with ';");
}

void ConfigParser::parseReturn(std::string &args, LocationConfig &loc)
{
	std::stringstream ss(args);
	std::string code;
	std::string url;

	ss >> code;
	if (code.empty())
		_throwError("Syntax Error: return directive needs a code");

	ss >> url;
	if (url.empty())
		_throwError("Syntax Error: return directive needs a URL");

	_checkAndStripSemicolon(url, ss);

	loc._returnCode = std::atoi(code.c_str());
	loc._returnPath = url;

	for (size_t i = 0; i < code.size(); i++)
	{
		if (!std::isdigit(code[i]))
			_throwError("Syntax Error: return directive '" + code + "' contains non-digit characters");
	}

	if (loc._returnCode < 300 || loc._returnCode > 399)
		_throwError("Config Error: return code must be a 3xx status");
}

void ConfigParser::parseUpload(std::string &args, LocationConfig &loc)
{
	std::stringstream ss(args);
	std::string value;
	ss >> value;

	if (value.empty())
		_throwError("Syntax Error: upload_store directive is empty");

	_checkAndStripSemicolon(value, ss);

	loc._uploadPath = value;
}

void	ConfigParser::parseCgi(std::string &args, LocationConfig &loc)
{
	std::string	path;
	std::string	extension;
	std::stringstream	ss(args);

	if (!(ss >> extension >> path))
		_throwError("Config Error: cgi_setup requires exactly 2 arguments (extension and path)");

	_checkAndStripSemicolon(path, ss);

	if (extension. empty() || extension[0] != '.')
		_throwError("Config Error: cgi extension must start with a dot");
	
	if (access(path.c_str(), X_OK) == -1)
		_throwError("Config Error: cgi binary not found or not executable");
	
	loc._cgiPaths[extension] = path;
}

void ConfigParser::parseLocation(std::ifstream &file, ServerConfig &server, std::string &path)
{
	LocationConfig loc;
	loc._path = path;
	std::string line;

	while (std::getline(file, line))
	{
		_lineNumber++;

		std::stringstream ss(line);
		std::string key;

		ss >> key;

		if (key.empty() || key[0] == '#')
			continue;
		if (key[0] == '}')
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
			_throwError("Unknown directive in location: " + key);
	}
	_throwError("Syntax Error: Location block not closed with '}");
}