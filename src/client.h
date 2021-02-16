//
// Created by Clément on 30/01/2021.
//

#ifndef CHAT_IN_C_CLIENT_H
#define CHAT_IN_C_CLIENT_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include "mySocket.h"
#include "ServerClientCommon.h"

#define CLIENT_DISCONNECTED 1

typedef struct Client{
    SOCKET socket_fd;
    char *receivingBuffer;
    char *sendingBuffer;
    int status;
    pthread_t receivingThread, sendingThread;
    size_t sendingBufferSize, receivingBufferSize;
}Client;

Client *Client_create(const char *address, int port, const char *name);
void startClient(Client *client);

int clientSendMessage(Client *client, const char *msg);

void closeClient(Client *client);
void freeClient(Client *client);

#endif //CHAT_IN_C_CLIENT_H
