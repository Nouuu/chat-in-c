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

typedef struct Client{
    SOCKET socket_fd;
    char *receivingBuffer, *sendingBuffer;
    int status;
    pthread_t receivingThread, sendingThread;
}Client;

Client *Client_create(char *address, int port, char *name);

#endif //CHAT_IN_C_CLIENT_H
