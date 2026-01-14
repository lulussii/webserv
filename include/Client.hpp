/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 09:48:42 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/14 09:26:44 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <ctime>
#include "Request.hpp"
#include "Response.hpp"

class Client
{
	public: 
		int			fd;
		std::string	readBuffer;
		std::string	writeBuffer;
		std::string reponse;
		request		req;
		response 	res;
		bool		isReadyToWrite;
		long		contentLength;
		bool		headersReceived;
		time_t		lastTime;


		Client() : fd(-1), isReadyToWrite(false), contentLength(0), headersReceived(false), lastTime(time(NULL)) {}
		Client(int client_fd) : fd(client_fd), isReadyToWrite(false), contentLength(0), headersReceived(false), lastTime(time(NULL)) {}
};

#endif