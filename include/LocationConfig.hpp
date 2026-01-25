/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/20 14:15:25 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/24 15:58:38 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

#include <string>
#include <vector>
#include <cstddef>

class LocationConfig
{
	friend class ConfigParser;
	friend class Server;

	private:
		std::string					_path;
		std::string					_root;
		std::vector<std::string>	_index;
		int							_autoIndex;
		size_t						_clientMaxBodySize;

		bool		_allowGet;
		bool		_allowPost;
		bool		_allowDelete;
		bool		_allowMethodsDefined;

		int			_returnCode;
		std::string	_returnPath;
		std::string	_uploadPath;

	public:
		LocationConfig();
};

#endif