cmake_minimum_required(VERSION 3.16)
project(chat_in_c C)

set(CMAKE_C_STANDARD 11)

add_executable(server src/server.c src/mySocket.h src/mySocket.c src/ServerClient.c src/ServerClient.h src/server.h src/ServerClientCommon.h)
add_executable(client src/client.c src/mySocket.h src/mySocket.c src/client.h src/ServerClientCommon.h)

if(WIN32)
    target_link_libraries(server Ws2_32 pthread)
    target_link_libraries(client Ws2_32 pthread)
else()
    target_link_libraries(server pthread)
    target_link_libraries(client pthread)
endif()