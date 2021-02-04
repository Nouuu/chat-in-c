//
// Created by Clément on 03/02/2021.
//

#ifndef CHAT_IN_C_SERVERCLIENT_H
#define CHAT_IN_C_SERVERCLIENT_H

#include "mySocket.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct ServerClient ServerClient;

#include "server.h"

struct ServerClient{
    char *name;
    char buffer[256];
    char status;
    pthread_t pthread;
    SOCKET clientSocketFd;
    struct sockaddr_in clientSocketAddr;

    Server *server;
};

ServerClient *createServerClient(Server *server);
void freeServerClient(ServerClient *client);

#endif //CHAT_IN_C_SERVERCLIENT_H
