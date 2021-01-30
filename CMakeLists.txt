cmake_minimum_required(VERSION 3.16)
project(chat_in_c C)

set(CMAKE_C_STANDARD 11)

add_executable(server server.c mySocket.h mySocket.c)
add_executable(client client.c mySocket.h mySocket.c)

if(WIN32)
    target_link_libraries(server Ws2_32 )
    target_link_libraries(client Ws2_32 )
endif()