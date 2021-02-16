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

void setServerBufferSize(Server *server, size_t size){
    int mutexError;
    char *newBuffer;

    mutexError = pthread_mutex_lock(&server->mutexClientList);
    if(mutexError != 0){
        fprintf(stderr, "setNewBufferSize mutex (%p) lock error: %d \n", &server->mutexClientList, mutexError);
    }

    while(size>server->messageBufferSize){
        server->messageBufferSize *= 2;
    }

    newBuffer = realloc( server->messageBuffer, server->messageBufferSize);
    if(newBuffer == NULL){
        perror("can't realloc server message buffer: ");
    }else{
        server->messageBuffer = newBuffer;
    }

    mutexError = pthread_mutex_unlock(&server->mutexClientList);
    if(mutexError != 0){
        fprintf(stderr, "setNewBufferSize mutex (%p) unlock error: %d \n", &server->mutexClientList, mutexError);
    }
}

void sendToAll(Server *server, char *senderName, char *msg) {
    size_t size;
    int mutexError;

    size = snprintf(NULL, 0, "[%s]: %s", senderName, msg)+1;
    setServerBufferSize(server,size);

    snprintf(server->messageBuffer, size, "[%s]: %s", senderName, msg );

    mutexError = pthread_mutex_lock(&server->mutexClientList);
    if(mutexError != 0){
        fprintf(stderr, "sendToAll mutex (%p) lock error: %d \n", &server->mutexClientList, mutexError);
    }

    for(int i = 0; i<server->size; i++){
        if( server->clients[i]->name != NULL && strcmp(senderName, server->clients[i]->name) != 0) {
            sendMsgClient(server->clients[i], server->messageBuffer);
        }
    }

    mutexError = pthread_mutex_unlock(&server->mutexClientList);
    if(mutexError != 0){
        fprintf(stderr, "sendToAll mutex (%p) unlock error: %d \n", &server->mutexClientList, mutexError);
    }

}

void sendToAllFromClient(Server *server, ServerClient  *client, char *msg){
    sendToAll(server, client->name, msg);
}

void sendToAllFromServer(Server *server, char *msg){
    sendToAll(server, "Server", msg);
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

    sendToAllFromClient(server, client, "disconnected");
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

    server->messageBufferSize = CLIENT_INITIAL_BUFFER_SIZE;
    server->messageBuffer = calloc( server->messageBufferSize, sizeof(char));

    server->serverThread = 0;
    server->commandThread = 0;

    server->status = 0;
    server->size = 0;

    pthread_mutex_init(&server->mutexClientList, NULL);
    printf("Server on and listen on %d port\n", port);
    return server;
}

void ServerAddClient(Server *server,  ServerClient *serverClient){
    int mutexError;

    mutexError = pthread_mutex_lock(&server->mutexClientList);
    if(mutexError != 0){
        fprintf(stderr, "ServerAddClient mutex (%p) lock error: %d \n", &server->mutexClientList, mutexError);
    }

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
        clientSocketFd = -1;
        clientSocketFd = accept(server->serverSocketFd, (struct sockaddr *) &(clientSocketAddr), &size);
        if(clientSocketFd != -1){
            ServerAddClient(server, createServerClient(server, clientSocketFd, clientSocketAddr));
        }else{
            #ifdef WIN32
                if(WSAGetLastError() != WSAEINTR) { // WSAEINTR (10004) Interrupted function call.
                                                    //                  A blocking operation was interrupted by a call to
                                                    // trigered when we call void closeServer(Server *server)
                    displayLastSocketError("Error runServer on accept (%llu): ", clientSocketFd);
                }
            #else
                if(errno != EINVAL) {
                    displayLastSocketError("Error runServer on accept (%llu): ", clientSocketFd);
                }
            #endif


            server->status = 1;
        }
    }
    return server;
}

void closeServer(Server *server){
    int pthreadError;

   // pthread_mutex_lock(&server->mutexClientList);

    if(server->status == 0){
        server->status = 1;
    }else{
        return;
    }

    printf("closing server with %lu connection left\n", server->size);
    sendToAllFromServer(server, "The Server is closing");


    while(server->size){
        ServerClientDisconnect(server->clients[0]);
    }

    #ifdef linux
        if( shutdown( server->serverSocketFd, SHUT_RDWR) != 0){
           displayLastSocketError("error shutdown server: ");
        }
    #endif

    CLOSE_SOCKET(server->serverSocketFd);

    if(server->serverThread != 0) {
        pthreadError = pthread_join(server->serverThread, NULL);
        if (pthreadError != 0) {
            perror("error closing serverThread: ");
            return;
        }
    }

    if(server->commandThread != 0 && server->commandThread != pthread_self()){
        printf("closing server command thread\n");
        pthreadError = pthread_join(server->commandThread, NULL);
        if(pthreadError != 0){
            perror("error closing command Thread: ");
            return;
        }
    }

  //  pthread_mutex_unlock(&server->mutexClientList);
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
    return server;
}

void startReceivingServer(Server *server){
    int pthreadError;

    pthreadError = pthread_create(&(server->serverThread), NULL, &runServer, server);
    if(pthreadError != 0){
        printf("error %d\n", pthreadError);
        perror("pthread failed server command: ");
        return ;
    }
}


void startServer(Server *server){
    int pthreadError;

    pthreadError = pthread_create(&(server->commandThread), NULL, &runServerCommand, server);
    if(pthreadError != 0){
        perror("pthread failed server command: ");
        return ;
    }

    startReceivingServer(server);
}

void waitForServer(Server *server){
   /* if( pthread_join(server->commandThread, NULL) != 0){
        perror("error waiting for server: ");
    }*/
    server->status = 1;
    printf("waiting\n");
    if( pthread_join(server->serverThread, NULL) != 0){
        perror("error waiting for server: ");
    }
    printf("waiting end\n");
}

void freeServer(Server *server){
    if(server != NULL){

        if(server->messageBuffer != NULL){
            free(server->messageBuffer);
        }

        if(server->clients != NULL){
            free(server->clients);
        }

        free(server);
    }
}