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
    std::string cgiExtension;
    std::string cgiBinary;
} locationsT;

typedef struct serverConfigS
{
    std::vector<Listen>	listens;
    int listen;
    std::string root;
    std::map<int, std::string> errorPage;
    int clientMaxBodySize;
    std::map<std::string, locationsT> locations;
	std::vector<std::string> servernames;
} serverT;

#endif