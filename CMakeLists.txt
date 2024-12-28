cmake_minimum_required(VERSION 3.14)
project(sectrans VERSION 1.0 LANGUAGES C)

include_directories(src/include)
link_directories(src/include)

add_executable(${PROJECT_NAME}_server src/server/server.c)
add_executable(${PROJECT_NAME}_client src/client/client.c)
target_link_libraries(${PROJECT_NAME}_client client)
target_link_libraries(${PROJECT_NAME}_server server)
