add_executable(TimerQueueTest TimerQueueTest.cpp)
target_link_libraries(TimerQueueTest muduo_learn_src ${LIBFMTLOG_PATH})

add_executable(EventLoopTest EventLoopTest.cpp)
target_link_libraries(EventLoopTest muduo_learn_src ${LIBFMTLOG_PATH})

add_executable(InetAddressTest InetAddressTest.cpp)
target_link_libraries(InetAddressTest muduo_learn_src ${LIBFMTLOG_PATH})

add_executable(BufferTest BufferTest.cpp)
target_link_libraries(BufferTest muduo_learn_src ${LIBFMTLOG_PATH})

add_executable(TcpEchoServerTest TcpEchoServerTest.cpp)
target_link_libraries(TcpEchoServerTest muduo_learn_src ${LIBFMTLOG_PATH})

