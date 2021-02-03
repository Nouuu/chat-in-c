//
// Created by Unknow on 14/01/2021.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

int clientEngine(const char *server_ip, int port);

int initServerSocket(int port, struct sockaddr_in *clientSocket, int *socketfd, const char *ip);

void clientConnectionHandler(int clientFD);

void *serverListeningThread(void *socketfd);

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("You need to provide ip address and port n°\n");
        return EXIT_FAILURE;
    }
    const char *destAddress = argv[1];
    int port = atoi(argv[2]);

    return clientEngine(destAddress, port);
}

int clientEngine(const char *server_ip, int port) {
    struct sockaddr_in clientSocket;
    int clientFD;
    pthread_t thread_id;

    if (initServerSocket(port, &clientSocket, &clientFD, server_ip) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread_id, NULL, serverListeningThread, (void *) &clientFD) != 0) {
        perror("could not create thread, exit program\n");
        return EXIT_FAILURE;
    }

    clientConnectionHandler(clientFD);

    return EXIT_SUCCESS;
}

int initServerSocket(int port, struct sockaddr_in *clientSocket, int *socketfd, const char *ip) {
    clientSocket->sin_family = AF_INET;
    clientSocket->sin_port = htons(port);

    inet_pton(AF_INET, ip, &clientSocket->sin_addr);

    *socketfd = socket(AF_INET, SOCK_STREAM, 0);

    int result = connect(*socketfd, (struct sockaddr *) clientSocket, sizeof(*clientSocket));

    if (result == -1) {
        printf("Error when connect socket!\n");
        perror("");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void clientConnectionHandler(int clientFD) {

    char *bufferMessage = calloc(256, sizeof(char));
    while (1) {
        printf("Enter a message (0 to exit) :\n");
        fgets(bufferMessage, 255, stdin);
        if (!strcmp(bufferMessage, "0\r\n") || !strcmp(bufferMessage, "0\n")) {
            break;
        }
        send(clientFD, bufferMessage, 255, 0);
    }

    close(clientFD);
    free(bufferMessage);
}

void *serverListeningThread(void *socketfd) {
    int clientFD = *(int *) socketfd;
    char *bufferMessage = calloc(2049, sizeof(char));

    while (recv(clientFD, bufferMessage, 2048, 0) > 0) {
        printf("SERVER MESSAGE :\n%s\n----------------\n", bufferMessage);
        memset(bufferMessage, 0, 2047);
    }


    pthread_exit(NULL);
}