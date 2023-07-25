cmake_minimum_required(VERSION 3.25)
project(muduo_learn)

set(CMAKE_CXX_STANDARD 17)

set(${PROJECT_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_PATH}/src/include)

aux_source_directory(${PROJECT_PATH}/src DIR_SRCS)


add_library(muduo_learn SHARED ${DIR_SRCS} src/EventLoop.cpp src/include/EventLoop.h)