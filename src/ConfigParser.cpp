/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:27:30 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/18 17:01:29 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>

void	ConfigParser::parseRoot(std::string &args, ServerConfig &server)
{
	std::stringstream	ss(args);
	std::string			value;

	ss >> value;

	if (value.empty())
		throw std::runtime_error("Syntax Error: Root directive is empty");
	if (value[value.size() - 1] != ';')
		throw std::runtime_error("Syntax Error: Root path must end with ';'");
	if (server._root != "")
		throw std::runtime_error("Config Error: Duplicte root directive");

	std::string extraArg;
	if (ss >> extraArg && extraArg[0] != '#')
		throw std::runtime_error("Syntax Error: Too many arguments for root directive");
	server._root = value.erase(value.size() - 1);
}

void	ConfigParser::parseListen(std::string &args, ServerConfig &server)
{
	if (server._port != 0)
			throw std::runtime_error("Config Error: Duplicate listen directive");
		
	std::stringstream	ss(args);
	std::string			value;

	ss >> value;

	if (value.empty())
		throw std::runtime_error("Syntax Error: Listen directive is empty");
	if (value[value.size() - 1] != ';')
		throw std::runtime_error("Syntax Error: Listen value must end with ';");
	std::string cleanValue = value.substr(0, value.size() - 1);

	size_t pos = cleanValue.find(':');
	if (pos == std::string::npos)
		server._port = std::atoi(cleanValue.c_str());
	else
	{
		std::string host = cleanValue.substr(0, pos);
		std::string port = cleanValue.substr(pos + 1);
		
		server._host = host;
		server._port = std::atoi(port.c_str());
	}
	
	std::string	extra;
	if (ss >> extra && extra[0] != '#')
		throw std::runtime_error("Syntax Error: Too many arguments for listen");
}

void	ConfigParser::parseServerName(std::string &args, ServerConfig &server)
{
	if (!server._serverNames.empty())
		throw std::runtime_error("Config Error: Duplicate server_name directive");
		
	std::stringstream	ss(args);
	std::string			value;
	bool				foundSemicolon = false;

	while (ss >> value)
	{
		if (value[0] == '#')
			break;
			
		if (value[value.size() - 1] == ';')
		{
			server._serverNames.push_back(value.erase(value.size() - 1));
			foundSemicolon = true;
			break;
		}
		else
			server._serverNames.push_back(value);
	}
	if (foundSemicolon == false)
		throw std::runtime_error("Syntax Error: server_name value must end with ';");
}

void	ConfigParser::parseErrorPage(std::string &args, ServerConfig &server)
{
	std::stringstream			ss(args);
	std::string					value;
	std::vector<std::string>	tmp;
	bool						foundSemicolon = false;
		
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
		throw std::runtime_error("Syntax Error: error_page value must end with ';");
	if (tmp.size() < 2)
		throw std::runtime_error("Syntax Error: error_page needs at least one code and a path");

	std::string errorPath = tmp.back();
	tmp.pop_back();

	for (size_t i = 0; i < tmp.size(); i++)
	{
		int code = std::atoi(tmp[i].c_str());
		if (code < 300 || code > 599)
			throw std::runtime_error("Config Error: Invalid HTTP error code");

		server._errorPages[code] = errorPath;
	}
}

void	ConfigParser::parseBodySize(std::string &args, ServerConfig &server)
{
	if (server._clientMaxBodySize != 1000000)
		throw std::runtime_error("Config Error: Duplicate client_max_body_size directive");

	std::stringstream	ss(args);
	std::string			value;

	ss >> value;

	if (value.empty())
		throw std::runtime_error("Syntax Error: Body size value must end with ';'");

	value.erase(value.size() - 1);
	
	if (value.empty())
		throw std::runtime_error("Syntax Error: Body size directive is empty");
	
	char unit = value[value.size() - 1];
	size_t	multiplier = 1;

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
		throw std::runtime_error("Syntax Error: Invalid unit (use k, m, g)");

	size_t valueInt = std::atoi(value.c_str());
	server._clientMaxBodySize = valueInt * multiplier;

	std::string extra;
	if (ss >> extra && extra[0] != '#')
		throw std::runtime_error("Syntax Error: Too many arguments for client_max_body_size");
}

void	ConfigParser::parseServer(std::ifstream &file)
{
	ServerConfig	currentServer;
	std::string		line;
	
	while (std::getline(file, line))
	{
		std::stringstream	ss(line);
		std::string			key;

		ss >> key;

		if (key.empty() || key[0] == '#')
			continue;
		
		std::string	args;
		std::getline(ss, args);

		if (key[0] == '}')
		{
			this->_servers.push_back(currentServer);
			return ;	
		}
		if (key == "root")
			parseRoot(args, currentServer);
		else if (key == "listen")
			parseListen(args, currentServer);
		else if (key == "server_name")
			parseServerName(args, currentServer);
		else if (key == "error_page")
			parseErrorPage(args, currentServer);
		else if (key == "client_max_body_size")
			parseBodySize(args, currentServer);
		else
			throw std::runtime_error("Syntax Error: Unknown directive in server block: " + key);
	}
	throw std::runtime_error("Syntax Error: Server block not closed. Missing '}'");
}
/**
 * 	@brief Helper: Advances file cursos to locate the opening brace '{'.
 * 	Skips empty lines and comments.
 *	Return true if '{' is found immediately, false otherwise.
 */
bool	ConfigParser::hasOpeningBraceOnNextLine(std::ifstream &file)
{
	std::string	line;
	while (std::getline(file, line))
	{
		if (line.empty())
			continue;
		
		std::stringstream	ss(line);
		std::string			token;

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
		if (line.empty())
			continue;
			
		std::stringstream 	ss(line);
		std::string			key;

		ss >> key;

		if (key.empty() || key[0] == '#') 
			continue;
		std::cout << key << std::endl;
		if (key == "server") {
			std::string next_token;

			if (ss >> next_token)
			{
				if ( next_token != "{")
					throw std::runtime_error("Error: Invalid token after server: " + next_token);
			}
			else
			{
				if (!hasOpeningBraceOnNextLine(config))
					throw std::runtime_error("Error: Server block not opened with '{'");
			}
			parseServer(config);
			std::cout << "serveur trouve !" << std::endl;
		}
		else {
			throw std::runtime_error("Content found outside server block: " + key);
		}
	}
}