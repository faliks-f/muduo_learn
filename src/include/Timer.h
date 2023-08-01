#ifndef MUDUO_LEARN_TIMER_H
#define MUDUO_LEARN_TIMER_H

#include <functional>
#include <atomic>

#include "base/include/NoneCopyable.h"
#include "base/include/Timestamp.h"

namespace faliks {
    class Timer : NoneCopyable {
    private:
        std::function<void()> m_callback;
        Timestamp m_expiration;
        const double m_interval;
        const bool m_repeat;
        const int64_t m_sequence;

        static std::atomic<int64_t> s_numCreated;
    public:
        Timer(std::function<void()> cb, Timestamp when, double interval);

        void run() const;

        [[nodiscard]] Timestamp expiration() const;

        [[nodiscard]] bool repeat() const;

        [[nodiscard]] int64_t sequence() const;

        void restart(Timestamp now);

        static int64_t numCreated();
    };
}

#endif //MUDUO_LEARN_TIMER_H
