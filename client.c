//
// Created by Unknow on 14/01/2021.
//

#include <errno.h>
#include "client.h"

void sendServerPseudo(Client  *client, char *name){
    char res = 0;

    if(name == NULL){
        printf("enter your name:");
        memset(client->sendingBuffer, 0, 256);
        fgets(client->sendingBuffer, 255, stdin);
        client->sendingBuffer[ strlen(client->sendingBuffer)-1] = '\0';
        name = client->sendingBuffer;
    }

    while( res == CLIENT_PSEUDO_ALREADY_USE) {
        if (send(client->socket_fd, name, strlen(name) , 0) < 0) {
            displayLastSocketError("Error on send() for the client name");
            return;
        }
        if (recv(client->socket_fd, &res, 1, 0) < 0) {
            displayLastSocketError("Error on recv() for the client name");
            return;
        }

        if(res == CLIENT_PSEUDO_ALREADY_USE){
            printf("name already use enter your name:");
            memset(client->sendingBuffer, 0, 256);
            fgets(client->sendingBuffer, 255, stdin);
            client->sendingBuffer[ strlen(client->sendingBuffer)-1] = '\0';
            name = client->sendingBuffer;
        }
    }

}

Client *Client_create(char *address, int port, char *name){
    Client *client = malloc( sizeof(Client));

    client->socket_fd = createClientSocket(AF_INET, SOCK_STREAM, 0, port, address);

    if(client->socket_fd  == -1){
        return NULL;
    }
    client->receivingBuffer = calloc(256, sizeof(char));
    client->sendingBuffer = calloc(256, sizeof(char));
    sendServerPseudo(client, name);

    client->status = 0;
    return client;
}

void closeClient(Client *client){
    client->status = 1;

    if( shutdown(client->socket_fd, SHUT_RDWR) != 0){
        displayLastSocketError("error shutdown client");
    }
    if( close(client->socket_fd) != 0){
        displayLastSocketError("error close client");
    }
    printf("closing client thread\n");
    if( pthread_join(client->receivingThread, NULL) != 0){
        displayLastSocketError("error pthread_join receiving client");
    }
    if( pthread_join(client->sendingThread, NULL) != 0){
        displayLastSocketError("error pthread_join sending client");
    }
}

void *runReceivingClient(void *args){
    Client *client = args;

    while (client->status == 0) {
        memset(client->receivingBuffer, 0, 256);
        int n = recv(client->socket_fd, client->sendingBuffer, 256, 0);
        if (n == 0) {
            printf("Server disconnected !\n");
            client->status = 1;
        } else {
            printf("[recv]: %s", client->receivingBuffer);
        }
    }


}

void *runSendingClient(void *args){
    Client *client = args;
    char *tmp;

    while (client->status == 0) {
        memset(client->sendingBuffer, 0, 256);
        fgets(client->sendingBuffer, 255, stdin);

        tmp = strpbrk(client->sendingBuffer, "\r\n");
        *tmp = '\0';

        if(strcmp(client->sendingBuffer, "\\exit") == 0){
            closeClient(client);
        }else{
            int n = send(client->socket_fd, client->sendingBuffer, 256, 0);
            if (n == 0) {
                printf("Server disconnected !\n");
                client->status = 1;
            } else {
                //printf("[you]: %s", client->sendingBuffer);
            }
        }

    }

}



void startClient(Client *client){
    int pthreadError;

    pthreadError = pthread_create(&(client->receivingThread), NULL, &runReceivingClient, client);
    if(pthreadError != 0){
        perror("Error creating the receiving thread: ");
        return;
    }

    pthreadError = pthread_create(&(client->sendingThread), NULL, &runSendingClient, client);
    if(pthreadError != 0){
        perror("Error creating the sending thread: ");
        return;
    }

    printf("Client started success\n");
}

int main(int argc, char **argv) {
    Client *client;
    if (argc < 3) {
        printf("You need to provide ip address and port n° and name\n");
        return EXIT_FAILURE;
    }

    initSocket();

    char *destAddress = argv[1];
    int port = atoi(argv[2]);

    if(argc >= 4) {
        client = Client_create(destAddress, port, argv[3]);
    }else{
        client = Client_create(destAddress, port, NULL);
    }
    if(client != NULL){
        startClient(client);
    }
    pthread_join(client->receivingThread, NULL);

    endSocket();
    return EXIT_SUCCESS;
}
