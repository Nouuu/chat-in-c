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

struct sockaddr_in client;
int sockedfd;
char bufferMessage[256] = {0};

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("You need to provide ip address and port n°\n");
        return EXIT_FAILURE;
    }
    const char *destAddress = argv[1];
    int port = atoi(argv[2]);

    client.sin_family = AF_INET;
    client.sin_port = htons(port);

    inet_pton(AF_INET, destAddress, &client.sin_addr);

    sockedfd = socket(AF_INET, SOCK_STREAM, 0);

    int result = connect(sockedfd, (struct sockaddr *) &client, sizeof(client));

    if (result == -1) {
        printf("Error when connect socket!\n");
        perror("");
        return EXIT_FAILURE;
    }

    while (1) {
        printf("Enter a message (0 to exit) :\n");
        fgets(bufferMessage, 255, stdin);
        if (!strcmp(bufferMessage, "0\r\n") || !strcmp(bufferMessage, "0\n")) {
            break;
        }
        send(sockedfd, bufferMessage, 255, 0);
    }
    close(sockedfd);
    return EXIT_SUCCESS;
}
