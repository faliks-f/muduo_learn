#ifndef MUDUO_LEARN_TIMERQUEUE_H
#define MUDUO_LEARN_TIMERQUEUE_H

#include <set>

#include "base/include/NoneCopyable.h"
#include "base/include/Timestamp.h"
#include "src/include/Channel.h"
#include "src/include/TimerId.h"

namespace faliks {
    class Timer;

    class EventLoop;

    class TimerQueue : public NoneCopyable {

    private:
        using Entry = std::pair<Timestamp, Timer *>;
        using ActiveTimer = std::pair<Timer *, int64_t>;
        EventLoop *m_loop;
        const int m_timerFd;
        Channel m_timerFdChannel;
        std::set<Entry> m_timers;
        std::set<ActiveTimer> m_activeTimers;
        bool m_callingExpiredTimers;
        std::set<ActiveTimer> m_cancelingTimers;

        bool insert(Timer *timer);

        void addTimerInLoop(Timer *timer);

        void cancelInLoop(Timer *timer);

        [[nodiscard]] std::vector<Entry> getExpired(Timestamp now);

        void reset(const std::vector<Entry> &expired, Timestamp now);

        void handleRead();

    public:
        explicit TimerQueue(EventLoop *loop);

        ~TimerQueue();

        TimerId addTimer(std::function<void()> cb, Timestamp when, double interval);

        void cancel(TimerId timerId);
    };
}


#endif //MUDUO_LEARN_TIMERQUEUE_H
