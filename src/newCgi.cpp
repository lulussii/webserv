/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   newCgi.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lserodon <lserodon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 11:08:12 by mathildelau       #+#    #+#             */
/*   Updated: 2026/02/22 09:09:40 by lserodon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <wait.h>

/**
 * @brief Reads data from a CGI process pipe and handles the end-of-output.
 *
 * This function is called when the CGI output pipe (stdout) is ready for reading
 * as indicated by poll(). It reads available data from the pipe and appends it
 * to the corresponding CGI client's response buffer. The function handles
 * three main cases:
 *
 * 1. Data available (n > 0):
 *    - Reads up to 4096 bytes from the pipe.
 *    - Appends the read data to `cgiClient->response`.
 *
 * 2. End-of-file (n == 0):
 *    - Indicates that the CGI process has finished writing.
 *    - Closes the pipe and removes the file descriptor from the poll set.
 *    - Removes the fd from `cgiReadMap` and marks `cgiClient->reading` as false.
 *    - Calls `parsStdout()` to parse the CGI output.
 *    - Calls `buildCgiResponse()` to create the HTTP response for the client.
 *    - If the client socket still exists:
 *        - Assigns the response to `client.writeBuffer`.
 *        - Marks the client as ready to write.
 *        - Adds the client socket to the poll set with POLLOUT.
 *    - If the CGI had no more input to write and its write pipe is still open:
 *        - Closes the write pipe.
 *        - Removes it from the poll set and `cgiWriteMap`.
 *        - Sets `cgiClient->writePipe[1]` to -1.
 *
 * 3. Read error (n < 0):
 *    - Indicates a read failure on the pipe.
 *    - Closes the pipe and removes it from the poll set and `cgiReadMap`.
 *    - Marks `cgiClient->reading` as false.
 *
 */
void Server::handleCgiRead(int fd)
{
	std::cout << "[INFO] CGI READ called\n";
    
	cgi *cgiClient = cgiReadMap[fd];
	char buffer[4096];

	ssize_t n = read(fd, buffer, sizeof(buffer));
	if (n > 0)
	{
		cgiClient->response.append(buffer, n);
		// std::cout << "[DEBUG] Reading is : " << cgiClient->response << std::endl;
	}
    
    int status;
    pid_t result = waitpid(cgiClient->pid, &status, WNOHANG);

    std::cout << "RESULT = " << result;
    std::cout << " PID = " << cgiClient->pid << std::endl;
    if (result == cgiClient->pid || n == 0)
	{
		std::cout << "ICI";
		close(fd);
		removeFdFromPoll(fd);
		cgiReadMap.erase(fd);
		cgiClient->reading = false;

		// Pars CGI output and and build HTTP response
		parsStdout(*cgiClient);
		responseT response;
		buildCgiResponse(*cgiClient, response);
		
		if (_clients.count(cgiClient->clientFd))
		{
			Client &client = _clients[cgiClient->clientFd];
			client.writeBuffer = response.response;
			client.isReadyToWrite = true;
			addFdToPoll(client.fd, POLLOUT);
		}

		if (!cgiClient->writing && cgiClient->writePipe[1] >= 0)
		{
			close(cgiClient->writePipe[1]);
			removeFdFromPoll(cgiClient->writePipe[1]);
			cgiWriteMap.erase(cgiClient->writePipe[1]);
			cgiClient->writePipe[1] = -1;
		}
	}
	else if (n == -1)
	{
		close(fd);
		removeFdFromPoll(fd);
		cgiReadMap.erase(fd);
		cgiClient->reading = false;
	}
}

/**
 * @brief Handles writing data to a CGI process via its write pipe.
 *
 * This function is called when a CGI pipe is ready for writing (POLLOUT).
 * It writes as much data as possible from the CGI client's buffer into the pipe.
 *
 * Steps:
 * 1. Retrieve the `cgi*` object corresponding to the fd from `cgiWriteMap`.
 * 2. If `cgiClient->writeBuffer` is empty:
 *    - Disable POLLOUT on this fd in `_fds`.
 *    - Return immediately, no data to write.
 *
 * 3. Attempt to write the buffer:
 *    - Calls `write(fd, buffer, size)`.
 *    - `n` is the number of bytes successfully written.
 *
 * 4. If `n > 0` (some data was written):
 *    - Remove the written portion from `writeBuffer`.
 *    - If the buffer still contains data:
 *       - Keep POLLOUT enabled so the next poll will attempt to write remaining data.
 *    - If the buffer is now empty:
 *       - Close the fd and remove it from poll.
 *       - Remove it from `cgiWriteMap`.
 *       - Mark the CGI client as no longer writing.
 *       - Set `writePipe[1]` to -1.
 *
 * 5. If `write` returns an error (n == -1):
 *    - Close the fd, remove it from poll, remove from `cgiWriteMap`, mark writing false.
 *
 * 6. Otherwise (would block):
 *    - Do nothing and wait for the next POLLOUT event.
 *
 * @note This function **does not use `errno`** after write.
 *       It assumes `n == -1` is a fatal error unless it can be retried later.
 *       POLLOUT events are used to know when the pipe is ready for writing.
 */
