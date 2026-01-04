#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <map>
#include <vector>

typedef struct locationsConfigS
{
    std::string path;
    std::vector<std::string> methods;
    std::string index;
    std::string autoindex;
    std::string upload_dir;

} locationsT;

typedef struct serverConfigS
{
   
    int listen;
    std::string root;
    std::map<int, std::string> errorPage;
    int clientMaxBodySize;
    std::map<std::string, locationsT> locations;

} serverT;

typedef struct utilsConfigS
{
    bool server;
    bool location;
} utilsConfigT;

int configMain(serverT &serverConfig, locationsT &locationsConfig);

#endif