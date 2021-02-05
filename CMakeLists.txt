cmake_minimum_required(VERSION 3.16)
project(chat_in_c C)

set(CMAKE_C_STANDARD 11)

add_executable(server server.c mySocket.h mySocket.c ServerClient.c ServerClient.h server.h ServerClientCommon.h)
add_executable(client client.c mySocket.h mySocket.c client.h ServerClientCommon.h)

if(WIN32)
    target_link_libraries(server Ws2_32 pthread)
    target_link_libraries(client Ws2_32 pthread)
else()
    target_link_libraries(server pthread)
    target_link_libraries(client pthread)
endif()