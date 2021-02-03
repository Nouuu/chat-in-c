//
// Created by Unknow on 14/01/2021.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

char bufferMessage[256] = {0};

int serverEngine(int port);

int initServerSocket(int port, struct sockaddr_in *serverSocket, int *socketfd);

int main(int arg, char **argv) {
    if (arg != 2) {
        printf("You need to provide port n°\n");
        return EXIT_FAILURE;
    }
    int port = atoi(argv[1]);

    if (port == 0) {
        return EXIT_FAILURE;
    }

    return serverEngine(port);
}

int serverEngine(int port) {
    struct sockaddr_in serverSocket; //main socket variable
    struct sockaddr_in clientSocket;
    int clientSocketSize = sizeof(clientSocket);
    int serverFD; // Socket file descriptor to identify socket
    int clientFD;

    if (initServerSocket(port, &serverSocket, &serverFD) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    listen(serverFD, 40);

    printf("Server on and listen on %d port\n", port);


    while ((clientFD = accept(serverFD, (struct sockaddr *) &clientSocket, (socklen_t *) &clientSocketSize))) {
        printf("Connected !\nClient IP is %s\n", inet_ntoa(clientSocket.sin_addr));

        int pid = fork();
        if (pid == 0) {
            while (recv(clientFD, bufferMessage, 255, 0) > 0) {

                if (!strcmp(bufferMessage, "0\r\n") || !strcmp(bufferMessage, "0\n")) {
                    break;
                }
                printf("New message from %s : %s\n", inet_ntoa(clientSocket.sin_addr), bufferMessage);
                memset(bufferMessage, 0, 256);
            }
            printf("%s leaving\n", inet_ntoa(clientSocket.sin_addr));
            close(serverFD);
            close(clientFD);
            return EXIT_SUCCESS;
        }
    }

    close(serverFD);
    return EXIT_SUCCESS;
}

int initServerSocket(int port, struct sockaddr_in *serverSocket, int *socketfd) {
    serverSocket->sin_family = AF_INET;
    serverSocket->sin_port = htons(port);
    serverSocket->sin_addr.s_addr = INADDR_ANY;

    *socketfd = socket(AF_INET, SOCK_STREAM, 0);

    int result = bind(*socketfd, (struct sockaddr *) serverSocket, sizeof(*serverSocket));
    if (result == -1) {
        printf("Error when bind socket!\n");
        perror("");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}