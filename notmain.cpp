#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <iostream>

#define PORT 8080
#define MAX_EVENTS 10
#define MAX_BUFFER 1024

// Set socket to non-blocking mode
void set_nonblocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int server_fd, epoll_fd, client_fd, nfds;
    struct sockaddr_in server_addr, client_addr;
    struct epoll_event ev, events[MAX_EVENTS];
    char buffer[MAX_BUFFER];

    // Create server socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Allow port reuse
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    // Listen

    if (listen(server_fd, 2) < 0) {
        perror("Listen failed");
        exit(1);
    }

    // Set server socket to non-blocking
    set_nonblocking(server_fd);
    // Create epoll instance
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("Epoll creation failed");
        exit(1);
    }

    // Add server socket to epoll
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        perror("Epoll add server socket failed");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    // Main loop
    while (1) {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
		// std::cout
        if (nfds < 0) {
            perror("Epoll wait failed");
            exit(1);
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_fd) {
                // New connection
                socklen_t client_len = sizeof(client_addr);
                client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                if (client_fd < 0) {
                    perror("Accept failed");
                    continue;
                }

                // Set client socket to non-blocking
                set_nonblocking(client_fd);

                // Add client socket to epoll
                ev.events = EPOLLIN | EPOLLET; // Edge-triggered
                ev.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
                    perror("Epoll add client socket failed");
                    close(client_fd);
                    continue;
                }
            } else {
                // Handle client data
                int client_fd = events[i].data.fd;
                ssize_t bytes_read = read(client_fd, buffer, MAX_BUFFER - 1);
                // std::cout << buffer<< "\n"; std::string header(buffer); //ICI
                if (bytes_read <= 0) {
                    // Connection closed or error
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                    close(client_fd);
                    continue;
                }

                buffer[bytes_read] = '\0';
                // Basic HTTP request parsing
                if (strncmp(buffer, "GET ", 4) == 0) {
                    // Respond with a simple HTML page
                    const char *response =
                        "HTTP/1.1 404 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "<html><head><title>Welcome</title></head>"
                        "<body><h2>Hello, World!</h2></body></html>";

                    write(client_fd, response, strlen(response));
                }

                // Close client socket
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                close(client_fd);
            }
        }
    }

    // Cleanup (unreachable in this example)
    close(server_fd);
    close(epoll_fd);
    return 0;
}