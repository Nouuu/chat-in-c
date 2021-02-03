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
#include <pthread.h>

struct arg_struct {
    int *socketfd;
    const char *ipaddress;
};

int serverEngine(int port);

int initServerSocket(int port, struct sockaddr_in *serverSocket, int *socketfd);

void serverListenLoop(int serverFD);

void *clientConnectionHandler(void *args);

void clientListenLoop(int clientFD, char *bufferMessage, char *ip);

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
    int serverFD; // Socket file descriptor to identify socket
    if (initServerSocket(port, &serverSocket, &serverFD) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    listen(serverFD, 40);

    printf("Server on and listen on %d port\n", port);

    serverListenLoop(serverFD);

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

void serverListenLoop(int serverFD) {
    struct sockaddr_in clientSocket;
    int clientSocketSize = sizeof(clientSocket);
    int clientFD;
    pthread_t thread_id;

    while ((clientFD = accept(serverFD, (struct sockaddr *) &clientSocket, (socklen_t *) &clientSocketSize))) {
        struct arg_struct args;
        args.socketfd = &clientFD;
        args.ipaddress = inet_ntoa(clientSocket.sin_addr);

        if (pthread_create(&thread_id, NULL, clientConnectionHandler, (void *) &args) != 0) {

            perror("could not create thread, exit program");
            return;
        }
    }
}

void *clientConnectionHandler(void *args) {
    struct arg_struct arguments = *(struct arg_struct *) args;

    char *ip = strdup(arguments.ipaddress);
    int clientFD = *(int *) arguments.socketfd;
    char *bufferMessage = calloc(256, sizeof(char));

    printf("Connected !\nClient IP is %s\n", ip);

    clientListenLoop(clientFD, bufferMessage, ip);

    printf("%s leaving\n", ip);
    close(clientFD);
    free(ip);
    free(bufferMessage);
    pthread_exit(NULL);
}

void clientListenLoop(int clientFD, char *bufferMessage, char *ip) {
    while (recv(clientFD, bufferMessage, 255, 0) > 0) {

        if (strstr(bufferMessage, server_exit_command) == bufferMessage) {
            break;
        }
        printf("New message from %s : %s\n", ip, bufferMessage);
        memset(bufferMessage, 0, 256);
    }
}