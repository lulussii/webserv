/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 09:53:53 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/24 15:41:54 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include <iostream>
#include <map>
#include <vector>
#include "LocationConfig.hpp"

class ServerConfig
{
	friend class ConfigParser;
	friend class Server;
	
	private:
		int 						_port; 
		int							_autoIndex;
		std::string					_host; 
		std::map<int, std::string>	_errorPages;
		size_t						_clientMaxBodySize;
		std::string					_root;
		std::vector<std::string>	_serverNames;
		std::vector<std::string>	_index;
		std::vector<LocationConfig>	_locations;

	public:
		ServerConfig();
};


#endif