add_executable(ThreadTest ThreadTest.cpp)
target_link_libraries(ThreadTest muduo_learn_base ${LIBFMTLOG_PATH})

add_executable(TimestampTest TimestampTest.cpp)
target_link_libraries(TimestampTest muduo_learn_base ${LIBFMTLOG_PATH})