/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 09:48:42 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/02 10:39:34 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "Request.hpp"
#include "Response.hpp"

class Client
{
	public: 
		int			fd;
		std::string	readBuffer;
		std::string	writeBuffer;
		request		req;
		response 	res;
		bool		isReadyToWrite;

		Client();
		Client(int client_fd);
		~Client();
};

#endif