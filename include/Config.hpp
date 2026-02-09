#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <map>
#include <vector>
#include "ServerConfig.hpp"

typedef struct locationsConfigS
{
    std::string path;
    std::vector<std::string> methods;
    std::string index;
    std::string autoindex;
    std::string upload_dir;
    std::string cgiExtension; //ici
    std::string cgiBinary; //ici
} locationsT;

typedef struct serverConfigS
{
    std::vector<Listen>	listen; //ici
    int listen; //METTRE VECTOR
    std::string root;
    std::map<int, std::string> errorPage;
    int clientMaxBodySize;
    std::map<std::string, locationsT> locations;
} serverT;

typedef struct utilsConfigS
{
    std::string server;
    std::string location;
    std::string l;
    std::string pars;
    std::string newS;
} utilsConfigT;

int configMain(serverT &serverConfig, utilsConfigT &utils);

#endif