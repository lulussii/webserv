/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mlaussel <mlaussel@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 10:05:42 by mlaussel          #+#    #+#             */
/*   Updated: 2025/12/16 12:49:17 by mlaussel         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "parsing.hpp"

#include <sys/socket.h> // socket, bind
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // htons
#include <cstring>      // memset

/*
             -------------------
             
1- socket() → create a cable

            -------------------
            
2- bind() → Specify which port I should plug the cable into.
        `bind()` tells the system:
        "When someone connects to this port, send them to this socket."

            -------------------
            
3- listen() → saying “I am ready”

    After listen()the port becomes open


    int listen(int sockfd, int backlog);
    - sockfd : The socket you created with socket()
            He must be of type SOCK_STREAM
            He must have been bind() before
    - backlog : Maximum number of pending connections
            It is a queue managed by the kernel

    Imagine a restaurant :

    listen() = you open the door

    accept() = you bring in a customer

    backlog = the number of people who can wait at the door

    If :

    backlog = 10

    10 customers arrive at the same time

    the 11th → refused (connection reset)

    In practice:

    - current values: 10, 50, 128

    - the kernel can automatically adjust
             -------------------

4- accept() → Someone connects → only then do you have a way to talk

 - bind = tells the system which port my server is listening on
 - sockaddr_in = the identity information for that port

             -------------------
*/



/**
 * @brief ``
 *
 * SOCKET :
 *
 * - AF_INET = IPv4(127.0.0.1)
 *
 * - SOCK_STREAM = TCP (fiable, ordonné)
 *      HTTP = TCP = SOCK_STREAM
 *
 * - 0 because automatic to choose good protocole
 *
 *
 */
int server()
{
    int fdServer;

    // step 1 socket :  create a cable
    if ((fdServer = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
        return (1);

    struct sockaddr_in server_addr;
    // This structure describes:
    //  the IP address
    //  the port
    //  the address type (IPv4)
    // Important fields:
    //  sin_family → address type (AF_INET)
    //  sin_port → port (always in network byte order)
    //  sin_addr.s_addr → IP address
    //  ├── famille : "I am IPv4"
    //  ├── port    : "I listen on the port 8080"
    //  └── adresse : "I listen on this machine"

    // --> it's like an ID card : describe where to listen

    // step 1 clean struct
    memset(&server_addr, 0, sizeof(server_addr));

    // choose same domain to socket
    server_addr.sin_family = AF_INET;

    // choose port, htons convert port into bytes
    server_addr.sin_port = htons(8080);

    // choose IP : It means: "I accept connections from any interface." (Exemple : localhost, wifi, ethernet) Here : localhost
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // step 2 bind : Specify which port I should plug the cable into. If the port is already in use, error.
    if (bind(fdServer, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cout << "Error: bind failed\n";
        return (1);
    }

    //step 3 listen : 

    if (listen(fdServer, 2) < 0)
    {
        std::cout << "Error: listen failed\n";
        return (1);
    }

    //step 4 accept :

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    int fdClient = accept(fdServer, (struct sockaddr*)&client_addr, &client_len);
    if (fdClient < 0)
    {
        std::cout << "Error: accept failed\n";
        return (1);
    }

    close(fdServer);
    close(fdClient);

    return (0);
    
}