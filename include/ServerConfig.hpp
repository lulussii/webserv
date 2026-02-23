/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 09:53:53 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/21 11:36:02 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include <iostream>
#include <map>
#include <vector>
#include "LocationConfig.hpp"

struct Listen
{
	std::string ip;
	int			port;

	Listen(std::string host, int port) : ip(host), port(port) {}
};

class ServerConfig
{
	friend class ConfigParser;
	friend class Server;
	
	private:
		std::vector<Listen>			_listen;
		int							_autoIndex;
		std::map<int, std::string>	_errorPages;
		size_t						_clientMaxBodySize;
		bool						_clientMaxBodySizeDefined;
		std::string					_root;
		std::vector<std::string>	_index;
		std::vector<LocationConfig>	_locations;

	public:
		ServerConfig();
};

#endif