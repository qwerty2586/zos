cmake_minimum_required(VERSION 3.6)
project(zos)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES fat-reader.c file_system.cpp file_system.h)
add_executable(zos ${SOURCE_FILES})