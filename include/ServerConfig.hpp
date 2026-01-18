/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 09:53:53 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/18 16:59:21 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include <iostream>
#include <map>
#include <vector>

class ServerConfig
{
	friend class ConfigParser;
	
	private:
		int 						_port;
		std::string					_host;
		std::map<int, std::string>	_errorPages;
		size_t						_clientMaxBodySize;
		std::string					_root;
		std::vector<std::string>	_serverNames;

	public:
		ServerConfig();
};


#endif