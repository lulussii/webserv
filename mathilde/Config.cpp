/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/04 15:47:48 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/05 10:42:25 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Request.hpp"
#include <fcntl.h>  //open
#include <unistd.h> //read
#include <cstdlib>  // atoi don't know if I can use it

int openConf()
{
    int fd;
    fd = open("./conf", O_RDONLY);
    if (fd < 0)
    {
        std::cout << "Error : cannot open file\n";
        return (-1);
    }
    return (fd);
}

int readConf(int fd, std::string &conf)
{
    ssize_t len = 1;
    while (len > 0)
    {
        char buffer[1024];
        len = read(fd, buffer, sizeof(buffer));
        if (len < 0)
        {
            std::cout << "Error : can't read file\n";
            return (1);
        }
        conf.append(buffer, len);
    }
    return (0);
}

void blocConfig(std::string conf, utilsConfigT &utils)
{
    size_t serverEnd = conf.find("location");
    if (serverEnd == std::string::npos)
    {
        std::cout << "Error: no locations block found\n";
        return;
    }
    utils.location = conf.substr(serverEnd);
    utils.server = conf.substr(0, serverEnd);
}

void serverPars(utilsConfigT &utils, serverT &serverConfig)
{
    std::string pars = utils.server;
    
    //step 1 : listen
    size_t start = pars.find("listen");
    size_t end = pars.find(";");
    
    std::string listen;
    listen = pars.substr(start, end - start);
    std::string tmp("listen");

    listen = listen.substr(tmp.size() + 1);
    
    serverConfig.listen = atoi(listen.c_str()); // c_str() convert std::string -> const char*

    pars = pars.substr(end + 1, pars.size() - end);
    
    //step 2 : root
    start = pars.find("root");
    end = pars.find(";");
    
    std::string root;
    root = pars.substr(start, end - start);
    std::string tmp2("root");

    root = root.substr(tmp2.size() + 1);
    
    serverConfig.root = root;

    pars = pars.substr(end + 1, pars.size() - end);
    
    //step 3 : error
    while (pars.find("error_page") != std::string::npos)
    {
        start = pars.find("error_page");
        end = pars.find(";");
        
        std::string errorPage;
        errorPage = pars.substr(start, end - start);
        std::string tmp3("error_page");
        
        errorPage = errorPage.substr(tmp3.size() + 1);

        end = errorPage.find("/");
        std::string number = errorPage.substr(start, end - start);
        errorPage = errorPage.substr(end);

        serverConfig.errorPage[atoi(number.c_str())] = errorPage;
        std::cout << serverConfig.errorPage[atoi(number.c_str())];
        
        pars = pars.substr(end + 1, pars.size() - end);
        std::cout << pars << std::endl;
        break;
    }

    //step 4 : client_max_body_size
    start = pars.find("client_max_body_size");
    end = pars.find(";");

    std::string client;
    client = pars.substr(start, end - start);
    std::string tmp4("client_max_body_size");
    
    client = client.substr(tmp4.size() + 1);
    
    serverConfig.clientMaxBodySize = atoi(client.c_str());
    
}

int configMain(serverT &serverConfig, locationsT &locationsConfig, utilsConfigT &utils)
{
    // step 1 : open conf file
    int fd = openConf();
    if (fd == -1)
    {
        close(fd);
        return (1);
    }

    // step 2 : read conf file
    std::string conf;
    if (readConf(fd, conf) == 1)
    {
        close(fd);
        return (1);
    }

    // step 3 : trim to ignore space tabs ; etc
    //std::string confFinal = trim(conf);

    // step 4 : find bloc
    blocConfig(conf, utils);

    // step 5 : pars server
    serverPars(utils, serverConfig);

    // step 6 : pars locations
    (void)locationsConfig;

    close(fd);
    return (0);
}