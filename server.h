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

#include "ServerClient.h"

struct server{
    SOCKET serverSocketFd;
    unsigned int clientSocketSize;
    int status;
    size_t capacityMax, size;
    ServerClient **clients;
};

void removeClient(Server *server, struct ServerClient *client);

#endif //CHAT_IN_C_SERVER_H
