cmake_minimum_required(VERSION 3.14)
project(sectrans VERSION 1.0 LANGUAGES C)

include_directories(src/include)
add_executable(sectrans src/sectrans.c)
