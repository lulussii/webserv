/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserServer.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 15:27:26 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/04 15:28:02 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

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