/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/02 09:47:55 by lserodon          #+#    #+#             */
/*   Updated: 2026/02/22 09:42:36 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <map>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <poll.h>
#include <signal.h>
#include <map>
#include "Client.hpp"
#include "Server.hpp"
#include "ConfigParser.hpp"

int main(int argc, char **argv) 
{
	std::string 	config_file_path;
	ConfigParser	parser;

	signal(SIGINT, handle_sigint);

	if (argc == 1)
	{
		std::cout << "No argument provided. Using default config" << std::endl;
		config_file_path = "config/default.conf";
	}
	else if (argc == 2)
		config_file_path = argv[1];
	else if (argc > 2)
	{
		std::cerr << "Error: Too many arguments" << std::endl;
		return (1);
	}
	
	try 
	{
		parser.parse(config_file_path);
		std::vector<ServerConfig> configs = parser.getConfigs();
		
		Server 			webServ(configs);
		webServ.setup();
		webServ.run();
	} catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}
