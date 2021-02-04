cmake_minimum_required(VERSION 3.16)
project(chat_in_c C)

set(CMAKE_C_STANDARD 11)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(server server.c)
add_executable(client client.c)

target_link_libraries(server PRIVATE Threads::Threads)