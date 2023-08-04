add_executable(TimerQueueTest TimerQueueTest.cpp)
target_link_libraries(TimerQueueTest muduo_learn_src ${LIBFMTLOG_PATH})

add_executable(EventLoopTest EventLoopTest.cpp)
target_link_libraries(EventLoopTest muduo_learn_src ${LIBFMTLOG_PATH})

add_executable(InetAddressTest InetAddressTest.cpp)
target_link_libraries(InetAddressTest muduo_learn_src ${LIBFMTLOG_PATH} boost_unit_test_framework)
add_test(NAME InetAddressTest COMMAND inetaddress_unittest)


