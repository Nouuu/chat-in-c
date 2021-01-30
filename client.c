//
// Created by Unknow on 14/01/2021.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mySocket.h"


struct sockaddr_in client;
SOCKET sockedfd;
char bufferMessage[256] = {0};

int main(int argc, char **argv) {

    if (argc != 3) {
        printf("You need to provide ip address and port n°\n");
        return EXIT_FAILURE;
    }

   // initSocket();

    const char *destAddress = argv[1];
    int port = atoi(argv[2]);

    sockedfd = createClientSocket(AF_INET, SOCK_STREAM, 0, port, destAddress);
    if(sockedfd == -1){
       return EXIT_FAILURE;
    }

    while (1) {
        printf("Enter a message (0 to exit) :\n");
        fgets(bufferMessage, 255, stdin);
        if (!strcmp(bufferMessage, "0\r\n") || !strcmp(bufferMessage, "0\n")) {
            break;
        }
        send(sockedfd, bufferMessage, 255, 0);
    }
    close(sockedfd);

    endSocket();
    return EXIT_SUCCESS;
}
