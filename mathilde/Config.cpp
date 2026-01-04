/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/04 15:47:48 by mathildelau       #+#    #+#             */
/*   Updated: 2026/01/04 17:11:55 by mathildelau      ###   ########.fr       */
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

void serverPars(utilsConfigT &utils)
{
    size_t start = utils.server.find("listen");
    size_t end = utils.server.find("root");
    std::string tmp;
    tmp = utils.server.substr(start, end);
    utils.s.listen = atoi(tmp.c_str()); // c_str() convert std::string -> const char*
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
    std::string confFinal = trim(conf);

    // step 4 : find bloc
    blocConfig(conf, utils);

    // step 5 : pars server
    serverPars(utils);

    // step 6 : pars locations
    (void)serverConfig;
    (void)locationsConfig;

    close(fd);
    return (0);
}