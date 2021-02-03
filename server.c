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

struct sockaddr_in clientSocket;
int clientSocketSize;
int socketfd;
int clientfd;
char bufferMessage[256] = {0};

int serverEngine(int port);
int main(int arg, char **argv) {
    if (arg != 2) {
        printf("You need to provide port n°\n");
        return EXIT_FAILURE;
    }
    int port = atoi(argv[1]);

    if(port == 0) {
        return EXIT_FAILURE;
    }

    return serverEngine(port);
}

int serverEngine(int port) {
    struct sockaddr_in serverSocket;
    serverSocket.sin_family = AF_INET;
    serverSocket.sin_port = htons(port);
    serverSocket.sin_addr.s_addr = INADDR_ANY;

    socketfd = socket(AF_INET, SOCK_STREAM, 0);

    int result = bind(socketfd, (struct sockaddr *) &serverSocket, sizeof(serverSocket));
    if (result == -1) {
        printf("Error when bind socket!\n");
        perror("");
        return EXIT_FAILURE;
    }

    listen(socketfd, 40);

    printf("Server on and listen on %d port\n", port);

    clientSocketSize = sizeof(clientSocket);

    while ((clientfd = accept(socketfd, (struct sockaddr *) &clientSocket, (socklen_t *) &clientSocketSize))) {
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
            close(socketfd);
            close(clientfd);
            return EXIT_SUCCESS;
        }
    }

    close(socketfd);
    return EXIT_SUCCESS;
}