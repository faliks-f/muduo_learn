#ifndef MUDUO_LEARN_COUNTDOWNLATCH_H
#define MUDUO_LEARN_COUNTDOWNLATCH_H

#include "base/include/NoneCopyable.h"
#include "base/include/ThreadSaftyCheck.h"

#include <mutex>
#include <condition_variable>

namespace faliks {
    class CountDownLatch : public NoneCopyable {
    private:
        std::mutex m_mutex;
        int m_count THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(m_mutex));
        std::condition_variable m_condition;
    public:
        explicit CountDownLatch(int count);

        void wait();

        void countDown();

        int getCount();
    };
}


#endif //MUDUO_LEARN_COUNTDOWNLATCH_H
