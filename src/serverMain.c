//
// Created by Cleme on 16/02/2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "server.h"

int main(int arg, char **argv) {
    Server *server;
    int port;

    if (arg != 2) {
        printf("You need to provide port n°\n");
        return EXIT_FAILURE;
    }

    initSocket();

    port = atoi(argv[1]);

    server = CreateServer(port, 40);

    if(server != NULL){
        startServer(server);
        waitForServer(server);
        closeServer(server);

        freeServer(server);
    }

    endSocket();
    return EXIT_SUCCESS;
}