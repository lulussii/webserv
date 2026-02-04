/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:27:30 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/04 15:27:56 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

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