/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 11:18:36 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/09 14:43:41 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"
#include "Multipart.hpp"
#include "Error.hpp"
#include <unistd.h>   //stat() access() pipe() fork()
#include <sys/stat.h> //struct stat
#include <sstream>    //std::stringstream
#include <sys/wait.h> //waitpid
#include <vector>

/**
 * @brief `check if cgi exist and allowed`
 *
 * step 1 : check if location allowed cgi
 *
 * step 2 : pars url to have test.php without query
 *
 * step 3 : build path (/Users/mathildelaussel/webserv/mathilde/server_files/test.php)
 *
 * step 4 : check if regular file, if yes, search if cgi extension is in path
 */
bool isCgi(request &request, cgi &cgi, responseT &response, serverT &serverConfig)
{
    // std::cout << "EXTENSION : " << response.location.cgiExtension << std::endl;
    // std::cout << "BINARY : " << response.location.cgiBinary << std::endl;
    
    std::string url = request._url;
    if (response.location.cgiBinary.empty() || response.location.cgiExtension.empty())
        return (false);

    size_t pos = url.find("?");
    if (pos != std::string::npos)
    {
        cgi.queryString = url.substr(pos + 1);
        url = url.substr(0, pos);
    }
    std::string path;
    if (url == "/")
        path = serverConfig.root + response.location.index;
    else
        path = serverConfig.root + url;

    struct stat test;
    if (stat(path.c_str(), &test) != -1 && S_ISREG(test.st_mode))
    {
        pos = path.find(response.location.cgiExtension);
        if (pos != std::string::npos)
        {
            cgi.binaryPath = response.location.cgiBinary;
            cgi.scriptPath = path;
            response.cgi = true;
            return (true);
        }
    }
    return (false);
}

/**
 * @brief `check binary and extension`
 *
 * step 1 : check can found script (F_OK) who is /Users/mathildelaussel/webserv/mathilde/server_files/cgi/test.php
 * 
 * step 2 : check can read script (R_OK) who is /Users/mathildelaussel/webserv/mathilde/server_files/cgi/test.php
 *
 * step 3 : check can found binary (F_OK) who is /Users/mathildelaussel/webserv/mathilde/server_files/cgi/php-cgi
 * 
 * step 2 : check can execute binary (X_OK) who is /Users/mathildelaussel/webserv/mathilde/server_files/cgi/php-cgi
 */
int accessCgi(cgi &cgi)
{
    if (access(cgi.scriptPath.c_str(), F_OK) == -1)
        return (404);
    if (access(cgi.scriptPath.c_str(), R_OK) == -1)
        return (403);
    if (access(cgi.binaryPath.c_str(), X_OK) == -1)
        return (404);
    if (access(cgi.binaryPath.c_str(), X_OK) == -1)
        return (403);
    return (0);
}

/**
 * @brief `handle cgi env``
 *
 * step 1 - cgi.method : add method GET POST or DELETE
 *
 * step 3 - cgi.contentType
 *
 * step 4 - cgi.serverName
 *
 * step 5 - cgi.serverPort : listen in server (8080)
 *
 * step 6 - cgi.gatewayInterface
 *
 * step 7 - cgi.serverProtocol : request._version (HTTP/1.1)
 *
 * step 8 - body (chunked or classic)
 * 
 * step 9 - cgi.contentLenght
 * 
 * step 10 - cgi code
 *
 */
