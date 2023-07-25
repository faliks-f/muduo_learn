set(SRCS
        Channel.cpp
        EventLoop.cpp
        )

add_library(muduo_learn_src ${SRCS})
target_link_libraries(muduo_learn_src muduo_base)

install(TARGETS muduo_learn_src DESTINATION lib)

set(GLOB HEADERS "include/*.h")
install(FILES ${HEADERS} DESTINATION lib/include/muduo_learn)
