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
#include <ctype.h>

#define server_exit_command "/exit"
#define server_pseudo_command "/pseudo"
#define server_pseudo_max_size 10

typedef struct arg_struct {
    char *ipaddress;
    int *clientSocketFD;
} arg_struct;

int serverEngine(int port);

int initServerSocket(int port, struct sockaddr_in *serverSocket, int *socketfd);

void serverListenLoop(int serverSocketFD);

void *clientHandler(void *args);

void clientListenLoop(int clientFD, const char *ip);

char *getPseudo(const char *bufferMessage, const char *ip);

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
    struct sockaddr_in serverSocket;
    int serverSocketFD;

    if (initServerSocket(port, &serverSocket, &serverSocketFD) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    listen(serverSocketFD, 40);


    printf("Server on and listen on %d port\n", port);

    serverListenLoop(serverSocketFD);

    close(serverSocketFD);
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

void serverListenLoop(int serverSocketFD) {
    struct sockaddr_in clientSocket;
    int clientSocketFD;
    int clientSocketSize;
    clientSocketSize = sizeof(clientSocket);
    pthread_t threadId;

    while ((clientSocketFD = accept(serverSocketFD, (struct sockaddr *) &clientSocket,
                                    (socklen_t *) &clientSocketSize))) {
        arg_struct arguments;
        arguments.clientSocketFD = &clientSocketFD;
        arguments.ipaddress = strdup(inet_ntoa(clientSocket.sin_addr));

        if (pthread_create(&threadId, NULL, clientHandler, (void *) &arguments) != 0) {
            perror("could not create thread\n");
            return;
        }
    }
}

void *clientHandler(void *args) {
    printf("Connected\n");
    arg_struct *arguments = (arg_struct *) args;

    const char *ip = arguments->ipaddress;
    int clientFD = *arguments->clientSocketFD;

    printf("Client connected IP is %s\n", ip);

    clientListenLoop(clientFD, ip);

    printf("%s leaving\n", ip);

    close(clientFD);
    pthread_exit(NULL);
}

void clientListenLoop(int clientFD, const char *ip) {
    char *bufferMessage = calloc(256, sizeof(char));
    char *pseudo = NULL;

    while (recv(clientFD, bufferMessage, 255, 0) > 0) {

        if (strstr(bufferMessage, server_exit_command) == bufferMessage) {
            break;
        } else if (strstr(bufferMessage, server_pseudo_command) == bufferMessage) {
            pseudo = getPseudo(bufferMessage, ip);
        } else {
            if (pseudo) {
                printf("New message from %s (%s) : %s\n", pseudo, ip, bufferMessage);
            } else {
                printf("New message from %s : %s\n", ip, bufferMessage);
            }
        }
        memset(bufferMessage, 0, 256);
    }

    if (pseudo != NULL) {
        free(pseudo);
    }
    free(bufferMessage);
}

char *getPseudo(const char *bufferMessage, const char *ip) {
    int command_size = strlen(server_pseudo_command);

    if (!isspace(bufferMessage[command_size])) {
        printf("Command error (%s): no space between command and arguments\n", ip);
        return NULL;
    }
    int size = 0;
    while (!isspace(bufferMessage[command_size + 1 + size])
           && bufferMessage[command_size + 1 + size] != '\0'
           && size <= server_pseudo_max_size) {
        size++;
    }

    if (size > server_pseudo_max_size || size == 0) {
        printf("Command error (%s): wrong pseudo size (between 1 and %d)\n", ip, server_pseudo_max_size);
        return NULL;
    }

    char *pseudo = strndup(bufferMessage + command_size + 1, size);
    printf("(%s) registered as '%s'\n", ip, pseudo);
    return pseudo;
}