void Server::handleCgiWrite(int fd)
{
	std::cout << "[INFO] CGI WRITE called\n";
	cgi *cgiClient = cgiWriteMap[fd];

	if (cgiClient->writeBuffer.empty())
	{
		// Disable POLLOUT
		for (int i = 0; i < MAX_TOTAL_FDS; i++)
		{
			if (_fds[i].fd == fd)
			{
				_fds[i].events &= ~POLLOUT;
				break;
			}
		}
		// cgiClient->writing = false;
		return;
	}

	ssize_t n = write(fd, cgiClient->writeBuffer.c_str(), cgiClient->writeBuffer.size());

	if (n > 0)
	{
		cgiClient->writeBuffer.erase(0, n);

		if (!cgiClient->writeBuffer.empty())
		{
			// Go in POLLOUT for the next poll
			for (int i = 0; i < MAX_TOTAL_FDS; i++)
			{
				if (_fds[i].fd == fd)
				{
					_fds[i].events |= POLLOUT;
					break;
				}
			}
		}
		else
		{
			close(fd);
			removeFdFromPoll(fd);
			cgiWriteMap.erase(fd);
			cgiClient->writePipe[1] = -1;
			cgiClient->writing = false;
		}
	}
    
	else if (n == -1)
	{
		close(fd);
		removeFdFromPoll(fd);
		cgiWriteMap.erase(fd);
		cgiClient->writing = false;
	}
}

/**
 * @brief Reaps finished CGI child processes to prevent zombie processes.
 *
 * This function iterates through all active CGI processes tracked in `cgiPidMap`.
 * For each CGI process:
 * 1. Calls `waitpid(pid, &status, WNOHANG)` to check if the child process has finished.
 *    - `WNOHANG` ensures that the call is non-blocking, i.e., it returns immediately
 *      whether or not the child has exited.
 * 2. If `waitpid` returns a positive value, the child process has exited.
 *    - The corresponding entry in `cgiPidMap` is erased, cleaning up the internal tracking.
 * 3. If `waitpid` returns 0, the child process is still running.
 *    - The iterator is incremented to check the next CGI process.
 */
void Server::checkCgiProcess()
{
	for (std::map<pid_t, cgi *>::iterator it = cgiPidMap.begin(); it != cgiPidMap.end();)
	{
		int status;

		pid_t result = waitpid(it->first, &status, WNOHANG);

		if (result > 0)
			it = cgiPidMap.erase(it);
		else
			++it;
	}
}

/**
 * @brief Build the environment variables needed for executing a CGI script and prepare them
 *        in a format suitable for execve.
 *
 *    - "REQUEST_METHOD=" + cgiClient.method
 *      Stores the HTTP request method (e.g., GET, POST) that the CGI script will need.
 *
 *    - "SCRIPT_FILENAME=" + cgiClient.scriptPath
 *      Provides the absolute path to the CGI script on the server filesystem.
 *
 *    - "QUERY_STRING=" + cgiClient.queryString
 *      Contains the portion of the URL after '?' for GET requests, so the CGI script can parse query parameters.
 *
 *    - "CONTENT_TYPE=" + cgiClient.contentType
 *      Indicates the MIME type of the request body (e.g., "application/x-www-form-urlencoded") for POST requests.
 *
 *    - "CONTENT_LENGTH=" + cgiClient.contentLenght
 *      Specifies the length, in bytes, of the HTTP request body for POST requests.
 *
 *    - "GATEWAY_INTERFACE=CGI/1.1"
 *      Standard CGI variable indicating the CGI specification version supported.
 *
 *    - "SERVER_PROTOCOL=" + cgiClient.serverProtocol
 *      HTTP protocol used by the client (e.g., "HTTP/1.1").
 *
 *    - "SERVER_NAME=" + cgiClient.serverName
 *      The server hostname or IP address as seen by the client.
 *
 *    - "SERVER_PORT=" + cgiClient.serverPort
 *      Port number on which the server received the request.
 *
 *    - "REDIRECT_STATUS=" + cgiClient.code
 *      HTTP status code used when redirection occurs, often required by some CGI implementations (e.g., PHP).
 *
 * Convert the `env` vector of std::string to a `envp` array of char* suitable for execve:
 *    - Iterate over each string in `env`:
 *        * Use `c_str()` to get a pointer to the internal null-terminated C string.
 *        * Use `const_cast<char*>` to remove constness because execve expects `char* const envp[]`.
 *        * Push this pointer into the `envp` vector.
 *
 * Append a final NULL pointer to `envp`:
 *    - This NULL terminator is required by execve to signal the end of the environment array.
 */
