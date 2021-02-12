//
// Created by Unknow on 14/01/2021.
//

#include "server.h"

ServerClient *getServerClient(Server *server, char *name){
    for(int i = 0; i<server->size; i++){
        if(server->clients[i]->name != NULL && !strcmp(server->clients[i]->name, name) ){
            return server->clients[i];
        }
    }
    return NULL;
}

void sendToAll(Server *server, ServerClient  *client, char *msg){
    size_t size;
    char *buffer;
    int mutexError;

    size = snprintf(NULL, 0, "[%s]: %s", client->name, msg);
    buffer = calloc(size+1, sizeof(char ));
    sprintf(buffer, "[%s]: %s", client->name, msg );

    mutexError = pthread_mutex_lock(&server->mutexClientList);
    if(mutexError != 0){
        fprintf(stderr, "sendToAll mutex (%p) lock error: %d \n", &server->mutexClientList, mutexError);
    }

    for(int i = 0; i<server->size; i++){
        if( server->clients[i] != client && server->clients[i]->name != NULL) {
            sendMsgClient(server->clients[i], buffer);
        }
    }

    mutexError = pthread_mutex_unlock(&server->mutexClientList);
    if(mutexError != 0){
        fprintf(stderr, "sendToAll mutex (%p) unlock error: %d \n", &server->mutexClientList, mutexError);
    }
    free(buffer);
}

void removeClient(Server *server, struct ServerClient *client){
    int mutexError;

    mutexError = pthread_mutex_lock(&server->mutexClientList);
    if(mutexError != 0){
        fprintf(stderr, "removeClient mutex (%p) lock error: %d \n", &server->mutexClientList, mutexError);
    }

    if(server == NULL || client == NULL){
        return;
    }

    for(int i = 0; i<server->size; i++){
        if(server->clients[i] == client){

            for(int j = i;j<server->size-1; j++ ){
                server->clients[j] = server->clients[j+1];
            }
            break;
        }
    }
    server->size -= 1;
    mutexError = pthread_mutex_unlock(&server->mutexClientList);
    if(mutexError != 0){
        fprintf(stderr, "removeClient mutex (%p) unlock error: %d \n", &server->mutexClientList, mutexError);
    }

    sendToAll(server, client, "disconnected");
    shutdown(client->clientSocketFd, SHUT_RDWR);
    CLOSE_SOCKET(client->clientSocketFd);
    printf("client \"%s\" (%s) left\n%lu/%lu connection\n", client->name, inet_ntoa(client->clientSocketAddr.sin_addr), server->size, server->capacityMax );
}

Server* CreateServer(int port, int capacity){
    Server *server = calloc(1, sizeof(Server) );

    initSocket();

    server->serverSocketFd = createServerSocket(AF_INET, SOCK_STREAM, 0, port);
    if(server->serverSocketFd == -1){
        return NULL;
    }

    server->capacityMax = capacity;
    if( listen(server->serverSocketFd, capacity) == -1){
        displayLastSocketError("Error on listen() server socket");
        return NULL;
    }

    server->clients = calloc(capacity, sizeof( ServerClient *) );

    server->status = 0;
    server->size = 0;

    pthread_mutex_init(&server->mutexClientList, NULL);
    printf("Server on and listen on %d port\n", port);
    return server;
}

void ServerAddClient(Server *server,  ServerClient *serverClient){
    int mutexError;
    printf("ServerAddClient test\n");
    mutexError = pthread_mutex_lock(&server->mutexClientList);
    if(mutexError != 0){
        fprintf(stderr, "ServerAddClient mutex (%p) lock error: %d \n", &server->mutexClientList, mutexError);
    }
    printf("ServerAddClient test2\n");

    if(server == NULL || serverClient == NULL || server->size == server->capacityMax){
        return;
    }

    server->clients[server->size] = serverClient;
    server->size += 1;

    mutexError = pthread_mutex_unlock(&server->mutexClientList);
    if(mutexError != 0){
        fprintf(stderr, "ServerAddClient mutex (%p) unlock error: %d \n", &server->mutexClientList, mutexError);
    }
    printf("client connected with ip: %s \n%lu/%lu connection\n",inet_ntoa(serverClient->clientSocketAddr.sin_addr), server->size, server->capacityMax);
}

void *runServer(void *args){
    Server *server = args;
    SOCKET clientSocketFd = -1;
    struct sockaddr_in clientSocketAddr;
    socklen_t size = clientSocketSize;

    while(server->status == 0){

        clientSocketFd = accept(server->serverSocketFd, (struct sockaddr *) &(clientSocketAddr), &size);
        if(clientSocketFd != -1){
            ServerAddClient(server, createServerClient(server, clientSocketFd, clientSocketAddr));
        }else{
            if(errno != EINVAL) {
                displayLastSocketError("Error runServer on accept (%llu): ", clientSocketFd);
            }
            server->status = 1;
        }
    }

}

void closeServer(Server *server){
    int pthreadError;

    //pthread_mutex_lock(&server->mutexClientList);

    if(server->status == 0){
        server->status = 1;
    }else{
        return;
    }

    printf("closing server with %lu connection left\n", server->size);

    if( shutdown(server->serverSocketFd, SHUT_RDWR) != 0){
        displayLastSocketError("error shutdown server: ");
    }

    while(server->size){
        ServerClientDisconnect(server->clients[0]);
    }

    pthreadError = pthread_join(server->serverThread, NULL);
    if(pthreadError != 0){
        perror("error closing serverThread: ");
    }

    if(server->commandThread != pthread_self()){
        printf("closing server command thread\n");
        pthreadError = pthread_join(server->commandThread, NULL);
        if(pthreadError != 0){
            perror("error closing command Thread: ");
        }
    }

    CLOSE_SOCKET(server->serverSocketFd);
   //pthread_mutex_unlock(&server->mutexClientList);
    printf("server close success\n");
}

void *runServerCommand(void *args){
    Server *server = args;
    char *tmp;

    while (server->status == 0) {
        memset(server->commandBuffer, 0, 256);
        fgets(server->commandBuffer, 255, stdin);

        tmp = strpbrk(server->commandBuffer, "\r\n");
        *tmp = '\0';

        if(strcmp(server->commandBuffer, "\\exit") == 0){
            closeServer(server);
        }else{
            printf("unknown command \"%s\"\n", server->commandBuffer);
        }
    }

}

void startServer(Server *server){
    int pthreadError;

    pthreadError = pthread_create(&(server->commandThread), NULL, &runServerCommand, server);
    if(pthreadError != 0){
        perror("pthread failed server command: ");
        return ;
    }

    pthreadError = pthread_create(&(server->serverThread), NULL, &runServer, server);
    if(pthreadError != 0){
        perror("pthread failed server command: ");
        return ;
    }

}

void waitForServer(Server *server){
    if( pthread_join(server->commandThread, NULL) != 0){
        perror("error waiting for server: ");
    }
}

int main(int arg, char **argv) {
    Server *server;
    int port;

    if (arg != 2) {
        printf("You need to provide port n°\n");
        return EXIT_FAILURE;
    }

    initSocket();

    port = atoi(argv[1]);

    server = CreateServer(port, 40);

    if(server != NULL){
        startServer(server);
        waitForServer(server);
        closeServer(server);
    }

    endSocket();
    return EXIT_SUCCESS;
}