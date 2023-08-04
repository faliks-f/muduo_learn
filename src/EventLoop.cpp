#include "src/include/EventLoop.h"
#include "src/include/Channel.h"
#include "src/include/Poller.h"
#include "src/include/TimerQueue.h"

#include "base/include/CurrentThread.h"

#include <sys/eventfd.h>
#include <cassert>

namespace faliks {

    __thread EventLoop *t_loopInThisThread = nullptr;

    constexpr int kPollTimeMs = 10000;

    int createEventFd() {
        int eventFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (eventFd < 0) {
            // todo: log
            abort();
        }
        return eventFd;
    }


    EventLoop::EventLoop()
            : m_looping(false),
              m_quit(false),
              m_eventHandling(false),
              m_callingPendingFunctors(false),
              m_iteration(0),
              m_threadId(CurrentThread::tid()),
              m_pollReturnTime(Timestamp::now()),
              m_poller(Poller::newDefaultPoller(this)),
              m_timerQueue(new TimerQueue(this)),
              m_wakeupFd(createEventFd()),
              m_wakeupChannel(new Channel(this, m_wakeupFd)),
              m_currentActiveChannel(nullptr) {
    }

    EventLoop::~EventLoop() {
        m_wakeupChannel->disableAll();
        m_wakeupChannel->remove();
        ::close(m_wakeupFd);
        m_currentActiveChannel = nullptr;
    }

    void EventLoop::loop() {
        assert(!m_looping);
        assertInLoopThread();
        m_looping = true;
        m_quit = false;

        // todo: log
        {
//            std::scoped_lock<std::mutex> lock(m_quitMutex);
            while (!m_quit) {
                m_activeChannels.clear();
                m_pollReturnTime = m_poller->poll(kPollTimeMs, &m_activeChannels);
                ++m_iteration;
                m_eventHandling = true;
                //todo: log

                for (auto *channel: m_activeChannels) {
                    m_currentActiveChannel = channel;
                    m_currentActiveChannel->handleEvent(m_pollReturnTime);
                }
                m_currentActiveChannel = nullptr;
                m_eventHandling = false;
                doPendingFunctors();
            }
        }
    }

    void EventLoop::assertInLoopThread() {
        if (!isInLoopThread()) {
            abortNotInLoopThread();
        }
    }

    void EventLoop::abortNotInLoopThread() {
        // todo: log
    }

    bool EventLoop::isInLoopThread() const {
        return m_threadId == CurrentThread::tid();
    }

    void EventLoop::doPendingFunctors() {
        std::vector<std::function<void()>> functors;
        m_callingPendingFunctors = true;
        {
            std::scoped_lock<std::mutex> lock(m_pendingMutex);
            functors.swap(m_pendingFunctors);
        }

        for (const auto &functor: functors) {
            functor();
        }
        m_callingPendingFunctors = false;
    }

    void EventLoop::quit() {
        {
//            std::scoped_lock<std::mutex> lock(m_quitMutex);
            m_quit = true;
        }
        if (!isInLoopThread()) {
            wakeup();
        }
    }

    void EventLoop::wakeup() {
        constexpr uint64_t one = 1;
        // todo: socket write
    }

    Timestamp EventLoop::pollReturnTime() const {
        return m_pollReturnTime;
    }

    int64_t EventLoop::iteration() const {
        return m_iteration;
    }

    void EventLoop::runInLoop(std::function<void()> cb) {
        if (isInLoopThread()) {
            cb();
        } else {
            queueInLoop(std::move(cb));
        }
    }

    void EventLoop::queueInLoop(std::function<void()> cb) {
        {
            std::scoped_lock<std::mutex> lock(m_pendingMutex);
            m_pendingFunctors.emplace_back(std::move(cb));
        }
        if (!isInLoopThread() || m_callingPendingFunctors) {
            wakeup();
        }
    }

    size_t EventLoop::queueSize() {
        std::scoped_lock<std::mutex> lock(m_pendingMutex);
        return m_pendingFunctors.size();
    }

    void EventLoop::updateChannel(Channel *channel) {
        assert(channel->ownerLoop() == this);
        assertInLoopThread();
        if (m_eventHandling) {
            assert(m_currentActiveChannel == channel ||
                   std::find(m_activeChannels.begin(), m_activeChannels.end(), channel) == m_activeChannels.end());
        }
        m_poller->updateChannel(channel);
    }

    void EventLoop::removeChannel(Channel *channel) {
        assert(channel->ownerLoop() == this);
        assertInLoopThread();
        if (m_eventHandling) {
            assert(m_currentActiveChannel == channel ||
                   std::find(m_activeChannels.begin(), m_activeChannels.end(), channel) == m_activeChannels.end());
        }
        m_poller->removeChannel(channel);
    }

    bool EventLoop::hasChannel(Channel *channel) {
        assert(channel->ownerLoop() == this);
        assertInLoopThread();
        return m_poller->hasChannel(channel);
    }

    bool EventLoop::eventHandling() const {
        return m_eventHandling;
    }

    void EventLoop::setContext(const boost::any &context) {
        m_context = context;
    }

    const boost::any &EventLoop::getContext() const {
        return m_context;
    }

    EventLoop *EventLoop::getEventLoopOfCurrentThread() {
        return t_loopInThisThread;
    }

    TimerId EventLoop::runAt(Timestamp time, std::function<void()> cb) {
        return m_timerQueue->addTimer(std::move(cb), time, 0.0);
    }

    TimerId EventLoop::runAfter(double delay, std::function<void()> cb) {
        Timestamp time(addTime(Timestamp::now(), delay));
        return runAt(time, std::move(cb));
    }

    TimerId EventLoop::runEvery(double interval, std::function<void()> cb) {
        Timestamp time(addTime(Timestamp::now(), interval));
        return m_timerQueue->addTimer(std::move(cb), time, interval);
    }

    void EventLoop::cancel(TimerId timerId) {
        return m_timerQueue->cancel(timerId);
    }
}
