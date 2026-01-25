/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:27:30 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/24 16:27:50 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "LocationConfig.hpp"
#include "ServerConfig.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>

std::vector<ServerConfig> ConfigParser::getConfigs() const
{
    return this->_servers;
}

void	ConfigParser::parseListen(std::string &args, ServerConfig &server)
{
	if (server._port != 0)
			 _throwError("Config Error: Duplicate listen directive");
		
	std::stringstream	ss(args);
	std::string			value;

	ss >> value;

	if (value.empty())
		 _throwError("Syntax Error: Listen directive is empty");
	if (value[value.size() - 1] != ';')
		 _throwError("Syntax Error: Listen value must end with ';");
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
		 _throwError("Syntax Error: Too many arguments for listen");
}

void	ConfigParser::parseServerName(std::string &args, ServerConfig &server)
{
	if (!server._serverNames.empty())
		 _throwError("Config Error: Duplicate server_name directive");
		
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
		 _throwError("Syntax Error: server_name value must end with ';");
		
	std::string extra;
	if (ss >> extra && extra[0] != '#')
		 _throwError("Syntax Error: Too many arguments for server_names");
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

void	ConfigParser::parseMethods(std::string &args, LocationConfig &loc)
{
	if (loc._allowMethodsDefined)
		 _throwError("Config Error: Duplicate methods directive");
	loc._allowMethodsDefined = true;
	
	std::stringstream ss(args);
	std::string			value;
	bool				foundSemicolon = false;
	
	while (ss >> value)
	{
		if (value[0] == '#')
			break;
		
		bool isLast = false;
		if (value[value.size() - 1] == ';')
		{
			value.erase(value.size() - 1);
			isLast = true;
			foundSemicolon = true;
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

void	ConfigParser::parseReturn(std::string &args, LocationConfig &loc)
{
	std::stringstream	ss(args);
	std::string			code;
	std::string			url;

	ss >> code;
	if (code.empty())
		 _throwError("Syntax Error: return directive needs a code");

	ss >> url;
	if (url.empty())
		 _throwError("Syntax Error: return directive needs a URL");

	if (url[url.size() - 1] != ';')
		 _throwError("Syntax Error: return directive must end with ';'");
	url.erase(url.size() - 1); 

	loc._returnCode = std::atoi(code.c_str());
	loc._returnPath = url;

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
    
	if (value[value.size() - 1] != ';')
		 _throwError("Syntax Error: upload_store must end with ';'");
	value.erase(value.size() - 1);

	loc._uploadPath = value;
}

void	ConfigParser::parseLocation(std::ifstream &file, ServerConfig &server, std::string &path)
{
	LocationConfig loc;
	loc._path = path;
	std::string	line;

	while(std::getline(file, line))
	{
		_lineNumber++;
		
		std::stringstream	ss(line);
		std::string			key;

		ss >> key;

		if (key.empty() || key[0] == '#')
			continue;
		if (key[0] == '}')
		{
			server._locations.push_back(loc);
			return ;
		}

		std::string args;
		std::getline(ss, args);

		if (key == "root")
			parseRoot(args, loc);
		else if (key == "client_max_body_size")
			parseBodySize(args, loc);
		else if (key == "autoindex")
			parseAutoIndex(args, loc);
		else if (key == "index")
			parseIndex(args, loc);
		else if (key == "allow_methods")
			parseMethods(args, loc);
		else if (key == "return")
			parseReturn(args, loc);
		else if (key == "upload_store")
			parseUpload(args, loc);
		else
			 _throwError("Unknown directive in location: " + key);
	}
	 _throwError("Syntax Error: Location block not closed with '}");
}

void	ConfigParser::parseServer(std::ifstream &file)
{
	ServerConfig	currentServer;
	std::string		line;
	
	while (std::getline(file, line))
	{
		_lineNumber++;
		
		std::stringstream	ss(line);
		std::string			key;

		ss >> key;

		if (key.empty() || key[0] == '#')
			continue;
		if (key[0] == '}')
		{
			this->_servers.push_back(currentServer);
			return ;	
		}
		
		std::string	args;
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
			
			if (path[path.size() -1] == '{')
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
bool	ConfigParser::hasOpeningBraceOnNextLine(std::ifstream &file)
{
	std::string	line;
	while (std::getline(file, line))
	{
		_lineNumber++;
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
		std::runtime_error("Unable to open file");
	
	std::string line;
	while (std::getline(config, line))
	{
		_lineNumber++;
		if (line.empty())
			continue;
			
		std::stringstream 	ss(line);
		std::string			key;

		ss >> key;

		if (key.empty() || key[0] == '#') 
			continue;
		if (key == "server") 
		{
			std::string next_token;

			if (ss >> next_token)
			{
				if ( next_token != "{")
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
		else {
			 _throwError("Content found outside server block: " + key);
		}
	}
}