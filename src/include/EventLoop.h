#ifndef MUDUO_LEARN_EVENTLOOP_H
#define MUDUO_LEARN_EVENTLOOP_H

#include <functional>
#include <atomic>
#include <unistd.h>
#include <memory>
#include <mutex>

#include <boost/any.hpp>

#include "base/include/NoneCopyable.h"
#include "base/include/Timestamp.h"
#include "src/include/TimerId.h"

namespace faliks {
    class Channel;

    class Poller;

    class TimerQueue;

    class EventLoop : NoneCopyable {
    private:
        using ChannelList = std::vector<Channel *>;
        std::atomic<bool> m_looping;
        std::atomic<bool> m_quit;
        std::atomic<bool> m_eventHandling;
        std::atomic<bool> m_callingPendingFunctors;
        int64_t m_iteration;
        const pid_t m_threadId;
        Timestamp m_pollReturnTime;
        std::unique_ptr<Poller> m_poller;
        std::unique_ptr<TimerQueue> m_timerQueue;
        int m_wakeupFd;
        std::unique_ptr<Channel> m_wakeupChannel;
        boost::any m_context;
        ChannelList m_activeChannels;
        Channel *m_currentActiveChannel;
        std::mutex m_pendingMutex;
        std::mutex m_quitMutex;
        std::vector<std::function<void()>> m_pendingFunctors;

        void abortNotInLoopThread();

        void handleRead() const;

        void printActiveChannels() const;

        void doPendingFunctors();

    public:
        EventLoop();

        ~EventLoop();

        void assertInLoopThread();

        void loop();

        void wakeup() const;

        void quit();

        [[nodiscard]] Timestamp pollReturnTime() const;

        [[nodiscard]] int64_t iteration() const;

        void runInLoop(std::function<void()> cb);

        void queueInLoop(std::function<void()> cb);

        [[nodiscard]] size_t queueSize();

        [[nodiscard]] bool isInLoopThread() const;

        TimerId runAt(Timestamp time, std::function<void()> cb);

        TimerId runAfter(double delay, std::function<void()> cb);

        TimerId runEvery(double interval, std::function<void()> cb);

        void cancel(TimerId timerId);

        void updateChannel(Channel *channel);

        void removeChannel(Channel *channel);

        bool hasChannel(Channel *channel);

        [[nodiscard]] bool eventHandling() const;

        void setContext(const boost::any &context);

        [[nodiscard]] const boost::any &getContext() const;

        static EventLoop *getEventLoopOfCurrentThread();
    };
}


#endif //MUDUO_LEARN_EVENTLOOP_H
