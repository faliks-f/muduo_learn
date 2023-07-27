set(BASE_SRCS
        CurrentThread.cpp
        Thread.cpp
        CountDownLatch.cpp
        Timestamp.cpp
)

add_library(muduo_learn_base SHARED ${BASE_SRCS})
target_link_libraries(muduo_learn_base pthread)


install(TARGETS muduo_learn_base DESTINATION lib)
FILE(GLOB HEADERS "include/*.h")
install(FILES ${HEADERS} DESTINATION lib/include/base)

add_subdirectory(test)