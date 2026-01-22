/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/20 14:32:18 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/20 14:35:25 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

LocationConfig::LocationConfig()
{
	this->_path = "";
	this->_root = "";
	this->_autoIndex = -1;
	this->_clientMaxBodySize = 1000000;
	this->_allowDelete = false;
	this->_allowGet = false;
	this->_allowPost = false;
	this->_allowMethodsDefined = false;
}