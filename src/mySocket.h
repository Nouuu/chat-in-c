//
// Created by Clément on 28/01/2021.
//

#ifndef CHAT_IN_C_MYSOCKET_H
#define CHAT_IN_C_MYSOCKET_H

#include <stdio.h>
#include <stdarg.h>

#pragma comment(lib, "ws2_32.lib")

#ifdef WIN32 /* si vous êtes sous Windows */

#include <winsock2.h>

#define SHUT_RDWR 2
#define socklen_t unsigned int
#define CLOSE_SOCKET closesocket

#elif defined (linux) /* si vous êtes sous Linux */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef unsigned long long SOCKET;
#define CLOSE_SOCKET close

#else /* sinon vous êtes sur une plateforme non supportée */
#error not defined for this platform
#endif

void initSocket();
void endSocket();

SOCKET createClientSocket(int domain, int type, int protocol, int port, const char * address);
SOCKET createServerSocket(int domain, int type, int protocol, int port);

void displayLastSocketError(char *msg, ...);



#endif //CHAT_IN_C_MYSOCKET_H
