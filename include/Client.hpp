/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 09:48:42 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/19 18:56:01 by mathildelau      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <ctime>
#include <sys/types.h>
#include <sys/socket.h>
#include "Cgi.hpp"
#include "Config.hpp"
#include "Delete.hpp"
#include "Error.hpp"
#include "Get.hpp"
#include "Init.hpp"
#include "Post.hpp"
#include "Request.hpp"
#include "Response.hpp"

/**
 * @class Client
 * @brief Représente une connexion active avec un navigateur.
 * * Son rôle est de :
 * 1. Stocker les données brutes qui arrivent (readBuffer)
 * 2. Transformer ces données en objets compréhensibles (res/req)
 * 3. Stocker la réponse finale à renvoyer (writeBuffer).
 */
class Client
{
	private:
		void 		_dispatchMethod(serverT &config, cgi &cgiInstance);
		serverT*	_selectServerConfig(std::vector<serverT> &allConfigs);

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
		long		bodyStartIndex;
		bool		headersReceived;
		bool		requestComplete;
		
		bool		isReadyToWrite;
		bool		isChunkedRequest;

		cgi	cgiClient;

		Client();
		Client(int fd, int port);

		int getServerPort() const;

		long	getContentLength(const std::string &buffer);
		void	parseHeaders(const std::string& rawHeaders, request &req);

		void	requestLine(request &request);
		void	handleRead(serverT &serverConfig, request &request, responseT &response, cgi &cgi);
		// void 	handleRead(serverT &serverConfig);
		void	handleWrite();
		void	processRequest(serverT &serverConfig, request &request, responseT &response, cgi &cgi);
		// void	processRequest(serverT &serverConfig);
		void	reset();
};

#endif