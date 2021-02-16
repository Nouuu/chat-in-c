//
// Created by Unknow on 14/01/2021.
//

#include "client.h"

void sendServerPseudo(Client  *client, const char *name){
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

Client *Client_create(const char *address, int port, const char *name){
    Client *client = malloc( sizeof(Client));

    client->socket_fd = createClientSocket(AF_INET, SOCK_STREAM, 0, port, address);

    if(client->socket_fd  == -1){
        return NULL;
    }
    client->receivingBufferSize = 64;
    client->receivingBuffer = calloc(client->receivingBufferSize, sizeof(char));

    client->sendingBufferSize = CLIENT_INITIAL_BUFFER_SIZE;
    client->sendingBuffer = calloc(client->sendingBufferSize, sizeof(char));

    sendServerPseudo(client, name);

    client->status = 0;
    printf("client created\n");
    return client;
}

void closeClient(Client *client){
    if(client->status == 0){
        client->status = 1;
    }else{ return; }

    printf("closing client\n");

    if( shutdown(client->socket_fd, SHUT_RDWR) != 0){
        displayLastSocketError("error shutdown client: ");
    }
    if( CLOSE_SOCKET(client->socket_fd) != 0){
        displayLastSocketError("error close client socket: ");
    }
    printf("closing client thread\n");
    if( client->receivingThread != pthread_self() && pthread_join(client->receivingThread, NULL) != 0){
        displayLastSocketError("error pthread_join receiving client: ");
    }
    if( client->sendingThread != pthread_self() && pthread_cancel(client->sendingThread) != 0){
        displayLastSocketError("error pthread_join sending client: ");
    }
    printf("closing client success\n");
}

void *runReceivingClient(void *args){
    Client *client = args;

    while (client->status == 0) {
        memset(client->receivingBuffer, 0, client->receivingBufferSize);
        int n = recv(client->socket_fd, client->receivingBuffer, client->receivingBufferSize, 0);
        if (n == 0) {
            printf("Server disconnected !\n");
            client->status = 1;
             // to close the fgets on the sending thread
        } else {
            printf("%s\n", client->receivingBuffer);
        }
    }
    return args;
}

void getLine(Client *client){
    int buffer;
    size_t i = FRAME_DATA_START;
    char *newSendingBuffer = NULL;

    while(1){
        buffer = getc(stdin);
        client->sendingBuffer[i] = (char)buffer;
        i += 1;

        if(buffer == EOF || buffer == '\n'){
            break;
        }else if(i == client->sendingBufferSize){
            client->sendingBufferSize *= 2;

            newSendingBuffer = realloc(client->sendingBuffer, client->sendingBufferSize*sizeof(char));

            if(newSendingBuffer == NULL){
                perror("can't realloc\n: ");
            }else{
                client->sendingBuffer = newSendingBuffer;
            }
        }
    }
}

FrameType getMessageType(const char *msg){
    if(msg == NULL){
        return UNDEFINED;
    }else if(msg[0] == COMMAND_CHAR && msg[1] == COMMAND_CHAR){ // if the string was only 1 long the msg[0] should not be equal to '\\' so no need to check
            return  INTERNAL_COMMAND;
    }else{
        return MESSAGE;
    }
}

void removeLastBackLine(char *string){
    char *tmp = strpbrk(string, "\r\n");
    if(tmp != NULL) {
        *tmp = '\0';
    }
}

char *getClientMessage(Client *client){
    return client->sendingBuffer + FRAME_DATA_START;
}

void executeClientInternalCommand(Client *client){
    char *command = getClientMessage(client) +2; // +2 for the \\

    if(strcmp(command, "exit") == 0){
        closeClient(client);
    }else{
        printf("unknown command \"%s\" \n", command);
    }
}

int clientSendMessage(Client *client, const char *message){
    int n = 0;
    size_t size;
    char buffer[FRAME_DATA_START] = {0};

    if(message == NULL || client == NULL){ return 0; }

    size = strlen(message);

    if(size >= 1) {
        size += FRAME_DATA_START;

        //split the size in the first 4 byte of the frame
        for(int i = 0; i<FRAME_DATA_START; i++ ){
            buffer[3-i] = (char)((size >> (i*8))&0xFF);
        }

        n = send(client->socket_fd, buffer, FRAME_DATA_START, 0);
        if (n == 0) {
            printf("Server disconnected !\n");
            client->status = CLIENT_DISCONNECTED;
            return -1;
        }

        n = send(client->socket_fd, message, size, 0);
        if (n == 0) {
            printf("Server disconnected !\n");
            client->status = CLIENT_DISCONNECTED;
            return -1;
        }
    }
    return n;
}

void sendMessage(Client *client){
    FrameType messageType;
    char *message = getClientMessage(client);
    removeLastBackLine( message );
    messageType = getMessageType( message );

    switch (messageType) {
        case INTERNAL_COMMAND: executeClientInternalCommand(client);
            break;
        case MESSAGE:
            clientSendMessage(client, message);
            break;
        default:break;
    }
}

void *runSendingClient(void *args){
    Client *client = args;

    while (client->status == 0) {
        memset(client->sendingBuffer, 0, client->sendingBufferSize);
        getLine(client);
        sendMessage(client);
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
}

void freeClient(Client *client){
    if(client != NULL){
        if(client->sendingBuffer != NULL){
            free(client->sendingBuffer);
        }

        if(client->receivingBuffer != NULL){
            free(client->receivingBuffer);
        }

        free(client);

    }
}
