/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:13:02 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/12 15:14:06 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

#include <iostream>
#include <vector>
#include <map>
#include "ServerConfig.hpp"

class ConfigParser
{
	private:
		std::vector<ServerConfig> _servers;
		
		bool	hasOpeningBraceOnNextLine(std::ifstream &file);

	public: 

		std::vector<ServerConfig> getServers() const;
		void	parse(std::string path);
		void	parseServer(std::ifstream &file);

		void	parseRoot(std::string &args, ServerConfig &server);
		void	parseListen(std::string &args, ServerConfig &server);
		void	parseServerName(std::string &args, ServerConfig &server);
		void	parseErrorPage(std::string &args, ServerConfig &server);
		void	parseBodySize(std::string &args, ServerConfig &server);
};

#endif