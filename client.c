//
// Created by Unknow on 14/01/2021.
//

#include "client.h"

SOCKET sockedfd;
char bufferMessage[256] = {0};


Client *Client_create(char *address, int port){
    Client *client = malloc( sizeof(Client));

    client->socket_fd = createClientSocket(AF_INET, SOCK_STREAM, 0, port, address);

    if(client->socket_fd  == -1){
        return NULL;
    }

    return client;
}

int main(int argc, char **argv) {

    if (argc != 4) {
        printf("You need to provide ip address and port n° and name\n");
        return EXIT_FAILURE;
    }

    initSocket();

    const char *destAddress = argv[1];
    int port = atoi(argv[2]);

    sockedfd = createClientSocket(AF_INET, SOCK_STREAM, 0, port, destAddress);
    if(sockedfd == -1){
       return EXIT_FAILURE;
    }

    send(sockedfd, argv[3], strlen(argv[3])+1, 0);

    while (1) {
        printf("Enter a message (0 to exit) :\n");
        memset(bufferMessage, 0, 256);
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
