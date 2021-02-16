//
// Created by Clément on 04/02/2021.
//

#ifndef CHAT_IN_C_SERVERCLIENTCOMMON_H
#define CHAT_IN_C_SERVERCLIENTCOMMON_H

#define CLIENT_PSEUDO_ALREADY_USE 0
#define CLIENT_PSEUDO_VALIDATED 1
#define CLIENT_SERVER_CLOSED 2

#define FRAME_DATA_SIZE 4
#define FRAME_DATA_START FRAME_DATA_SIZE

#define COMMAND_CHAR '\\'

#define CLIENT_INITIAL_BUFFER_SIZE 32

#if( CLIENT_INITIAL_BUFFER_SIZE <= FRAME_DATA_START)
#error "client initial buffer cant be <= FRAME_DATA_START"
#endif

typedef enum FrameType{UNDEFINED, COMMAND, INTERNAL_COMMAND, MESSAGE, SERVER_INFO}FrameType;

#ifdef WIN32 /* si vous êtes sous Windows */

#define STD_INPUT_FD (SOCKET)(*GetStdHandle(stdin))

#elif defined (linux) /* si vous êtes sous Linux */

#define STD_INPUT_FD STDIN_FILENO

#endif

#endif //CHAT_IN_C_SERVERCLIENTCOMMON_H