void handleCgi(request &request, cgi &cgi, serverT &serverConfig, responseT &response, Multipart &m)
{
    if (request._method == "GET" || request._method == "POST")
        cgi.method = request._method;

    std::map<std::string, std::string>::iterator it = request.headers.find("Content-Type");
    if (it != request.headers.end())
        cgi.contentType = it->second;

    it = request.headers.find("Host");
    if (it != request.headers.end())
        cgi.serverName = it->second;

    std::stringstream convert;
    //convert << serverConfig.listen;
    //cgi.serverPort = convert.str();
    (void)serverConfig;
    cgi.serverPort = "8080"; //hard code to test;

    cgi.gatewayInterface = "CGI/1.1";

    cgi.serverProtocol = request._version;

    if (m.content.empty())
        cgi.body = response.body;
    else
        cgi.body = m.content;
    
    convert << cgi.body.size();
    cgi.contentLenght = convert.str();


    std::stringstream code;
    code << response.code;
    cgi.code = code.str();
    
    // std::cout << "CGI METHOD : " << cgi.method << std::endl;
    // std::cout << "CGI QURRY : " << cgi.queryString << std::endl;
    // std::cout << "CGI CONTENT LENGHT : " << cgi.contentLenght << std::endl;
    // std::cout << "CGI CONTENT TYPE : " << cgi.contentType << std::endl;
    // std::cout << "CGI SCRIPT PATH : " << cgi.scriptPath << std::endl;
    // std::cout << "CGI BINARY PATH : " << cgi.binaryPath << std::endl;
    // std::cout << "CGI SERVER NAME : " << cgi.serverName << std::endl;
    // std::cout << "CGI SERVER PORT : " << cgi.serverPort << std::endl;
    // std::cout << "CGI GATE WAY : " << cgi.gatewayInterface << std::endl;
    // std::cout << "CGI SERVER PROTOCOL : " << cgi.serverProtocol << std::endl;
    // std::cout << "BODY : " << cgi.body << std::endl;
}

/**
 * @brief `create pipe`
 *
 * step 1 : creat two pipe
 *
 * step 2 : fork and check pid
 */
int cgiPipe(cgi &cgi)
{
    int body[2];
    int response[2];
    cgi.response.clear();

    if (pipe(body) == -1 || pipe(response) == -1)
        return (500);

    pid_t pid = fork();

    if (pid < 0)
    {
        close(body[0]);
        close(body[1]);
        close(response[0]);
        close(response[1]);
        return (500);
    }

    else if (pid == 0)
    {
        close(body[1]);     // don't write
        close(response[0]); // dont read

        if (dup2(body[0], STDIN_FILENO) == -1) // read stdin from body
        {
            close(body[0]);
            close(response[1]);
            // return (500);
        }
        if (dup2(response[1], STDOUT_FILENO) == -1) // write on stdout to body
        {
            close(body[0]);
            close(response[1]);
            // return (500);
        }
        close(body[0]);
        close(response[1]);

        char *args[] = {const_cast<char *>(cgi.binaryPath.c_str()), const_cast<char *>(cgi.scriptPath.c_str()), NULL};

        std::vector<std::string> env;
        env.push_back("REQUEST_METHOD=" + cgi.method);
        env.push_back("SCRIPT_FILENAME=" + cgi.scriptPath);
        env.push_back("QUERY_STRING=" + cgi.queryString);
        env.push_back("CONTENT_TYPE=" + cgi.contentType);
        env.push_back("CONTENT_LENGTH=" + cgi.contentLenght);
        env.push_back("GATEWAY_INTERFACE=CGI/1.1");
        env.push_back("SERVER_PROTOCOL=" + cgi.serverProtocol);
        env.push_back("SERVER_NAME=" + cgi.serverName);
        env.push_back("SERVER_PORT=" + cgi.serverPort);
        env.push_back("REDIRECT_STATUS=" + cgi.code);

        std::vector<char *> envp;
        for (size_t i = 0; i < env.size(); i++)
            envp.push_back(const_cast<char *>(env[i].c_str()));
        envp.push_back(NULL);

        std::string dir = cgi.scriptPath; // go in repertory of script
        size_t pos = dir.rfind("/");
        dir = dir.substr(0, pos + 1);
        if (chdir(dir.c_str()) == -1)
        {
            ;
        }
        // std::cout << dir << std::endl;

        execve(cgi.binaryPath.c_str(), args, envp.data());

        // return (500);// check if exceve fail
    }

    else if (pid > 0)
    {
        close(body[0]);     // don't read
        close(response[1]); // don't write

        std::cout << cgi.body << std::endl;
        
        write(body[1], cgi.body.c_str(), cgi.body.size()); //write the body from request (body chunked too)
        close(body[1]); // EOF

        char buffer[1024];
        ssize_t n;
        while ((n = read(response[0], buffer, sizeof(buffer))) > 0)
            cgi.response.append(buffer, n);
        // std::cout.write(buffer, n); //  show exit CGI (test)

        close(response[0]);

        int status;
        waitpid(pid, &status, 0);
    }

    return (0);
}

