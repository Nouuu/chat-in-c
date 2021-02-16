//
// Created by Clément on 03/02/2021.
//

#ifndef CHAT_IN_C_SERVER_H
#define CHAT_IN_C_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "mySocket.h"

typedef struct server Server;
#define clientSocketSize (socklen_t)sizeof( struct  sockaddr_in)

#include "ServerClient.h"

struct server{
    pthread_mutex_t mutexClientList;
    SOCKET serverSocketFd;
    int status;
    size_t capacityMax, size;
    ServerClient **clients;
    pthread_t serverThread, commandThread;
    char *messageBuffer;
    size_t messageBufferSize;
    char commandBuffer[256];
};

Server* CreateServer(int port, int capacity);
void startServer(Server *server);
void startReceivingServer(Server *server);

void waitForServer(Server *server);

ServerClient *getServerClient(Server *server, char *name);

void removeClient(Server *server, ServerClient *client);
void sendToAllFromClient(Server *server, ServerClient  *client, char *msg);

void closeServer(Server *server);
void freeServer(Server *server);

#endif //CHAT_IN_C_SERVER_H
