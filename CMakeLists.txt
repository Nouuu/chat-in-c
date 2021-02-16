cmake_minimum_required(VERSION 3.16)
project(chat_in_c C)

set(CMAKE_C_STANDARD 11)

add_library(serverCore src/server.c src/client.c src/client.h src/mySocket.h src/mySocket.c src/ServerClient.c src/ServerClient.h src/server.h src/ServerClientCommon.h src/serverMain.c)

add_executable(server src/serverMain.c )
add_executable(client src/clientMain.c)
add_executable(test src/test.c)
add_test(Test test)

if(WIN32)
    target_link_libraries(serverCore Ws2_32 pthread)
else()
    target_link_libraries(serverCore pthread)
endif()

target_link_libraries(server serverCore)
target_link_libraries(client serverCore)
target_link_libraries(test serverCore)

