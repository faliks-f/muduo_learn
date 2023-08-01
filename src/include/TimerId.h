#ifndef MUDUO_LEARN_TIMERID_H
#define MUDUO_LEARN_TIMERID_H

#include "base/include/Copyable.h"
#include "src/include/Timer.h"

namespace faliks {
    class TimerId : public Copyable {
    private:
        Timer *m_timer;
        int64_t m_sequence;
    public:
        TimerId() : m_timer(nullptr), m_sequence(0) {}

        TimerId(Timer *timer, int64_t seq) : m_timer(timer), m_sequence(seq) {}

        friend class TimerQueue;
    };
}

#endif //MUDUO_LEARN_TIMERID_H
