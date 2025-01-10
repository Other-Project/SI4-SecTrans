cmake_minimum_required(VERSION 3.14)
project(sectrans VERSION 1.0 LANGUAGES C)

include(FetchB64.cmake)
include(FetchSodium.cmake)

include_directories(src/include)
link_directories(src/include)

add_executable(${PROJECT_NAME}_server src/server/server.c src/server/server_utils.c src/server/server_message.c src/client/client_message.c src/client/client_utils.c)
add_executable(${PROJECT_NAME}_client src/client/client.c src/server/server_utils.c src/client/client_message.c src/server/server_message.c src/client/client_utils.c)

target_link_libraries(${PROJECT_NAME}_client client server b64c sodium)
target_link_libraries(${PROJECT_NAME}_server server client b64c sodium)

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
