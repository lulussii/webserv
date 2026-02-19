/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:27:30 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/18 13:59:05 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

void ConfigParser::_validateBlockStart(std::stringstream &ss, std::ifstream &file)
{
	std::string	token;
	if (ss >> token)
	{
		if (token != "{")
			_throwError("Syntax Error: Expected '{', found: " + token);
	}
	else
	{
		if (!_hasOpeningBraceOnNextLine(file))
			_throwError("Syntax Error: Block not opened with '{'");
	}
}

void ConfigParser::_finalizeServer(ServerConfig &server)
{
	if (server._listen.empty())
	{
		std::cout << "[WARNING] Server block ended without 'listen'. Defaulting to 8080." << std::endl;
		Listen defaultListen("0.0.0.0", 8080);
		server._listen.push_back(defaultListen);
	}
	this->_servers.push_back(server);
}

void ConfigParser::_handleLocationBlock(std::string &args, std::ifstream &file, ServerConfig &server)
{
	std::stringstream ssLoc(args);
	std::string path;
	ssLoc >> path;

	if (path.empty())
		_throwError("Syntax Error: Location path missing");

	if (path[path.size() - 1] == '{')
		path = path.substr(0, path.size() - 1);
	else
		_validateBlockStart(ssLoc, file);
	parseLocation(file, server, path);
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

		if (key == "}") {
			_finalizeServer(currentServer);
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
		else if (key == "error_page")
			parseErrorPage(args, currentServer);
		else if (key == "location")
			_handleLocationBlock(args, file, currentServer);
		else
			_throwError("Syntax Error: Unknown directive: " + key);
	}
	_throwError("Syntax Error: Server block not closed. Missing '}'");
}

void ConfigParser::parse(std::string path)
{
	std::ifstream config(path.c_str());
	if (!config.is_open())
		throw std::runtime_error("Unable to open file: " + path);

	std::string line;
	while (std::getline(config, line))
	{
		_lineNumber++;
		if (line.empty()) continue;

		std::stringstream ss(line);
		std::string key;
		ss >> key;

		if (key.empty() || key[0] == '#') continue;

		if (key == "server")
		{
			_validateBlockStart(ss, config);
			parseServer(config);
		}
		else
		{
			_throwError("Syntax Error: Content found outside server block: " + key);
		}
	}
}