static void buildEnv(cgi &cgiClient, std::vector<std::string> &env, std::vector<char *> &envp)
{
    env.push_back("REQUEST_METHOD=" + cgiClient.method);
    env.push_back("SCRIPT_FILENAME=" + cgiClient.scriptPath);
    env.push_back("QUERY_STRING=" + cgiClient.queryString);
    env.push_back("CONTENT_TYPE=" + cgiClient.contentType);
    env.push_back("CONTENT_LENGTH=" + cgiClient.contentLenght);
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=" + cgiClient.serverProtocol);
    env.push_back("SERVER_NAME=" + cgiClient.serverName);
    env.push_back("SERVER_PORT=" + cgiClient.serverPort);
    env.push_back("REDIRECT_STATUS=" + cgiClient.code);
    
    for (size_t i = 0; i < env.size(); i++)
        envp.push_back(const_cast<char *>(env[i].c_str()));
    envp.push_back(NULL);
}


/**
 * @brief Fork a new process to handle a CGI request for a given client.
 *
 * This function prepares and executes a CGI script in a separate process while setting up
 * non-blocking communication between the server and the CGI child process using pipes.
 *
 * 1. Check if the CGI process has already been forked:
 *    - If cgiClient.pid != -1, it means a process already exists, so return immediately.
 *
 * 2. If no process exists (pid == -1), prepare to fork
 *
 * 3. Create two pipes for communication with the CGI child:
 *    - writePipe: Server writes data to the CGI process (stdin of CGI).
 *    - readPipe: Server reads data from the CGI process (stdout of CGI).
 *    - If either pipe creation fails, close any opened file descriptors and throw a runtime error.
 *
 * 4. Fork the current process:
 *    - If fork fails (pid == -1), close the pipes and throw a runtime error.
 *
 * 5. Child process (pid == 0):
 *    - Redirect standard output (STDOUT_FILENO) to the readPipe[1] so CGI output goes to the pipe.
 *    - Redirect standard input (STDIN_FILENO) to the writePipe[0] so CGI can read server input.
 *    - Close all unused pipe file descriptors.
 *    - Prepare the argument array `args` for execve:
 *        * args[0]: Path to the CGI binary (e.g., /usr/bin/php)
 *        * args[1]: Path to the CGI script
 *        * args[2]: NULL terminator
 *    - Build the environment variables (env and envp) for the CGI script using buildEnv().
 *    - Change the current working directory to the directory of the CGI script using chdir().
 *        * Extract the directory path from the full script path.
 *        * If chdir fails, print an error message.
 *    - Execute the CGI binary with execve(), passing the binary path, arguments, and environment.
 *        * If execve fails, print an error message.
 *
 * 6. Parent process (pid > 0):
 *    - Associate the client file descriptor with the CGI process.
 *    - Close the ends of the pipes that the parent does not use:
 *        * Close writePipe[0] (read end of write pipe) and readPipe[1] (write end of read pipe)
 *    - Set the reading pipe to non-blocking mode using fcntl().
 *    - Mark cgiClient.reading as true and clear any previous read buffer.
 *    - Initialize the write buffer with the HTTP body to send to CGI.
 *
 * 7. Handle writing to the CGI process:
 *    - If there is no body to send:
 *        * Close the writing pipe to signal EOF to the CGI process.
 *        * Set cgiClient.writing to false.
 *    - If there is a body:
 *        * Set the writing pipe to non-blocking mode using fcntl().
 *        * Mark cgiClient.writing as true.
 *        * Add the writing pipe to poll for POLLOUT events to send data to CGI.
 *        * Store a mapping from the writing pipe FD to the CGI client object.
 *
 * 8. Handle reading from the CGI process:
 *    - Add the reading pipe to poll for POLLIN events to receive CGI output.
 *    - Store a mapping from the reading pipe FD to the CGI client object.
 *
 * 9. Save the CGI process PID:
 *    - Map the child PID to the CGI client object for tracking purposes.
 *
 * 10. Mark the client as not ready to write:
 *    - Set client.isReadyToWrite to false until the CGI response is ready.
 */
