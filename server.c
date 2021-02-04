//
// Created by Unknow on 14/01/2021.
//

#include "server.h"

void removeClient(Server *server, struct ServerClient *client){

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

    server->clientSocketSize = sizeof( struct  sockaddr_in);
    server->status = 1;
    server->size = 0;
    printf("Server on and listen on %d port\n", port);
    return server;
}

void ServerAddClient(Server *server,  ServerClient *serverClient){

    if(server == NULL || serverClient == NULL || server->size == server->capacityMax){
        return;
    }

    server->clients[server->size] = serverClient;
    server->size += 1;

    printf("client connected with ip: %s \n%lu/%lu connection\n",inet_ntoa(serverClient->clientSocketAddr.sin_addr), server->size, server->capacityMax);
}

void runServer(Server *server){
    fd_set fdSet;
    int fdMax = server->serverSocketFd;;

    while(server->status == 1){
        FD_ZERO(&fdSet);
        FD_SET(STDIN_FILENO, &fdSet);
        FD_SET(server->serverSocketFd, &fdSet);

        if(select(fdMax + 1, &fdSet, NULL, NULL, NULL) == -1){
            perror("select()");
            exit(errno);
        }

        if(FD_ISSET(STDIN_FILENO, &fdSet)){
            /* stop process when type on keyboard */
            server->status = 0;
        }else if(FD_ISSET(server->serverSocketFd, &fdSet)) {
            ServerClient *serverClient = createServerClient(server);
            ServerAddClient(server, serverClient);
        }

    }

}

void closeServer(Server *server){
    printf("closing server with %lu connection left\n", server->size);
    while(server->size){
        server->clients[0]->status = 0;
        printf("closing client connection %s(%s)\n", server->clients[0]->name, inet_ntoa(server->clients[0]->clientSocketAddr.sin_addr));
        shutdown(server->clients[0]->clientSocketFd, SHUT_RDWR);
        close(server->clients[0]->clientSocketFd);
        printf("closing client thread\n");
        pthread_join(server->clients[0]->pthread, NULL);
        printf("closing client success\n");
    }
    close(server->serverSocketFd);
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
        runServer(server);
        closeServer(server);
    }

    endSocket();
    return EXIT_SUCCESS;
}