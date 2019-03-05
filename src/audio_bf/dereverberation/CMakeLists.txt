cmake_minimum_required(VERSION 3.0.0)
project(test_kalman VERSION 0.1.0)

INCLUDE_DIRECTORIES(inc)
AUX_SOURCE_DIRECTORY(src DIR_SRCS)

set(CMAKE_C_FLAGS_DEBUG "-g") 
add_executable(test_kalman ${DIR_SRCS})

