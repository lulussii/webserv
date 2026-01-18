/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 09:53:30 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/18 16:58:22 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"
#include "ConfigParser.hpp"

ServerConfig::ServerConfig()
{
	this->_port = 0;
	this->_host = "127.0.0.1";
	this->_root = "";
	this->_clientMaxBodySize = 1000000;
}

std::vector<ServerConfig> ConfigParser::getServers() const
{
	return (this->_servers);
}
