//
// Created by Unknow on 14/01/2021.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mySocket.h"

struct sockaddr_in clientSocket;

unsigned int clientSocketSize;

SOCKET serverSocket_fd;
SOCKET clientfd;

int port;
char bufferMessage[256] = {0};

int main(int arg, char **argv) {

    if (arg != 2) {
        printf("You need to provide port n°\n");
        return EXIT_FAILURE;
    }

    initSocket();

    port = atoi(argv[1]);

    serverSocket_fd = createServerSocket(AF_INET, SOCK_STREAM, 0, port);
    if(serverSocket_fd == -1){
        return EXIT_FAILURE;
    }

    if( listen(serverSocket_fd, 40) == -1){
        displayLastSocketError("Error on listen() server socket");
        return  EXIT_FAILURE;
    }

    printf("Server on and listen on %d port\n", port);

    clientSocketSize = sizeof(clientSocket);

    while ((clientfd = accept(serverSocket_fd, (struct sockaddr *) &clientSocket, &clientSocketSize))) {
        printf("Connected !\nClient IP is %s\n", inet_ntoa(clientSocket.sin_addr));

        int pid = fork();
        if (pid == 0) {
            while (recv(clientfd, bufferMessage, 255, 0) > 0) {

                if (!strcmp(bufferMessage, "0\r\n") || !strcmp(bufferMessage, "0\n")) {
                    break;
                }
                printf("New message from %s : %s\n", inet_ntoa(clientSocket.sin_addr), bufferMessage);
                memset(bufferMessage, 0, 256);
            }
            printf("%s leaving\n", inet_ntoa(clientSocket.sin_addr));
            close(serverSocket_fd);
            close(clientfd);
            return EXIT_SUCCESS;
        }
    }

    close(serverSocket_fd);
    endSocket();
    return EXIT_SUCCESS;
}