#ifndef MUDUO_LEARN_THREADPOOL_H
#define MUDUO_LEARN_THREADPOOL_H

#include "base/include/NoneCopyable.h"

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace faliks {

    class EventLoop;

    class EventLoopThread;

    class ThreadPool : NoneCopyable {
    private:
        EventLoop *m_baseLoop;
        std::string m_name;
        bool m_started;
        int m_numThreads;
        int m_next;
        std::vector<EventLoop *> m_loops;
        std::vector<std::unique_ptr<EventLoopThread>> m_threads;
    public:
        using ThreadInitCallback = std::function<void(EventLoop *)>;

        ThreadPool(EventLoop *baseLoop, const std::string &nameArg);

        ~ThreadPool();

        void setThreadNum(int numThreads) { m_numThreads = numThreads; }

        void start(const ThreadInitCallback &cb = ThreadInitCallback());

        EventLoop *getNextLoop();

        EventLoop *getLoopForHash(size_t hashCode);

        std::vector<EventLoop *> getAllLoops();
    };
}


#endif //MUDUO_LEARN_THREADPOOL_H
