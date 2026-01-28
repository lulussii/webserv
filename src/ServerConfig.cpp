/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 09:53:30 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/27 15:39:32 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include "ConfigParser.hpp"

ServerConfig::ServerConfig()
{
	this->_root = "";
	this->_clientMaxBodySize = 1000000;
	this->_autoIndex = -1;
}

std::vector<ServerConfig> ConfigParser::getServers() const
{
	return (this->_servers);
}
