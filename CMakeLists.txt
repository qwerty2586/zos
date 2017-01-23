cmake_minimum_required(VERSION 3.6)
project(zos)

if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif ()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
if (LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")
endif ()

set(SOURCE_FILES main.cpp file_system.cpp file_system.h zmk.h string_functions.cpp string_functions.h file_system_defragmenter.cpp file_system_defragmenter.h job.h job_list.cpp job_list.h)
add_executable(zos ${SOURCE_FILES})