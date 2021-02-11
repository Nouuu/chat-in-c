//
// Created by Clément on 03/02/2021.
//


#include "ServerClient.h"
void sendMsgClient(ServerClient *client , char *msg){
    if(send(client->clientSocketFd, msg, strlen(msg), 0) < 0)
    {
        displayLastSocketError("Error on send() for the client (%s)", inet_ntoa(client->clientSocketAddr.sin_addr));
        return ;
    }
}

int getClientPseudo(ServerClient *client){
    ServerClient *res;
    char code = CLIENT_PSEUDO_ALREADY_USE;
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
                displayLastSocketError("Error on send() for the client name (%S)", inet_ntoa(client->clientSocketAddr.sin_addr));
                return 2;
            }
        }
    }while(res != NULL);

    code = CLIENT_PSEUDO_VALIDATED;
    if(send(client->clientSocketFd, &code, 1, 0) < 0)
    {
        displayLastSocketError("Error on send() for the client name (%s)", inet_ntoa(client->clientSocketAddr.sin_addr));
        return 2;
    }
    client->name = strdup(name);
    printf("client (%s) running with name %s \n",inet_ntoa(client->clientSocketAddr.sin_addr), client->name);

    return 0;
}

void *runServerClient(void *arg){
    int n;
    char bufferMessage[256] = {0};
    ServerClient *client = arg;
    memset(client->buffer, 0, 256);

    if( !getClientPseudo(client)){

        sendToAll(client->server, client, "connected");

        while (client->status == 0) {
            n = recv(client->clientSocketFd, bufferMessage, 255, 0);
            if(n <= 0){
                client->status = 1;
            }else{
                printf("[%s]%s\n", client->name, bufferMessage);
                sendToAll(client->server, client, bufferMessage);
                memset(bufferMessage, 0, 256);
            }
        }

        sendToAll(client->server, client, "disconnected");
        shutdown(client->clientSocketFd, SHUT_RDWR);
        CLOSE_SOCKET(client->clientSocketFd);
        removeClient(client->server, client);
        freeServerClient(client);
    }

    return NULL;
}


ServerClient *createServerClient(Server *server){
    int pthreadError;
    ServerClient *serverClient = calloc(1, sizeof(ServerClient));
    serverClient->clientSocketFd = accept(server->serverSocketFd, (struct sockaddr *) &(serverClient->clientSocketAddr), &server->clientSocketSize);
    serverClient->server = server;

    if(serverClient->clientSocketFd == -1){
        if(errno != EINVAL) {
            displayLastSocketError("Error createServerClient on accept (%llu): ", serverClient->clientSocketFd);
        }
        free(serverClient);
        return  NULL;
    }

    serverClient->status = 0;

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