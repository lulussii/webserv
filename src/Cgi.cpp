/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mathildelaussel <mathildelaussel@studen    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/02 11:18:36 by mlaussel          #+#    #+#             */
/*   Updated: 2026/02/10 17:41:51 by mathildelau      ###   ########.fr       */
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
 * @brief `Check if the request should be handled by a CGI script`
 *
 * step 1 : verify that the location allows CGI (cgiBinary and cgiExtension are set)
 *
 * step 2 : parse URL to separate query string from script path
 *
 * step 3 : build the absolute script path using server root and URL (/Users/mathildelaussel/webserv/mathilde/server_files/test.php)
 *
 * step 4 : check if file exists and is a regular file
 * 
 * step 5 : verify that the file extension matches the configured CGI extension
 * 
 * step 6 : set CGI paths and response flag if CGI
 */
bool isCgi(request &request, cgi &cgi, responseT &response, serverT &serverConfig)
{
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
        pos = path.rfind(response.location.cgiExtension);
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
 * @brief `Check file permissions for CGI execution`
 *
 * step 1 : check if the script exists (F_OK)
 * 
 * step 2 : check if the script is readable (R_OK)
 *
 * step 3 : check if the CGI binary exists (F_OK)
 * 
 * step 4 : check if the CGI binary is executable (X_OK)
 */
int accessCgi(cgi &cgi)
{
    if (access(cgi.scriptPath.c_str(), F_OK) == -1)
        return (404);
    if (access(cgi.scriptPath.c_str(), R_OK) == -1)
        return (403);
    if (access(cgi.binaryPath.c_str(), F_OK) == -1)
        return (404);
    if (access(cgi.binaryPath.c_str(), X_OK) == -1)
        return (403);
    return (0);
}

/**
 * @brief `Prepare CGI environment variables and request body`
 *
 * step 1 : set the request method (GET, POST, etc.)
 * step 2 : get Content-Type from request headers
 * step 3 : get Host from request headers
 * step 4 : set server port
 * step 5 : set gateway interface (CGI/1.1)
 * step 6 : set server protocol (HTTP version)
 * step 7 : set request body (either multipart content or raw body)
 * step 8 : calculate Content-Length
 * step 9 : set status code for CGI environment (REDIRECT_STATUS)
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
 * @brief `Execute the CGI script using fork and pipes`
 *
 * step 1 : create two pipes (one for input, one for output)
 * 
 * step 2 : fork a child process
 *   - (child) : redirect stdin/stdout to pipes using dup2
 *   - (child) : build argv and envp for execve
 *   - (child) : change working directory to script directory
 *   - (child) : execute CGI binary with script
 * 
 * step 3 : parent process
 *   - write request body to child stdin pipe
 *   - read CGI stdout from child into cgi.response
 *   - wait for child process to finish
 *
 * @return 0 on success, 500 on pipe/fork errors
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
            ; //ICI GERER LE CAS ECHEC
        }
        // std::cout << dir << std::endl;

        execve(cgi.binaryPath.c_str(), args, envp.data());
        //ICI GERER LE CAS ECHEC
        // return (500);// check if exceve fail
    }

    else if (pid > 0)
    {
        close(body[0]);     // don't read
        close(response[1]); // don't write
        
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
 * @brief `Parse CGI stdout into headers and body`
 *
 * step 1 : search for header/body separator (\r\n\r\n or \n\n)
 * step 2 : split response into headers and body
 * step 3 : extract Status: header if present, default to 200
 * step 4 : extract Content-Type header if present, default to text/plain
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
    pos = cgi.headers.find("Status:");
    if (pos != std::string::npos)
    {
        std::string tmp = cgi.headers.substr(pos);
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
}

/**
 * @brief `Build the HTTP response from CGI output`
 *
 * step 1 : start response with HTTP version and CGI status code
 * 
 * step 2 : add status text if available
 * 
 * step 3 : determine if Content-Length should be 0 (DELETE, 413) or size of CGI body
 * 
 * step 4 : add Content-Type header
 * 
 * step 5 : add empty line to separate headers from body
 * 
 * step 6 : append CGI body if applicable
 */

void buildCgiResponse(cgi &cgi, responseT &response)
{
    response.response.clear();

    response.response += cgi.serverProtocol + " " + cgi.code;
    
    std::map<std::string, std::string>::iterator it = cgi.errorTxt.find(cgi.code);
    if (it != cgi.errorTxt.end())
        response.response += " " + it->second;

    if (cgi.method == "DELETE" || cgi.code == "413")
    {
        response.response += "\r\n";
        response.response += "Content-Length: 0";
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

    response.response += "\r\n";

    response.response += cgi.body;

    // std::cout << "---DEBUG RESPONSE REQUEST AFTER CGI---\n\n[" << response.response << "]" << std::endl;
    // if (cgi.method == "GET")
    // {
    //     std::cout << "\n---DEBUG BODY NOT IN RESPONSE---\n\n";
    //     std::cout << cgi.body;
    // }
}

/**
 * @brief `Main CGI handler for the request`
 *
 * step 1 : check if request matches a CGI script (isCgi)
 * 
 * step 2 : check script and binary access permissions (accessCgi)
 * 
 * step 3 : set error response (404/403) if checks fail
 */

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