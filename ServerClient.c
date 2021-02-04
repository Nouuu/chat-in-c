//
// Created by Clément on 03/02/2021.
//


#include "ServerClient.h"

void *runServerClient(void *arg){
    char bufferMessage[256] = {0};
    ServerClient *client = arg;
    memset(client->buffer, 0, 256);

    printf("waiting for client name %s\n", inet_ntoa(client->clientSocketAddr.sin_addr));
    if( recv(client->clientSocketFd, client->buffer, 255, 0) <= 0 ){
        displayLastSocketError("Error on recv() for the client name");
    }else{

        client->name = strdup(client->buffer);
        printf("client (%s) running with name %s \n",inet_ntoa(client->clientSocketAddr.sin_addr), client->name);

        while (recv(client->clientSocketFd, bufferMessage, 255, 0) > 0 && client->status == 1) {

            if (!strcmp(bufferMessage, "0\r\n") || !strcmp(bufferMessage, "0\n")) {
                break;
            }
            printf("New message from %s : %s\n", client->name, bufferMessage);
            memset(bufferMessage, 0, 256);
        }

    }

    close(client->clientSocketFd);
    removeClient(client->server, client);
    freeServerClient(client);

    return NULL;
}


ServerClient *createServerClient(Server *server){
    char bufferMessage[256] = {0};
    int pthreadError;
    ServerClient *serverClient = calloc(1, sizeof(ServerClient));
    serverClient->clientSocketFd = accept(server->serverSocketFd, (struct sockaddr *) &(serverClient->clientSocketAddr), &server->clientSocketSize);
    serverClient->server = server;

    if(serverClient->clientSocketFd == -1){
        displayLastSocketError("Error on accept()");
        free(serverClient);
        return  NULL;
    }

    serverClient->status = 1;

    pthreadError = pthread_create(&(serverClient->pthread), NULL, &runServerClient, serverClient);
    if(pthreadError != 0){
        perror("pthread failed: ");
        free(serverClient);
        return  NULL;
    }

   // pthread_join(serverClient->pthread, NULL);
    return serverClient;
}


void freeServerClient(ServerClient *client){
    if(client != NULL){
        if(client->name != NULL){
            free(client->name);
        }
        free(client);
    }

}