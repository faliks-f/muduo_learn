#include "src/include/TimerQueue.h"
#include "src/include/EventLoop.h"
#include "src/include/Timer.h"
#include "src/include/Channel.h"
#include "base/include/fmtlog.h"

#include <sys/timerfd.h>
#include <cassert>

namespace faliks {
    int createTimerFd() {
        int timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        if (timerFd < 0) {
            logw("Failed in timerfd_create");
        }
        return timerFd;
    }

    struct timespec howMuchTimeFromNow(Timestamp when) {
        int64_t microseconds = when.microSecondsSinceEpoch() - Timestamp::now().microSecondsSinceEpoch();
        if (microseconds < 100) {
            microseconds = 100;
        }
        struct timespec ts{};
        ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
        ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
        return ts;

    }

    void resetTimerFd(int timerFd, Timestamp expiration) {
        struct itimerspec newValue{};
        struct itimerspec oldValue{};
        bzero(&newValue, sizeof(newValue));
        bzero(&oldValue, sizeof(oldValue));
        newValue.it_value = howMuchTimeFromNow(expiration);
        int ret = ::timerfd_settime(timerFd, 0, &newValue, &oldValue);
        if (ret) {
            logw("timerfd_settime error");
        }
    }

    void readTimerFd(int timerFd, Timestamp now) {
        uint64_t howmany;
        ssize_t n = ::read(timerFd, &howmany, sizeof(howmany));
        logi("TimerQueue::handleRead() {} at {}", howmany, now.toString());
        if (n != sizeof(howmany)) {
            logw("TimerQueue::handleRead() reads {} bytes instead of 8", n);
        }
    }

    void TimerQueue::addTimerInLoop(Timer *timer) {
        m_loop->assertInLoopThread();
        bool earliestChanged = insert(timer);
        if (earliestChanged) {
            resetTimerFd(m_timerFd, timer->expiration());
        }
    }

    bool TimerQueue::insert(Timer *timer) {
        m_loop->assertInLoopThread();
        assert(m_timers.size() == m_activeTimers.size());
        bool earliestChanged = false;
        Timestamp when = timer->expiration();
        auto it = m_timers.begin();
        if (it == m_timers.end() || when < it->first) {
            earliestChanged = true;
        }
        {
            std::pair<std::set<Entry>::iterator, bool> result = m_timers.insert(Entry(when, timer));
            assert(result.second);
        }
        {
            std::pair<std::set<ActiveTimer>::iterator, bool> result = m_activeTimers.insert(
                    ActiveTimer(timer, timer->sequence()));
            assert(result.second);
        }
        assert(m_timers.size() == m_activeTimers.size());
        return earliestChanged;
    }

    void TimerQueue::cancelInLoop(Timer *timer) {
        m_loop->assertInLoopThread();
        assert(m_timers.size() == m_activeTimers.size());
        ActiveTimer timer1(timer, timer->sequence());
        auto it = m_activeTimers.find(timer1);
        if (it != m_activeTimers.end()) {
            size_t n = m_timers.erase(Entry(it->first->expiration(), it->first));
            assert(n == 1);
            delete it->first;
            m_activeTimers.erase(it);
        } else if (m_callingExpiredTimers) {
            m_cancelingTimers.insert(timer1);
        }
        assert(m_timers.size() == m_activeTimers.size());
    }

    void TimerQueue::handleRead() {
        m_loop->assertInLoopThread();
        Timestamp now(Timestamp::now());
        readTimerFd(m_timerFd, now);

        std::vector<Entry> expired = getExpired(now);
        m_callingExpiredTimers = true;
        m_cancelingTimers.clear();

        for (const auto &it: expired) {
            it.second->run();
        }
        m_callingExpiredTimers = false;
        reset(expired, now);
    }

    std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now) {
        assert(m_timers.size() == m_activeTimers.size());
        std::vector<Entry> expired;
        Entry sentry(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
        auto end = m_timers.lower_bound(sentry);
        assert(end == m_timers.end() || now < end->first);
        std::copy(m_timers.begin(), end, back_inserter(expired));
        m_timers.erase(m_timers.begin(), end);

        for (const Entry &it: expired) {
            ActiveTimer timer(it.second, it.second->sequence());
            size_t n = m_activeTimers.erase(timer);
            assert(n == 1);
        }

        assert(m_timers.size() == m_activeTimers.size());
        return expired;
    }

    void TimerQueue::reset(const std::vector<Entry> &expired, Timestamp now) {
        Timestamp nextExpire;

        for (const auto &it: expired) {
            ActiveTimer timer(it.second, it.second->sequence());
            if (it.second->repeat() && m_cancelingTimers.find(timer) == m_cancelingTimers.end()) {
                it.second->restart(now);
                insert(it.second);
            } else {
//                delete it.second;
            }
        }

        if (!m_timers.empty()) {
            nextExpire = m_timers.begin()->second->expiration();
        }

        if (nextExpire.valid()) {
            resetTimerFd(m_timerFd, nextExpire);
        }
    }

    TimerQueue::TimerQueue(EventLoop *loop)
            : m_loop(loop),
              m_timerFd(createTimerFd()),
              m_timerFdChannel(loop, m_timerFd),
              m_timers(),
              m_callingExpiredTimers(false) {
        m_timerFdChannel.setReadCallback([this](Timestamp) { handleRead(); });
        m_timerFdChannel.enableReading();
    }

    TimerQueue::~TimerQueue() {
        m_timerFdChannel.disableAll();
        m_timerFdChannel.remove();
        ::close(m_timerFd);

//        for (const Entry &it: m_timers) {
//            delete it.second;
//        }
    }

    TimerId TimerQueue::addTimer(std::function<void()> cb, Timestamp when, double interval) {
        auto timer = new Timer(std::move(cb), when, interval);
        m_loop->runInLoop([this, timer]() { addTimerInLoop(timer); });
        return {timer, timer->sequence()};
    }

    void TimerQueue::cancel(TimerId timerId) {
        m_loop->runInLoop([this, timerId]() { cancelInLoop(timerId.m_timer); });
    }


}