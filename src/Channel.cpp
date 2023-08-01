#include "src/include/Channel.h"
#include "src/include/EventLoop.h"

#include <cassert>
#include <sys/epoll.h>
#include <sstream>

faliks::Channel::Channel(faliks::EventLoop *loop, int fd)
        : m_loop(loop),
          m_fd(fd),
          m_events(0),
          m_revents(0),
          m_index(-1),
          m_logHup(true),
          m_tied(false),
          m_eventHandling(false),
          m_addedToLoop(false) {
    printf("here created Channel\n");
}

faliks::Channel::~Channel() {
    assert(!m_eventHandling);
    assert(!m_addedToLoop);
    if (m_loop->isInLoopThread()) {
        assert(!m_loop->hasChannel(this));
    }
}

void faliks::Channel::handleEvent(faliks::Timestamp receiveTime) {
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

void faliks::Channel::handleEventWithGuard(faliks::Timestamp receiveTime) {
    m_eventHandling = true;
    // todo: add log
    if ((m_revents & EPOLLHUP) && !(m_revents & EPOLLIN)) {
        if (m_logHup) {
            // todo: add log
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

void faliks::Channel::tie(const std::shared_ptr<void> &obj) {
    m_tie = obj;
    m_tied = true;
}

void faliks::Channel::update() {
    m_addedToLoop = true;
    m_loop->updateChannel(this);
}

int faliks::Channel::getFd() const {
    return m_fd;
}

int faliks::Channel::events() const {
    return m_events;
}

void faliks::Channel::setRevents(int revt) {
    m_revents = revt;
}

bool faliks::Channel::isNoneEvent() const {
    return m_events == kNoneEvent;
}

void faliks::Channel::enableReading() {
    m_events |= kReadEvent;
    update();
}

void faliks::Channel::disableReading() {
    m_events &= ~kReadEvent;
    update();
}

void faliks::Channel::enableWriting() {
    m_events |= kWriteEvent;
    update();
}

void faliks::Channel::disableWriting() {
    m_events &= ~kWriteEvent;
    update();
}

void faliks::Channel::disableAll() {
    m_events = kNoneEvent;
    update();
}

bool faliks::Channel::isWriting() const {
    return m_events & kWriteEvent;
}

bool faliks::Channel::isReading() const {
    return m_events & kReadEvent;
}

int faliks::Channel::index() const {
    return m_index;
}

void faliks::Channel::setIndex(int index) {
    m_index = index;
}

std::string faliks::Channel::reventsToString() const {
    return eventsToString(m_fd, m_revents);
}

std::string faliks::Channel::eventsToString() const {
    return eventsToString(m_fd, m_events);
}

std::string faliks::Channel::eventsToString(int fd, int ev) {
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

void faliks::Channel::doNotLogHup() {
    m_logHup = false;
}


void faliks::Channel::remove() {
    assert(isNoneEvent());
    m_addedToLoop = false;
    m_loop->removeChannel(this);
}

faliks::EventLoop *faliks::Channel::ownerLoop() const {
    return m_loop;
}

