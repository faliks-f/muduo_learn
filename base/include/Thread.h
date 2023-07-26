//
// Created by faliks on 23-7-25.
//

#ifndef MUDUO_LEARN_THREAD_H
#define MUDUO_LEARN_THREAD_H

#include "base/include/NoneCopyable.h"
#include "base/include/CurrentThread.h"
#include "base/include/CountDownLatch.h"

#include <pthread.h>
#include <functional>
#include <memory>
#include <string>
#include <atomic>

namespace faliks {
    class Thread : public NoneCopyable {
    private:
        bool m_started;
        bool m_joined;
        pthread_t m_pthreadId;
        pid_t m_tid;
        std::function<void()> m_func;
        std::string m_name;
        CountDownLatch m_latch;

        static std::atomic<int64_t> m_numCreated;

        void setDefaultName();

    public:
        typedef std::function<void()> ThreadFunc;

        explicit Thread(ThreadFunc func, const std::string &name = std::string());

        ~Thread();

        void start();

        int join();

        bool started() const { return m_started; }

        pid_t tid() const { return m_tid; }

        const std::string &name() const { return m_name; }

        static int64_t numCreated() {
            return m_numCreated.fetch_add(0);
        }
    };
}
std::atomic<int64_t> faliks::Thread::m_numCreated(0);

#endif //MUDUO_LEARN_THREAD_H
