//
// Created by Clément on 30/01/2021.
//

#ifndef CHAT_IN_C_CLIENT_H
#define CHAT_IN_C_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mySocket.h"

typedef struct Client{
    SOCKET socket_fd;
}Client;

Client *Client_create(char *address, int port);

#endif //CHAT_IN_C_CLIENT_H
