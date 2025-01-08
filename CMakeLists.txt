cmake_minimum_required(VERSION 3.14)
project(sectrans VERSION 1.0 LANGUAGES C)

include(FetchB64.cmake)
include(FetchSodium.cmake)

include_directories(src/include)
link_directories(src/include)

add_library(client_message src/client/client_message.c)
add_library(server_message src/server/server_message.c)
add_library(client_utils src/client/client_utils.c)

target_include_directories(client_utils PRIVATE src/include)
target_include_directories(client_message PRIVATE src/include)
target_include_directories(server_message PRIVATE src/include)

target_link_libraries(client_utils PRIVATE b64c sodium)
target_link_libraries(client_message PRIVATE client_utils b64c sodium)
target_link_libraries(server_message PRIVATE client_utils b64c sodium)

add_executable(${PROJECT_NAME}_server src/server/server.c)
add_executable(${PROJECT_NAME}_client src/client/client.c)

target_link_libraries(${PROJECT_NAME}_client client server client_utils client_message b64c sodium)
target_link_libraries(${PROJECT_NAME}_server server client client_utils server_message b64c sodium)

# TESTS

include(CTest)
include(FetchCMocka.cmake)

add_executable(${PROJECT_NAME}_server_test tests/server.c)
add_executable(${PROJECT_NAME}_client_test tests/client.c)
target_link_libraries(${PROJECT_NAME}_server_test PRIVATE cmocka-static)
target_link_libraries(${PROJECT_NAME}_client_test PRIVATE cmocka-static)

enable_testing()
add_test(NAME Server COMMAND ${PROJECT_NAME}_server_test)
add_test(NAME Client COMMAND ${PROJECT_NAME}_client_test)
