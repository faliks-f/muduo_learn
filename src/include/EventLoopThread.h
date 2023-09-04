#ifndef MUDUO_LEARN_EVENTLOOPTHREAD_H
#define MUDUO_LEARN_EVENTLOOPTHREAD_H

#include "base/include/NoneCopyable.h"
#include "base/include/ThreadSaftyCheck.h"
#include "base/include/Thread.h"

#include <mutex>
#include <condition_variable>
#include <functional>

namespace faliks {

    class EventLoop;

    class EventLoopThread : NoneCopyable {
    private:
        using ThreadInitCallback = std::function<void(EventLoop *)>;

        EventLoop *m_loop GUARDED_BY(m_mutex);
        bool m_exiting;
        Thread m_thread;
        std::mutex m_mutex;
        std::condition_variable m_cond GUARDED_BY(m_mutex);
        ThreadInitCallback m_callback;

        void threadFunc();

    public:

        EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                        const std::string &name = std::string());

        ~EventLoopThread();

        EventLoop *startLoop();
    };

}


#endif //MUDUO_LEARN_EVENTLOOPTHREAD_H
