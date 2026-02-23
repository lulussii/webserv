/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParserUtils.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/04 15:11:38 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/21 11:55:55 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

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

/**
 * 	@brief Helper: Advances file cursos to locate the opening brace '{'.
 * 	Skips empty lines and comments.
 *	Return true if '{' is found immediately, false otherwise.
 */
bool ConfigParser::_hasOpeningBraceOnNextLine(std::ifstream &file)
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

void	ConfigParser::_stripComments(std::string &line)
{
	size_t	hasPos = line.find('#');
	if (hasPos != std::string::npos)
		line = line.substr(0, hasPos);
}