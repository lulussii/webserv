/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:27:30 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/09 12:33:02 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include <fstream>
#include <string>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <climits>

std::vector<ServerConfig> ConfigParser::getConfigs() const
{
	return this->_servers;
}

void ConfigParser::_checkAndStripSemicolon(std::string &value, std::stringstream &ss)
{
	if (!value.empty() && value[value.size() - 1] == ';')
	{
		value.resize(value.size() - 1);
		return;
	}

	std::string nextToken;
	if (!(ss >> nextToken) || nextToken != ";")
		_throwError("Syntax Error: Missing or misplaced ';'");
}

bool ConfigParser::_isValidIP(const std::string &ip)
{
	int dots = 0;
	for (size_t i = 0; i < ip.size(); i++)
	{
		if (ip[i] == '.')
			dots++;
	}
	if (dots != 3)
		return (false);

	std::stringstream ss(ip);
	std::string str;
	int count = 0;

	while (std::getline(ss, str, '.'))
	{
		if (str.empty())
			return (false);

		for (size_t i = 0; i < str.size(); i++)
		{
			if (!std::isdigit(str[i]))
				return (false);
		}
		if (str.size() > 3)
			return (false);

		int num = std::atoi(str.c_str());
		if (num < 0 || num > 255)
			return (false);
		count++;
	}
	if (count != 4 || ip[ip.size() - 1] == '.')
		return (false);
	return (true);
}

int ConfigParser::_parsePort(const std::string &str)
{
	if (str.empty())
		_throwError("Syntax Error: Port is missing");

	for (size_t i = 0; i < str.size(); i++)
	{
		if (!std::isdigit(str[i]))
			_throwError("Syntax Error: Port '" + str + "' contains non-digit characters");
	}

	long port = std::atol(str.c_str());
	if (port < 1 || port > 65535)
		_throwError("Config Error: Port '" + str + "' is out of range [1-65535]");

	return (static_cast<int>(port));
}

void ConfigParser::parseListen(std::string &args, ServerConfig &server)
{
	std::stringstream ss(args);
	std::string value;

	ss >> value;

	if (value.empty())
		_throwError("Syntax Error: Listen directive is empty");
	_checkAndStripSemicolon(value, ss);

	if (value.empty())
		_throwError("Syntax Error: Listen value is missing before ';'");
	std::string host = "0.0.0.0";
	int port = 8080;

	size_t pos = value.find(':');
	if (pos == std::string::npos)
		port = _parsePort(value);
	else
	{
		host = value.substr(0, pos);
		std::string portStr = value.substr(pos + 1);

		if (!_isValidIP(host))
			_throwError("Config Errror: Invalid IP adress format '" + host + "'");

		port = _parsePort(portStr);
	}

	for (size_t i = 0; i < server._listen.size(); i++)
	{
		if (server._listen[i].ip == host && server._listen[i].port == port)
			_throwError("Config Error: Duplicate listen " + host + ":" + value);
	}

	server._listen.push_back(Listen(host, port));

	std::string extra;
	if (ss >> extra && extra[0] != '#')
		_throwError("Syntax Error: Too many arguments for listen");
}

void ConfigParser::parseServerName(std::string &args, ServerConfig &server)
{
	std::stringstream ss(args);
	std::string value;
	bool foundSemicolon = false;

	while (ss >> value)
	{
		if (value[0] == '#')
			break;

		if (value[value.size() - 1] == ';')
		{
			value.erase(value.size() - 1);
			if (!value.empty())
				server._serverNames.push_back(value.erase(value.size() - 1));
			foundSemicolon = true;
			break;
		}
		else
			server._serverNames.push_back(value);
	}
	if (foundSemicolon == false)
		_throwError("Syntax Error: server_name value must end with ';'");

	std::string extra;
	if (ss >> extra && extra[0] != '#')
		_throwError("Syntax Error: Too many arguments for server_names");
}

void ConfigParser::parseBodySize(std::string &args, ServerConfig &server)
{
	if (server._clientMaxBodySize != 1000000)
		_throwError("Config Error: Duplicate client_max_body_size directive");

	std::stringstream ss(args);
	std::string value;

	ss >> value;

	if (value.empty())
		_throwError("Syntax Error: Body size value must end with ';'");

	_checkAndStripSemicolon(value, ss);

	if (value.empty())
		_throwError("Syntax Error: Body size directive is empty");

	char unit = value[value.size() - 1];
	size_t multiplier = 1;

	if (unit == 'K' || unit == 'k')
	{
		multiplier = 1024;
		value.erase(value.size() - 1);
	}
	else if (unit == 'M' || unit == 'm')
	{
		multiplier = 1024 * 1024;
		value.erase(value.size() - 1);
	}
	else if (unit == 'G' || unit == 'g')
	{
		multiplier = 1024 * 1024 * 1024;
		value.erase(value.size() - 1);
	}
	else if (!std::isdigit(unit))
		_throwError("Syntax Error: Invalid unit (use k, m, g)");

	unsigned long result = 0;
	for (size_t i = 0; i < value.size(); i++)
	{
		if (!std::isdigit(value[i]))
			_throwError("Syntax Error: Invalid character in size: " + value);
		int digit = value[i] - '0';
		if (result > (ULONG_MAX - digit) / 10)
			_throwError("Config Error: client_max_body_size is too large (overflow)");

		result = result * 10 + digit;
	}
	if (multiplier > 1 && result > ULONG_MAX / multiplier)
		_throwError("Config Error: client_max_body_size is too large (overflow)");

	server._clientMaxBodySize = (multiplier * result);

	std::string extra;
	if (ss >> extra && extra[0] != '#')
		_throwError("Syntax Error: Too many arguments for client_max_body_size");
}

