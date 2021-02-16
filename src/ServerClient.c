//
// Created by Clément on 03/02/2021.
//


#include "ServerClient.h"
void sendMsgClient(ServerClient *client , char *msg){
    if(send(client->clientSocketFd, msg, strlen(msg), 0) < 0)
    {
        displayLastSocketError("Error on send() for the client (%s)", inet_ntoa(client->clientSocketAddr.sin_addr));
        return;
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

int getNextByteNumber(ServerClient *client){
    int n = 0;

    for(int i = 0; i<FRAME_DATA_SIZE; i++){
        n |= ( ((uint8_t) (client->buffer[i]))<<((3-i)*8));
    }

    return n;
}

void setNewBufferSize(ServerClient *client, size_t size){
    char *tmp;

    while(size>client->allocatedBuffer) {
        client->allocatedBuffer *= 2;
    }

    tmp = realloc( client->buffer, client->allocatedBuffer);
    if(tmp != NULL){
        client->buffer = tmp;
    }else{
        perror("can't realloc client buffer: ");
    }

}

void *runServerClient(void *arg){
    int nextComingByteNumber = 0;
    int receivedByte;
    ServerClient *client = arg;
    memset(client->buffer, 0, client->allocatedBuffer);

    if( !getClientPseudo(client)){

        sendToAllFromClient(client->server, client, "connected");

        while (client->status == 0) {
            receivedByte = recv(client->clientSocketFd, client->buffer, FRAME_DATA_SIZE, 0);

            if(receivedByte <= 0){
                client->status = 1;
            }else{
                nextComingByteNumber = getNextByteNumber(client);

                setNewBufferSize(client, nextComingByteNumber);

                client->buffer[nextComingByteNumber] = '\0';

                receivedByte = recv(client->clientSocketFd, client->buffer, nextComingByteNumber, 0);

                if(receivedByte <= 0){
                    client->status = 1;
                }else {
                    printf("[%s]%s\n", client->name, client->buffer);
                    sendToAllFromClient(client->server, client, client->buffer);
                    memset(client->buffer, 0, client->allocatedBuffer);
                }
            }
        }
        printf("exit ServerCLient\n");
        removeClient(client->server, client);
        freeServerClient(client);
    }

    return NULL;
}


ServerClient *createServerClient(Server *server, SOCKET socketFd, struct sockaddr_in clientSocketAddr ){
    int pthreadError;
    ServerClient *serverClient = calloc(1, sizeof(ServerClient));
    serverClient->allocatedBuffer = CLIENT_INITIAL_BUFFER_SIZE;
    serverClient->buffer = calloc( serverClient->allocatedBuffer, sizeof(char));
    serverClient->clientSocketFd = socketFd;
    serverClient->clientSocketAddr = clientSocketAddr;
    serverClient->server = server;
    serverClient->status = 0;

    pthreadError = pthread_create(&(serverClient->pthread), NULL, &runServerClient, serverClient);
    if(pthreadError != 0){
        perror("pthread failed: ");
        free(serverClient);
        return  NULL;
    }

    return serverClient;
}

void ServerClientDisconnect(ServerClient *client){
    client->status = 1;

    printf("closing client connection %s(%s)\n", client->name, inet_ntoa(client->clientSocketAddr.sin_addr));
    if( shutdown(client->clientSocketFd, SHUT_RDWR) != 0){
        displayLastSocketError("error shutdown client: ");
    }
    if( CLOSE_SOCKET(client->clientSocketFd) != 0){
        displayLastSocketError("error close client: ");
    }
    printf("closing client thread\n");
    if( pthread_join(client->pthread, NULL) != 0){
        displayLastSocketError("error pthread_join client: ");
    }
    printf("closing client success\n");
}


void freeServerClient(ServerClient *client){
    if(client != NULL){
        if(client->name != NULL){
            free(client->name);
        }
        free(client);
    }
}