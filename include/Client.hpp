/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 09:48:42 by lserodon          #+#    #+#             */
/*   Updated: 2026/01/15 14:00:28 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <ctime>
#include "Request.hpp"
#include "Response.hpp"


/**
 * @class Client
 * @brief Représente une connexion active avec un navigateur.
 * * Son rôle est de :
 * 1. Stocker les données brutes qui qarrivent (readBuffer)
 * 2. Transformer ces données en objets compréhensibles (res/req)
 * 3. Stocker la réponse finale à renvoyer (writeBuffer).
 */
class Client
{
	public:
		int			fd;
		time_t		lastTime;

		std::string	readBuffer;
		std::string	writeBuffer;
		
		request		req;
		response 	res;
		
		long		contentLength;
		bool		headersReceived;
		bool		requestComplete;
		
		bool		isReadyToWrite;

		Client();
		Client(int client_fd);

		long	getContentLength(const std::string &buffer);

		void	handleRead(serverT &serverconfig);
		void	handleWrite();
		void	processRequest(serverT &serverConfig);
		void	reset();
};

#endif