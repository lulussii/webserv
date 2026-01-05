/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/04 15:47:48 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/05 13:45:54 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Request.hpp"
#include <fcntl.h>  //open
#include <unistd.h> //read
#include <cstdlib> // atoi don't know if I can use it

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

std::string serverPars(utilsConfigT &utils, std::string string)
{
    size_t start = utils.pars.find(string);
    std::string tmp = utils.pars.substr(start);
    size_t end = tmp.find(";");
    
    utils.newS = tmp.substr(0, end);
    
    utils.newS = utils.newS.substr(string.size() + 1);

    return(utils.newS);
}

void errorPagePars(utilsConfigT &utils, serverT &serverConfig, std::string string)
{
    while (utils.pars.find(string) != std::string::npos)
    {
        size_t start = utils.pars.find(string);
        size_t end = utils.pars.find(";");
        
        std::string errorPage = utils.pars.substr(start, end - start);
        
        errorPage = errorPage.substr(string.size() + 1);

        size_t space = errorPage.find(" ");
        std::string number = errorPage.substr(0, space);
        errorPage = errorPage.substr(space + 1);

        serverConfig.errorPage[atoi(number.c_str())] = errorPage;
        
        utils.pars = utils.pars.substr(end + 1, utils.pars.size() - end);
    }
}

void locationsPars(utilsConfigT &utils, serverT &serverConfig)
{
    (void)serverConfig;
    size_t start = utils.pars.find("location");
    size_t end = utils.pars.find("{");
    
    std::string location("location");
    std::string path = utils.pars.substr(start, end - start - 1);
    path = path.substr(location.size() + 1);

    utils.pars = utils.pars.substr(end + 1);

    serverConfig.locations[path].path = path;

    // std::cout << utils.pars;

    // start = utils.pars
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
    utils.pars = utils.server;
    serverConfig.listen = atoi(serverPars(utils, "listen").c_str());
    serverConfig.root = serverPars(utils, "root");
    errorPagePars(utils, serverConfig, "error_page");
    serverConfig.clientMaxBodySize = atoi(serverPars(utils, "client_max_body_size").c_str());
    
    // step 6 : pars locations
    utils.pars = utils.location;
    locationsPars(utils, serverConfig);
    (void)locationsConfig;

    close(fd);
    return (0);
}