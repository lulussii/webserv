/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 09:53:53 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/12 12:16:53 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

#include <iostream>
#include <map>
#include <vector>

class ServerConfig
{
	private:
		int 						_port;
		std::string					_host;
		std::map<int, std::string>	_errorPages;
		size_t						_clientMaxBodySize;
		std::string					_root;
		std::vector<std::string>	_serverNames;

	public:
		ServerConfig();
		~ServerConfig();
		
		//SETTERS
		void	setPort(int port);
		void	setHost(std::string host);
		void	setErrorPages(std::map<int, std::string> errorPages);
		void	setClientMaxBodySize(size_t clientMaxBodySize);
};


#endif