/**
 * @brief
 *
 * step 1 : split headers and body
 *
 * step 2 : pars in headers content type and status
 */
void parsStdout(cgi &cgi)
{
    int space = 4;
    cgi.headers.clear();
    cgi.body.clear();
    size_t pos = cgi.response.find("\r\n\r\n");
    if (pos == std::string::npos)
    {
        space = 2;
        pos = cgi.response.find("\n\n");
    }
    if (pos != std::string::npos)
    {
        cgi.headers = cgi.response.substr(0, pos);
        cgi.body = cgi.response.substr(pos + space);
    }
    else
        cgi.body = cgi.response;
    // std::cout << "[DEBUG HEADERS] : " << cgi.headers << std::endl;
    // std::cout << "[DEBUG BODY] : " << cgi.body << std::endl;

    pos = cgi.headers.find("Status:");
    if (pos != std::string::npos)
    {
        std::string tmp = cgi.headers.substr(pos);
        // size_t end = tmp.find("\r\n");
        // if (end == std::string::npos)
        //     end = tmp.find("\n");
        cgi.code = tmp.substr(8, 3);
    }
    else
        cgi.code = "200";

    pos = cgi.headers.find("Content-Type:");
    if (pos != std::string::npos)
    {
        std::string tmp = cgi.headers.substr(pos);
        size_t end = tmp.find("\r\n");
        if (end == std::string::npos)
            end = tmp.find("\n");
        cgi.contentType = tmp.substr(14, end - 14);
    }
    else
        cgi.contentType = "text/plain";

    // std::cout << "[DEBUG content] : [" << cgi.contentType << "]" << std::endl;
    // std::cout << "[DEBUG status] : [" << cgi.code << "]" << std::endl;
}

void buildCgiResponse(cgi &cgi, responseT &response)
{
    response.response.clear();

    response.response += cgi.serverProtocol + " " + cgi.code;
    std::map<std::string, std::string>::iterator it = cgi.errorTxt.find(cgi.code);
    if (it != cgi.errorTxt.end())
        response.response += " " + it->second;

    if (cgi.method == "DELETE" || cgi.method == "GET" || cgi.code == "413")
    {
        if (cgi.method == "DELETE" || cgi.code == "413")
        {
            response.response += "\r\n";
            response.response += "Content-Length: 0";
        }
    }
        
    else
    {
        std::stringstream length;
        length << cgi.body.size();
        response.response += "Content-Length: " + length.str();
    }
    response.response += "\r\n";

    response.response += "Content-Type: " + cgi.contentType;
    response.response += "\r\n";

    // empty line
    response.response += "\r\n";

    // if (cgi.method == "POST")
        response.response += cgi.body;

    // std::cout << "---DEBUG RESPONSE REQUEST AFTER CGI---\n\n[" << response.response << "]" << std::endl;
    // if (cgi.method == "GET")
    // {
    //     std::cout << "\n---DEBUG BODY NOT IN RESPONSE---\n\n";
    //     std::cout << cgi.body;
    // }
}

void cgiMain(request &request, cgi &cgi, serverT &serverConfig, responseT &response)
{
    if (isCgi(request, cgi, response, serverConfig) == false)
    {
        std::cout << "FALSE " << std::endl; //debug
        return ;
    }

    int value = accessCgi(cgi);
    if (value == 404)
    {
        errorCode(response, serverConfig, 404);
        return ;
    }
    if (value == 403)
    {
        errorCode(response, serverConfig, 403);
        return ;
    }
}