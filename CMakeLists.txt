cmake_minimum_required(VERSION 3.25)
project(muduo_learn)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")


include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(LIBFMTLOG_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/libfmtlog-static.a
        ${CMAKE_CURRENT_SOURCE_DIR}/libfmt.a
)

add_subdirectory(base)
add_subdirectory(src)

