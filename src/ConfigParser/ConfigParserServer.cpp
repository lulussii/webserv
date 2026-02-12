/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserServer.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 15:27:26 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/12 14:46:31 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include <climits>

void ConfigParser::_checkExtraArgs(std::stringstream &ss)
{
	std::string extra;
	if (ss >> extra && extra[0] != '#')
		_throwError("Syntax Error: Too many arguments");
}

unsigned long ConfigParser::_parseBytes(std::string str)
{
	if (str.empty())
		_throwError("Syntax Error: Size value empty");

	char unit = str[str.size() - 1];
	unsigned long multiplier = 1;

	if (!std::isdigit(unit))
	{
		if (unit == 'K' || unit == 'k') multiplier = 1024;
		else if (unit == 'M' || unit == 'm') multiplier = 1024 * 1024;
		else if (unit == 'G' || unit == 'g') multiplier = 1024 * 1024 * 1024;
		else _throwError("Syntax Error: Invalid unit (use k, m, g)");
		str.erase(str.size() - 1);
	}

	unsigned long result = 0;
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!std::isdigit(str[i]))
			_throwError("Syntax Error: Invalid character in size: " + str);
		
		unsigned long digit = str[i] - '0';
		if (result > (ULONG_MAX - digit) / 10)
			_throwError("Config Error: Size too large (overflow)");
		result = result * 10 + digit;
	}

	if (multiplier > 1 && result > ULONG_MAX / multiplier)
		_throwError("Config Error: Size too large (overflow)");

	return result * multiplier;
}

int ConfigParser::_parsePort(const std::string &str)
{
	if (str.empty()) _throwError("Syntax Error: Port is missing");

	for (size_t i = 0; i < str.size(); i++)
	{
		if (!std::isdigit(str[i]))
			_throwError("Syntax Error: Port contains non-digits");
	}
	
	long port = std::atol(str.c_str());
	if (port < 1 || port > 65535)
		_throwError("Config Error: Port out of range [1-65535]");
	
	return static_cast<int>(port);
}

void ConfigParser::parseListen(std::string &args, ServerConfig &server)
{
	std::stringstream ss(args);
	std::string value;

	if (!(ss >> value))
		_throwError("Syntax Error: Listen directive empty");

	_checkAndStripSemicolon(value, ss);
	_checkExtraArgs(ss);

	std::string host = "0.0.0.0";
	int port = 8080;

	size_t pos = value.find(':');
	if (pos == std::string::npos)
	{
		port = _parsePort(value);
	}
	else
	{
		host = value.substr(0, pos);
		if (!_isValidIP(host))
			_throwError("Config Error: Invalid IP address: " + host);
		port = _parsePort(value.substr(pos + 1));
	}

	for (size_t i = 0; i < server._listen.size(); i++)
	{
		if (server._listen[i].ip == host && server._listen[i].port == port)
			_throwError("Config Error: Duplicate listen " + host + ":" + value);
	}

	server._listen.push_back(Listen(host, port));
}

void ConfigParser::parseServerName(std::string &args, ServerConfig &server)
{
	std::stringstream ss(args);
	std::string name;
	bool foundSemicolon = false;

	while (ss >> name)
	{
		if (name[0] == '#') break;
		
		if (name[name.size() - 1] == ';')
		{
			name.erase(name.size() - 1);
			foundSemicolon = true;
			if (!name.empty()) server._serverNames.push_back(name);
			break; 
		}
		server._serverNames.push_back(name);
	}

	if (!foundSemicolon)
		_throwError("Syntax Error: server_name must end with ';'");
	
	_checkExtraArgs(ss);
}

void ConfigParser::parseBodySize(std::string &args, ServerConfig &server)
{
	if (server._clientMaxBodySize != 1000000) 
		_throwError("Config Error: Duplicate client_max_body_size");

	std::stringstream ss(args);
	std::string value;

	if (!(ss >> value))
		_throwError("Syntax Error: Body size empty");

	_checkAndStripSemicolon(value, ss);
	_checkExtraArgs(ss);

	server._clientMaxBodySize = _parseBytes(value);
}

void ConfigParser::parseErrorPage(std::string &args, ServerConfig &server)
{
	std::stringstream ss(args);
	std::string token;
	std::vector<std::string> tokens;
	bool foundSemicolon = false;

	while (ss >> token)
	{
		if (token[0] == '#') break;
		if (token[token.size() - 1] == ';')
		{
			token.erase(token.size() - 1);
			tokens.push_back(token);
			foundSemicolon = true;
			break;
		}
		tokens.push_back(token);
	}

	if (!foundSemicolon)
		_throwError("Syntax Error: error_page must end with ';'");
	if (tokens.size() < 2)
		_throwError("Syntax Error: error_page requires at least one code and a path");
	
	_checkExtraArgs(ss);

	std::string path = tokens.back();
	tokens.pop_back();

	for (size_t i = 0; i < tokens.size(); i++)
	{
		int code = std::atoi(tokens[i].c_str());
		if (code < 300 || code > 599)
			_throwError("Config Error: Invalid HTTP error code: " + tokens[i]);
		
		server._errorPages[code] = path;
	}
}