//
// Created by Cleme on 16/02/2021.
//

#include "server.h"
#include "client.h"

int main(int arg, char **argv) {
    Server *server = CreateServer(6666, 10);
    startReceivingServer(server);
    Client *client = Client_create("127.0.0.1", 6666, "test");

    clientSendMessage(client, "hello");

    closeServer(server);

    return 1;
}
