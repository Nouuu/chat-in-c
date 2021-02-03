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
} arg_struct;

typedef struct user_list_struct {
    int size;
    char *userlist[10];
    pthread_mutex_t mutex;
} user_list_struct;

const char *server_exit_command = "/exit";
const char *server_pseudo_command = "/pseudo";
const char *server_list_users_command = "/list";
const int server_pseudo_max_size = 10;

static user_list_struct user_list = {
        .size=0,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
};

int serverEngine(int port);

int initServerSocket(int port, struct sockaddr_in *clientSocket, int *socketfd, const char *ip);

void serverListenLoop(int serverFD);

void *clientConnectionHandler(void *args);

void clientListenLoop(int clientFD, char *bufferMessage, char *ip);

char *getPseudo(const char *bufferMessage, const char *ip);

char *listUsers();

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
    if (initServerSocket(port, &serverSocket, &serverFD, NULL) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    listen(serverFD, 40);

    printf("Server on and listen on %d port\n", port);

    serverListenLoop(serverFD);

    close(serverFD);
    return EXIT_SUCCESS;
}

int initServerSocket(int port, struct sockaddr_in *clientSocket, int *socketfd, const char *ip) {
    clientSocket->sin_family = AF_INET;
    clientSocket->sin_port = htons(port);
    clientSocket->sin_addr.s_addr = INADDR_ANY;

    *socketfd = socket(AF_INET, SOCK_STREAM, 0);

    int result = bind(*socketfd, (struct sockaddr *) clientSocket, sizeof(*clientSocket));
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

            perror("could not create thread, exit program\n");
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
        } else if (strstr(bufferMessage, server_list_users_command) == bufferMessage) {
            char *user_list_str = listUsers();
            send(clientFD, user_list_str, 2048, 0);
            free(user_list_str);
        } else {
            if (!pseudo) {
                printf("New message from %s : %s\n", ip, bufferMessage);
            } else {
                printf("New message from %s (%s) : %s\n", pseudo, ip, bufferMessage);
            }
        }
        memset(bufferMessage, 0, 256);
    }
    if (pseudo) {
        free(pseudo);
    }
}

char *getPseudo(const char *bufferMessage, const char *ip) {
    if (!isspace(bufferMessage[strlen(server_pseudo_command)])) {
        printf("Command error (%s): no space between command and arguments\n", ip);
        return NULL;
    }
    int command_size = (int) strlen(server_pseudo_command);
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

    pthread_mutex_lock(&user_list.mutex);

    user_list.userlist[user_list.size] = strdup(pseudo);
    user_list.size++;

    pthread_mutex_unlock(&user_list.mutex);

    return pseudo;
}

char *listUsers() {
    char *str = calloc(2049, sizeof(char));

    pthread_mutex_lock(&user_list.mutex);

    sprintf(str, "User registered :\n");
    for (int i = 0; i < user_list.size; ++i) {
        strcat(str, "- ");
        strcat(str, user_list.userlist[i]);
        strcat(str, "\n");
    }

    pthread_mutex_unlock(&user_list.mutex);

    printf("%s\n", str);
    return str;
}