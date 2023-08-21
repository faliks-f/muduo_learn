set(SRCS
        Channel.cpp
        EventLoop.cpp
        Poller.cpp
        EPollPoller.cpp
        Timer.cpp
        TimerQueue.cpp
        InetAddress.cpp
        Socket.cpp
        Buffer.cpp
)


add_library(muduo_learn_src ${SRCS})
target_link_libraries(muduo_learn_src muduo_learn_base ${LIBFMTLOG_PATH} pthread)

install(TARGETS muduo_learn_src DESTINATION lib)

set(GLOB HEADERS "include/*.h")
install(FILES ${HEADERS} DESTINATION lib/include/muduo_learn)

add_subdirectory(test)