/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:13:02 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/28 14:47:39 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <cstdlib>
#include <fstream>
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"

class ConfigParser
{
	private:
		std::vector<ServerConfig>	_servers;
		size_t						_lineNumber;
		
		void	_throwError(const std::string &msg)
		{
			std::stringstream ss;
			ss << "Error on line " << _lineNumber << ": " << msg;
			throw std::runtime_error(ss.str());
		}
		
		bool	hasOpeningBraceOnNextLine(std::ifstream &file);
		void	_checkAndStripSemicolon(std::string &value, std::stringstream &ss);
		int		_parsePort(const std::string &str);
		bool	_isValidIP(const std::string &ip);

	public: 

		ConfigParser(): _lineNumber(0) {};
		~ConfigParser() {};

		std::vector<ServerConfig> getServers() const;
		std::vector<ServerConfig> getConfigs() const;
		
		void	parse(std::string path);
		void	parseServer(std::ifstream &file);
		void	parseLocation(std::ifstream &file, ServerConfig &server,std::string &path);

		void	parseListen(std::string &args, ServerConfig &server);
		void	parseServerName(std::string &args, ServerConfig &server);
		void	parseErrorPage(std::string &args, ServerConfig &server);
		void	parseMethods(std::string &args, LocationConfig &loc);
		void	parseReturn(std::string &args, LocationConfig &loc);
		void	parseUpload(std::string &args, LocationConfig &loc);
		void	parseBodySize(std::string &args, ServerConfig &server);

		template <typename T>
		void	parseRoot(std::string &args, T &config)
		{

			if (config._root != "")
				_throwError("Config Error: Duplicate root directive");

			std::stringstream	ss(args);
			std::string			value;

			ss >> value;

			if (value.empty())
				 _throwError("Syntax Error: Root directive is empty");
			_checkAndStripSemicolon(value, ss);
			if (value[value.size() - 1] != '/')
				value += "/";
			if (config._root != "")
				 _throwError("Config Error: Duplicate root directive");

			std::string extraArg;
			if (ss >> extraArg && extraArg[0] != '#')
				 _throwError("Syntax Error: Too many arguments for root directive");
			config._root = value;
		}

		template <typename T>
		void	parseAutoIndex(std::string &args, T &config)
		{
			std::stringstream	ss(args);
			std::string			value;

			ss >> value;

			if (config._autoIndex != -1)
				 _throwError("Config Error: Duplicate autoindex directive");

			if (value.empty())
				 _throwError("Syntax Error: Autoindex directive is empty");
		
			_checkAndStripSemicolon(value, ss);
			
			if (value == "on")
				config._autoIndex = 1;
			else if (value == "off")
				config._autoIndex = 0;
			else
				 _throwError("Syntax Error: Autoindex must be \"on\" or \"off\"");
			
			std::string extra;
			if (ss >> extra && extra[0] != '#')
				 _throwError("Syntax Error: Too many arguments for autoindex");
		}

		template <typename T>
		void	parseIndex(std::string &args, T &config)
		{
			if (!config._index.empty())
				 _throwError("Config Error: Duplicate index directive");
				
			std::stringstream	ss(args);
			std::string			value;
			bool				foundSemicolon = false;

			while (ss >> value)
			{
				if (value[0] == '#')
					break;
					
				if (value == ";")
				{
					foundSemicolon = true;
					break;
				}
				if (value[value.size() - 1] == ';')
				{
					value.erase(value.size() - 1);
					foundSemicolon = true;
					
					if (!value.empty())
						config._index.push_back(value);
					break;
				}
				else
					config._index.push_back(value);
			}
			if (foundSemicolon == false)
				 _throwError("Syntax Error: index value must end with ';");

			std::string extra;
			if (ss >> extra && extra[0] != '#')
				 _throwError("Syntax Error: Too many arguments for index");
		}
};

#endif