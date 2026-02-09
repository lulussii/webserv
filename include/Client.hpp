/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 09:48:42 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/09 10:24:54 by mlaussel         ###   ########.fr       */
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
		int			serverPort;
		time_t		lastTime;

		std::string	readBuffer;
		std::string	writeBuffer;
		std::string response;
		
		request				req;
		responseT	res;
		
		long		contentLength;
		bool		headersReceived;
		bool		requestComplete;
		
		bool		isReadyToWrite;

		Client();
		Client(int fd, int port);

		int getServerPort() const;

		long	getContentLength(const std::string &buffer);

		void	handleRead(serverT &serverconfig);
		void	handleWrite();
		void	processRequest(serverT &serverConfig);
		void	reset();
};

#endif