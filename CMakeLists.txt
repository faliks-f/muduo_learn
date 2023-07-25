cmake_minimum_required(VERSION 3.25)
project(muduo_learn)

set(CMAKE_CXX_STANDARD 17)

set(${PROJECT_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(base)
add_subdirectory(src)