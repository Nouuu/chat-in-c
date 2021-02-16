//
// Created by Cleme on 16/02/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include "client.h"

int main(int argc, char **argv) {
    Client *client;
    if (argc < 3) {
        printf("You need to provide ip address and port n° and name\n");
        return EXIT_FAILURE;
    }

    initSocket();

    char *destAddress = argv[1];
    int port = atoi(argv[2]);

    if(argc >= 4) {
        client = Client_create(destAddress, port, argv[3]);
    }else{
        client = Client_create(destAddress, port, NULL);
    }
    if(client != NULL){
        startClient(client);

        pthread_join(client->sendingThread, NULL);
        pthread_join(client->receivingThread, NULL);

        closeClient(client);
        freeClient(client);
    }

    endSocket();
    return EXIT_SUCCESS;
}


