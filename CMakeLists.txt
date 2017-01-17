cmake_minimum_required(VERSION 3.6)
project(zos)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp file_system.cpp file_system.h zmk.h string_functions.cpp string_functions.h)
add_executable(zos ${SOURCE_FILES})