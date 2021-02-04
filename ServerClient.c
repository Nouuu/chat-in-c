//
// Created by Clément on 03/02/2021.
//


#include "ServerClient.h"


int getClientPseudo(ServerClient *client){
    ServerClient *res;
    char code = 0;
    char name[256] = {0};
    printf("waiting for client name %s\n", inet_ntoa(client->clientSocketAddr.sin_addr));

    do{
        if( recv(client->clientSocketFd, name, 255, 0) <= 0 ) {
            displayLastSocketError("Error on recv() for the client name (%s)", inet_ntoa(client->clientSocketAddr.sin_addr));
            return 1;
        }
        res = getServerClient(client->server, name);
        if(res != NULL){
            if(send(client->clientSocketFd, &code, 1, 0) < 0)
            {
                displayLastSocketError("Error on send() for the client name");
                return 2;
            }
        }
    }while(res != NULL);

    code = 1;
    if(send(client->clientSocketFd, &code, 1, 0) < 0)
    {
        displayLastSocketError("Error on send() for the client name");
        return 2;
    }
    client->name = strdup(name);
    printf("client (%s) running with name %s \n",inet_ntoa(client->clientSocketAddr.sin_addr), client->name);

    return 0;
}

void *runServerClient(void *arg){
    char bufferMessage[256] = {0};
    ServerClient *client = arg;
    memset(client->buffer, 0, 256);

    if( !getClientPseudo(client)){

        while (recv(client->clientSocketFd, bufferMessage, 255, 0) > 0 && client->status == 1) {

            if (!strcmp(bufferMessage, "0\r\n") || !strcmp(bufferMessage, "0\n")) {
                break;
            }
            printf("New message from %s : %s\n", client->name, bufferMessage);
            memset(bufferMessage, 0, 256);
        }

    }else{

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