//
// Created by Clément on 04/02/2021.
//

#ifndef CHAT_IN_C_SERVERCLIENTCOMMON_H
#define CHAT_IN_C_SERVERCLIENTCOMMON_H

#define CLIENT_PSEUDO_ALREADY_USE 0
#define CLIENT_PSEUDO_VALIDATED 1
#define CLIENT_SERVER_CLOSED 2

#ifdef WIN32 /* si vous êtes sous Windows */

#define STD_INPUT_FD (SOCKET)(*GetStdHandle(stdin))

#elif defined (linux) /* si vous êtes sous Linux */

#define STD_INPUT_FD STDIN_FILENO

#endif

#endif //CHAT_IN_C_SERVERCLIENTCOMMON_H