void Server::forkCgi(cgi &cgiClient, Client &client)
{
	if (cgiClient.pid != -1)
		return;

	if (cgiClient.pid == -1)
	{
		std::cout << "[INFO] CGI Fork called\n";
		if (pipe(cgiClient.writePipe) == -1 || pipe(cgiClient.readPipe) == -1)
		{
			close(cgiClient.writePipe[0]);
			close(cgiClient.writePipe[1]);
			close(cgiClient.readPipe[0]);
			close(cgiClient.readPipe[1]);
			throw std::runtime_error("Pipe error");
		}

		cgiClient.pid = fork();

		if (cgiClient.pid == -1)
		{
			close(cgiClient.writePipe[0]);
			close(cgiClient.writePipe[1]);
			close(cgiClient.readPipe[0]);
			close(cgiClient.readPipe[1]);
			throw std::runtime_error("Fork error");
		}

		else if (cgiClient.pid == 0) // child
		{
			if (dup2(cgiClient.writePipe[0], STDIN_FILENO) == -1)
                std::cerr << "Error: dup2 failed." << std::endl;
			if (dup2(cgiClient.readPipe[1], STDOUT_FILENO) == -1)
                std::cerr << "Error: dup2 failed." << std::endl;
			close(cgiClient.readPipe[0]);
			close(cgiClient.readPipe[1]);
			close(cgiClient.writePipe[0]);
			close(cgiClient.writePipe[1]);

			char *args[] = {const_cast<char *>(cgiClient.binaryPath.c_str()), const_cast<char *>(cgiClient.scriptPath.c_str()), NULL};
            std::vector<std::string> env;
            std::vector<char *> envp;
            buildEnv(cgiClient, env, envp);

			std::string dir = cgiClient.scriptPath; // go in repertory of script
			size_t pos = dir.rfind("/");
			dir = dir.substr(0, pos + 1);
			if (chdir(dir.c_str()) == -1)
				std::cerr << "Error: Directory Cgi failed." << std::endl;
			
			execve(cgiClient.binaryPath.c_str(), args, envp.data());
			std::cerr << "Error: Execve failed." << std::endl;
		}
		else // parent
		{
			cgiClient.clientFd = client.fd;
			
			//close what we don't use
			close(cgiClient.writePipe[0]);
			close(cgiClient.readPipe[1]);
			
			// do reading no blocking
			int info = fcntl(cgiClient.readPipe[0], F_GETFL, 0);
			fcntl(cgiClient.readPipe[0], F_SETFL, info | O_NONBLOCK);

			cgiClient.reading = true;
			cgiClient.readBuffer.clear();
			
			//Init writing buffer to CGI
			cgiClient.writeBuffer = cgiClient.body;

            std::cout << "BODY SIZE = " << cgiClient.body.size() << std::endl;
			if (cgiClient.body.empty())
			{
				//nothing to write so we close writing pipe to have EOF (GET)
				close(cgiClient.writePipe[1]);
				cgiClient.writePipe[1] = -1;
				cgiClient.writing = false;
			}
            
			else if (!cgiClient.body.empty())
			{
				// do writing no blocking
				info = fcntl(cgiClient.writePipe[1], F_GETFL, 0);
				fcntl(cgiClient.writePipe[1], F_SETFL, info | O_NONBLOCK);

				// can write
				cgiClient.writing = true;
				
				// add writing pipe to send infos
				addFdToPoll(cgiClient.writePipe[1], POLLOUT);
				cgiWriteMap[cgiClient.writePipe[1]] = &cgiClient;
			}

			// add the CGI reading pipe to the poll to retrieve its output
			addFdToPoll(cgiClient.readPipe[0], POLLIN);

			cgiReadMap[cgiClient.readPipe[0]] = &cgiClient;
			
			// save the pid of the CGI
			cgiPidMap[cgiClient.pid] = &cgiClient;
			
			client.isReadyToWrite = false;
		}
	}
}