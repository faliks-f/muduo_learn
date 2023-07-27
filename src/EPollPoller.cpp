//
// Created by faliks on 23-7-27.
//

#include "src/include/EPollPoller.h"
#include "src/include/Channel.h"

#include <map>
#include <string>
#include <cassert>


using std::map;
using std::string;

namespace faliks {

    const char *EPollPoller::operationToString(int op) {
        static const char *ops[4] = {
                "None",
                "Add",
                "Del",
                "Mod"
        };
        if (op < 0 || op > 3) {
            assert(false && "ERROR Operation");
            return "Unknown Operation";
        }
        return ops[op];
    }

    void faliks::EPollPoller::fillActiveChannels(int numEvents, Poller::ChannelList *activeChannels) const {
        assert(implicit_cast<size_t>(numEvents) <= m_events.size());
        for (int i = 0; i < numEvents; ++i) {
            auto *channel = static_cast<Channel *>(m_events[i].data.ptr);
            channel->setRevents(m_events[i].events);
            activeChannels->push_back(channel);
        }
    }

    void EPollPoller::update(int operation, Channel *channel) {
        struct epoll_event event{};
        event.events = channel->events();
        event.data.ptr = channel;
        int fd = channel->getFd();
        // todo: add log
        if (::epoll_ctl(m_epollFd, operation, fd, &event) < 0) {
            if (operation == EPOLL_CTL_DEL) {
                assert(false && "epoll_ctl del error");
            } else {
                assert(false && "epoll_ctl add/mod error");
            }
        }
    }

    EPollPoller::EPollPoller(EventLoop *loop) : Poller(loop) {
        m_epollFd = ::epoll_create1(EPOLL_CLOEXEC);
        if (m_epollFd < 0) {
            assert(false && "epoll_create1 error");
        }
    }

    EPollPoller::~EPollPoller() {
        ::close(m_epollFd);
    }

    Timestamp EPollPoller::poll(int timeoutMs, Poller::ChannelList *activeChannels) {
        //todo: add log
        int numEvents = ::epoll_wait(m_epollFd, &*m_events.begin(), static_cast<int>(m_events.size()), timeoutMs);
        int savedErrno = errno;
        Timestamp now(Timestamp::now());
        if (numEvents > 0) {
            //todo: add log
            fillActiveChannels(static_cast<size_t>(numEvents), activeChannels);
            if (static_cast<size_t>(numEvents) == m_events.size()) {
                m_events.resize(m_events.size() * 2);
            }
        } else if (numEvents == 0) {
            //todo: add log
        } else {
            if (savedErrno != EINTR) {
                errno = savedErrno;
                //todo: add log
            }
        }
        return now;
    }

    void EPollPoller::updateChannel(Channel *channel) {
        // todo
    }

    void EPollPoller::removeChannel(Channel *channel) {
        // todo
    }
}
