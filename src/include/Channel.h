#ifndef MUDUO_LEARN_CHANNEL_H
#define MUDUO_LEARN_CHANNEL_H

#include "base/include/NoneCopyable.h"
#include "EventLoop.h"

#include <memory>
#include <functional>
#include <sys/epoll.h>

namespace faliks {

    class EventLoop;

    class Channel : NoneCopyable {
    private:
        using EventCallback = std::function<void()>;
        using ReadEventCallback = std::function<void(Timestamp)>;

        static const int kNoneEvent = 0;
        static const int kReadEvent = EPOLLIN | EPOLLPRI;
        static const int kWriteEvent = EPOLLOUT;

        const int m_fd;
        EventLoop *m_loop;
        int m_events;
        int m_revents;
        int m_index;
        bool m_logHup;

        std::weak_ptr<void> m_tie;
        bool m_tied;
        bool m_eventHandling;
        bool m_addedToLoop;

        ReadEventCallback m_readCallback;
        EventCallback m_writeCallback;
        EventCallback m_closeCallback;
        EventCallback m_errorCallback;

        void handleEventWithGuard(Timestamp receiveTime);

        static std::string eventsToString(int fd, int ev);

        void update();

    public:
        Channel(EventLoop *loop, int fd);

        ~Channel();

        void handleEvent(Timestamp receiveTime);

        void setReadCallback(ReadEventCallback cb) {
            m_readCallback = std::move(cb);
        }

        void setWriteCallback(EventCallback cb) {
            m_writeCallback = std::move(cb);
        }

        void setCloseCallback(EventCallback cb) {
            m_closeCallback = std::move(cb);
        }

        void setErrorCallback(EventCallback cb) {
            m_errorCallback = std::move(cb);
        }

        void tie(const std::shared_ptr<void> &);

        [[nodiscard]] int getFd() const;

        [[nodiscard]] int events() const;

        void setRevents(int revt);

        [[nodiscard]] bool isNoneEvent() const;

        void enableReading();

        void disableReading();

        void enableWriting();

        void disableWriting();

        void disableAll();

        [[nodiscard]] bool isWriting() const;

        [[nodiscard]] bool isReading() const;

        [[nodiscard]] int index() const;

        void setIndex(int index);

        [[nodiscard]] std::string reventsToString() const;

        [[nodiscard]] std::string eventsToString() const;

        void doNotLogHup();

        [[nodiscard]] EventLoop *ownerLoop() const;

        void remove();
    };

}


#endif //MUDUO_LEARN_CHANNEL_H
