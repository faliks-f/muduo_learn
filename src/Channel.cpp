#include "src/include/Channel.h"
#include "src/include/EventLoop.h"
#include "base/include/fmtlog.h"

#include <cassert>
#include <sys/epoll.h>
#include <sstream>

namespace faliks {

    Channel::Channel(EventLoop *loop, int fd)
            : m_loop(loop),
              m_fd(fd),
              m_events(0),
              m_revents(0),
              m_index(-1),
              m_logHup(true),
              m_tied(false),
              m_eventHandling(false),
              m_addedToLoop(false) {
    }

    Channel::~Channel() {
        assert(!m_eventHandling);
        assert(!m_addedToLoop);
        if (m_loop->isInLoopThread()) {
            assert(!m_loop->hasChannel(this));
        }
    }

    void Channel::handleEvent(Timestamp receiveTime) {
        std::shared_ptr<void> guard;
        if (m_tied) {
            guard = m_tie.lock();
            if (guard) {
                handleEventWithGuard(receiveTime);
            }
        } else {
            handleEventWithGuard(receiveTime);
        }
    }

    void Channel::handleEventWithGuard(Timestamp receiveTime) {
        m_eventHandling = true;
        logd(reventsToString());
        if ((m_revents & EPOLLHUP) && !(m_revents & EPOLLIN)) {
            if (m_logHup) {
                logw("fd = {} Channel::handle_event() EPOLLHUP", m_fd);
            }
            if (m_closeCallback) {
                m_closeCallback();
            }
        }
        if (m_revents & EPOLLERR) {
            if (m_errorCallback) {
                m_errorCallback();
            }
        }
        if (m_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
            if (m_readCallback) {
                m_readCallback(receiveTime);
            }
        }
        if (m_revents & EPOLLOUT) {
            if (m_writeCallback) {
                m_writeCallback();
            }
        }
        m_eventHandling = false;
    }

    void Channel::tie(const std::shared_ptr<void> &obj) {
        m_tie = obj;
        m_tied = true;
    }

    void Channel::update() {
        m_addedToLoop = true;
        m_loop->updateChannel(this);
    }

    int Channel::getFd() const {
        return m_fd;
    }

    int Channel::events() const {
        return m_events;
    }

    void Channel::setRevents(int revt) {
        m_revents = revt;
    }

    bool Channel::isNoneEvent() const {
        return m_events == kNoneEvent;
    }

    void Channel::enableReading() {
        m_events |= kReadEvent;
        update();
    }

    void Channel::disableReading() {
        m_events &= ~kReadEvent;
        update();
    }

    void Channel::enableWriting() {
        m_events |= kWriteEvent;
        update();
    }

    void Channel::disableWriting() {
        m_events &= ~kWriteEvent;
        update();
    }

    void Channel::disableAll() {
        m_events = kNoneEvent;
        update();
    }

    bool Channel::isWriting() const {
        return m_events & kWriteEvent;
    }

    bool Channel::isReading() const {
        return m_events & kReadEvent;
    }

    int Channel::index() const {
        return m_index;
    }

    void Channel::setIndex(int index) {
        m_index = index;
    }

    std::string Channel::reventsToString() const {
        return eventsToString(m_fd, m_revents);
    }

    std::string Channel::eventsToString() const {
        return eventsToString(m_fd, m_events);
    }

    std::string Channel::eventsToString(int fd, int ev) {
        std::ostringstream oss;
        oss << fd << ": ";
        if (ev & EPOLLIN) {
            oss << "IN ";
        }
        if (ev & EPOLLPRI) {
            oss << "PRI ";
        }
        if (ev & EPOLLOUT) {
            oss << "OUT ";
        }
        if (ev & EPOLLHUP) {
            oss << "HUP ";
        }
        if (ev & EPOLLRDHUP) {
            oss << "RDHUP ";
        }
        if (ev & EPOLLERR) {
            oss << "ERR ";
        }
        return oss.str();
    }

    void Channel::doNotLogHup() {
        m_logHup = false;
    }


    void Channel::remove() {
        assert(isNoneEvent());
        m_addedToLoop = false;
        m_loop->removeChannel(this);
    }

    EventLoop *Channel::ownerLoop() const {
        return m_loop;
    }

}