void ConfigParser::parseErrorPage(std::string &args, ServerConfig &server)
{
	std::stringstream ss(args);
	std::string value;
	std::vector<std::string> tmp;
	bool foundSemicolon = false;

	while (ss >> value)
	{
		if (value[0] == '#')
			break;
		if (value[value.size() - 1] == ';')
		{
			tmp.push_back(value.erase(value.size() - 1));
			foundSemicolon = true;
			break;
		}
		else
			tmp.push_back(value);
	}
	if (foundSemicolon == false)
		_throwError("Syntax Error: error_page value must end with ';'" + args);
	if (tmp.size() < 2)
		_throwError("Syntax Error: error_page needs at least one code and a path");

	std::string errorPath = tmp.back();
	tmp.pop_back();

	for (size_t i = 0; i < tmp.size(); i++)
	{
		int code = std::atoi(tmp[i].c_str());
		if (code < 300 || code > 599)
			_throwError("Config Error: Invalid HTTP error code");

		server._errorPages[code] = errorPath;
	}
}

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

		if (key == "root")
			parseRoot(args, loc);
		else if (key == "autoindex")
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

void ConfigParser::parseServer(std::ifstream &file)
{
	ServerConfig currentServer;
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
			this->_servers.push_back(currentServer);
			return;
		}

		std::string args;
		std::getline(ss, args);

		if (key == "root")
			parseRoot(args, currentServer);
		else if (key == "listen")
			parseListen(args, currentServer);
		else if (key == "client_max_body_size")
			parseBodySize(args, currentServer);
		else if (key == "autoindex")
			parseAutoIndex(args, currentServer);
		else if (key == "index")
			parseIndex(args, currentServer);
		else if (key == "server_name")
			parseServerName(args, currentServer);
		else if (key == "error_page")
			parseErrorPage(args, currentServer);
		else if (key == "location")
		{
			std::stringstream ssLoc(args);
			std::string path;
			ssLoc >> path;

			if (path.empty())
				_throwError("Syntax Error: Location path missing");

			if (path[path.size() - 1] == '{')
				path = path.substr(0, path.size() - 1);

			std::string brace;
			if (ssLoc >> brace)
			{
				if (brace != "{")
					_throwError("Error: Invalid token after location path: " + brace);
			}
			else
			{
				if (!hasOpeningBraceOnNextLine(file))
					_throwError("Error: Location block not opened with '{'");
			}
			parseLocation(file, currentServer, path);
		}
		else
			_throwError("Syntax Error: Unknown directive in server block: " + key);
	}
	_throwError("Syntax Error: Server block not closed. Missing '}'");
}

/**
 * 	@brief Helper: Advances file cursos to locate the opening brace '{'.
 * 	Skips empty lines and comments.
 *	Return true if '{' is found immediately, false otherwise.
 */
bool ConfigParser::hasOpeningBraceOnNextLine(std::ifstream &file)
{
	std::string line;
	while (std::getline(file, line))
	{
		_lineNumber++;
		if (line.empty())
			continue;

		std::stringstream ss(line);
		std::string token;

		ss >> token;

		if (token.empty())
			continue;
		if (token == "{")
			return (true);
		else
			return (false);
	}
	return (false);
}

/**
 * @brief Main parsing loop
 * 	1. File stream initialization
 * 	2. Global scope validation
 * 	3. Server block entry detection
 */
void ConfigParser::parse(std::string path)
{
	std::ifstream config(path.c_str());

	if (!config)
		throw std::runtime_error("Unable to open file");

	std::string line;
	while (std::getline(config, line))
	{
		_lineNumber++;
		if (line.empty())
			continue;

		std::stringstream ss(line);
		std::string key;

		ss >> key;

		if (key.empty() || key[0] == '#')
			continue;
		if (key == "server")
		{
			std::string next_token;

			if (ss >> next_token)
			{
				if (next_token != "{")
					_throwError("Error: Invalid token after server: " + next_token);
			}
			else
			{
				if (!hasOpeningBraceOnNextLine(config))
					_throwError("Error: Server block not opened with '{'");
			}
			parseServer(config);
			std::cout << "serveur trouve !" << std::endl;
		}
		else
		{
			_throwError("Content found outside server block: " + key);
		}
	}
}