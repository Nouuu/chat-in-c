//
// Created by Clément on 28/01/2021.
//
#include "mySocket.h"

void initSocket(void)
{
#ifdef WIN32
    WSADATA wsa;
    if( WSAStartup(MAKEWORD(2, 2), &wsa) < 0)
    {
        displayLastSocketError("WSAStartup()");
        exit(EXIT_FAILURE);
    }
#endif
}

void endSocket(void)
{
#ifdef WIN32
    if( WSACleanup() < 0){
        displayLastSocketError("WSACleanup()");
    }
#endif
}


SOCKET createClientSocket(int domain, int type, int protocol, int port, const char * address){
    struct sockaddr_in sockaddrIn;
    SOCKET socket_fd;

    sockaddrIn.sin_family = domain;
    sockaddrIn.sin_port = htons(port);
    sockaddrIn.sin_addr.s_addr = inet_addr(address);

    socket_fd = socket(domain, type, protocol);

    if(socket_fd == -1){
        displayLastSocketError("Failed to create the client socket %d %d %d : ", domain, type, protocol);
    }else if (connect(socket_fd, (struct sockaddr *) &sockaddrIn, sizeof(sockaddrIn)) == -1) {
        displayLastSocketError("Failed to connect %s:%d : ", address, port);
        socket_fd = -1;
    }else{
        printf("connection successful with %s:%d\n", address, port);
    }

    return socket_fd;
}

SOCKET createServerSocket(int domain, int type, int protocol, int port){
    struct sockaddr_in sockaddrIn;
    SOCKET socket_fd;

    sockaddrIn.sin_family = domain;
    sockaddrIn.sin_port = htons(port);
    sockaddrIn.sin_addr.s_addr = INADDR_ANY;

    socket_fd = socket(domain, type, protocol);

    //https://stackoverflow.com/questions/2084830/kill-thread-in-pthread-library
    #ifdef WIN32 /* si vous êtes sous Windows */
        const char true = 1;
    #elif defined (linux) /* si vous êtes sous Linux */
        int true = 1;
    #endif
    setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));

    if(socket_fd == -1){
        displayLastSocketError("Failed to create the server socket %d %d %d : ", domain, type, protocol);
    }else if ( bind(socket_fd, (struct sockaddr *) &sockaddrIn, sizeof(sockaddrIn)) == -1) {
        displayLastSocketError("Failed to bind %d : ", port);
        socket_fd = -1;
    }

    return socket_fd;
}

void displayLastSocketError(char *msg, ...){
    va_list args;

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    #ifdef WIN32 /* si vous êtes sous Windows */
        fprintf(stderr,"%d\n",  WSAGetLastError());
    #elif defined (linux) /* si vous êtes sous Linux */
        perror("");
    #endif
}

