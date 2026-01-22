/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:13:02 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/21 15:38:01 by lserodon         ###   ########.fr       */
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

	public: 

		ConfigParser(): _lineNumber(0) {};
		~ConfigParser() {};

		std::vector<ServerConfig> getServers() const;
		
		void	parse(std::string path);
		void	parseServer(std::ifstream &file);
		void	parseLocation(std::ifstream &file, ServerConfig &server,std::string &path);

		void	parseListen(std::string &args, ServerConfig &server);
		void	parseServerName(std::string &args, ServerConfig &server);
		void	parseErrorPage(std::string &args, ServerConfig &server);
		void	parseMethods(std::string &args, LocationConfig &loc);
		void	parseReturn(std::string &args, LocationConfig &loc);
		void	parseUpload(std::string &args, LocationConfig &loc);


		template <typename T>
		void	parseRoot(std::string &args, T &config)
		{
			std::stringstream	ss(args);
			std::string			value;

			ss >> value;

			if (value.empty())
				 _throwError("Syntax Error: Root directive is empty");
			if (value[value.size() - 1] != ';')
				 _throwError("Syntax Error: Root path must end with ';'");
			if (config._root != "")
				 _throwError("Config Error: Duplicate root directive");

			std::string extraArg;
			if (ss >> extraArg && extraArg[0] != '#')
				 _throwError("Syntax Error: Too many arguments for root directive");
			config._root = value.erase(value.size() - 1);
		}

		template <typename T>
		void	parseBodySize(std::string &args, T &config)
		{
			if (config._clientMaxBodySize != 1000000)
				 _throwError("Config Error: Duplicate client_max_body_size directive");

			std::stringstream	ss(args);
			std::string			value;

			ss >> value;

			if (value.empty())
				 _throwError("Syntax Error: Body size value must end with ';'");

			value.erase(value.size() - 1);
			
			if (value.empty())
				 _throwError("Syntax Error: Body size directive is empty");
			
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
				 _throwError("Syntax Error: Invalid unit (use k, m, g)");

			size_t valueInt = std::atoi(value.c_str());
			config._clientMaxBodySize = valueInt * multiplier;

			std::string extra;
			if (ss >> extra && extra[0] != '#')
				 _throwError("Syntax Error: Too many arguments for client_max_body_size");
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
			if (value[value.size() - 1] != ';')
				 _throwError("Syntax Error: Autoindex directive must end with ';'");
			value.erase(value.size() - 1);
			
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
					
				if (value[value.size() - 1] == ';')
				{
					config._index.push_back(value.erase(value.size() - 1));
					foundSemicolon = true;
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