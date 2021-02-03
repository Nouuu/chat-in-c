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

typedef struct arg_struct {
    int *socketfd;
    const char *ipaddress;
}arg_struct;

const char *server_exit_command = "/exit";
const char *server_pseudo_command = "/pseudo";
const int server_pseudo_max_size = 10;

int serverEngine(int port);

int initServerSocket(int port, struct sockaddr_in *serverSocket, int *socketfd);

void serverListenLoop(int serverFD);

void *clientConnectionHandler(void *args);

void clientListenLoop(int clientFD, char *bufferMessage, char *ip);

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
    char *pseudo = NULL;
    while (recv(clientFD, bufferMessage, 255, 0) > 0) {

        if (strstr(bufferMessage, server_exit_command) == bufferMessage) {
            break;
        } else if (strstr(bufferMessage, server_pseudo_command) == bufferMessage) {
            pseudo = getPseudo(bufferMessage, ip);
        } else {
            if (!pseudo) {
                printf("New message from %s : %s\n", ip, bufferMessage);
            } else {
                printf("New message from %s (%s) : %s\n", pseudo, ip, bufferMessage);
            }
        }
        memset(bufferMessage, 0, 256);
    }
}

char *getPseudo(const char *bufferMessage, const char *ip) {
    if (!isspace(bufferMessage[strlen(server_pseudo_command)])) {
        printf("Command error (%s): no space between command and arguments\n", ip);
        return NULL;
    }
    int command_size = strlen(server_pseudo_command);
    int size = 0;
    while (!isspace(bufferMessage[command_size + 1 + size])
           && bufferMessage[command_size + 1 + size] != '\0'
           && size <= server_pseudo_max_size) {
        size++;
    }
    if (size > server_pseudo_max_size) {
        printf("Command error (%s): pseudo size above %d characters\n", ip, server_pseudo_max_size);
        return NULL;
    }
    char *pseudo = strndup(bufferMessage + command_size + 1, size);
    printf("(%s) registered as '%s'\n", ip, pseudo);
    return pseudo;
}