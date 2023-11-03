#include "src/include/EPollPoller.h"
#include "src/include/Channel.h"
#include "base/include/fmtlog.h"

#include <map>
#include <string>
#include <cassert>


using std::map;
using std::string;

namespace faliks {

    constexpr int kNew = -1;
    constexpr int kAdded = 1;
    constexpr int kDeleted = 2;

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

    void EPollPoller::fillActiveChannels(int numEvents, Poller::ChannelList *activeChannels) const {
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
        logd("epoll_ctl op = {} fd = {} event = {}", operationToString(operation), fd, channel->eventsToString());
        if (::epoll_ctl(m_epollFd, operation, fd, &event) < 0) {
            if (operation == EPOLL_CTL_DEL) {
                loge("epoll_ctl op = {} fd = {}", operationToString(operation), fd);
                assert(false && "epoll_ctl del error");
            } else {
                loge("epoll_ctl op = {} fd = {}", operationToString(operation), fd);
                assert(false && "epoll_ctl add/mod error");
            }
        }
    }

    EPollPoller::EPollPoller(EventLoop *loop)
            : Poller(loop),
              m_epollFd(::epoll_create1(EPOLL_CLOEXEC)),
              m_events(kInitEventListSize) {
        if (m_epollFd < 0) {
            assert(false && "epoll_create1 error");
        }
    }

    EPollPoller::~EPollPoller() {
        ::close(m_epollFd);
    }

    Timestamp EPollPoller::poll(int timeoutMs, Poller::ChannelList *activeChannels) {
        logi("fd total count {}", m_channels.size());
        int numEvents = ::epoll_wait(m_epollFd, &*m_events.begin(), static_cast<int>(m_events.size()), timeoutMs);
        int savedErrno = errno;
        Timestamp now(Timestamp::now());
        if (numEvents > 0) {
            logi("{} events happened", numEvents);
            fillActiveChannels(static_cast<size_t>(numEvents), activeChannels);
            if (static_cast<size_t>(numEvents) == m_events.size()) {
                m_events.resize(m_events.size() * 2);
            }
        } else if (numEvents == 0) {
            logi("nothing happened");
        } else {
            if (savedErrno != EINTR) {
                errno = savedErrno;
                logw("EPollPoller::poll()");
            }
        }
        return now;
    }

    void EPollPoller::updateChannel(Channel *channel) {
        Poller::assertInLoopThread();
        const int index = channel->index();
        logd("fd = {} events = {} index = {}", channel->getFd(), channel->eventsToString(), index);
        if (index == kNew || index == kDeleted) {
            int fd = channel->getFd();
            if (index == kNew) {
                assert(m_channels.find(fd) == m_channels.end());
                m_channels[fd] = channel;
            } else {
                assert(m_channels.find(fd) != m_channels.end());
                assert(m_channels[fd] == channel);
            }
            channel->setIndex(kAdded);
            update(EPOLL_CTL_ADD, channel);
        } else {
            int fd = channel->getFd();
            assert(m_channels.find(fd) != m_channels.end());
            assert(m_channels[fd] == channel);
            assert(index == kAdded);
            if (channel->isNoneEvent()) {
                update(EPOLL_CTL_DEL, channel);
                channel->setIndex(kDeleted);
            } else {
                update(EPOLL_CTL_MOD, channel);
            }
        }
    }

    void EPollPoller::removeChannel(Channel *channel) {
        Poller::assertInLoopThread();
        int fd = channel->getFd();
        logd("fd = {}", fd);
        assert(m_channels.find(fd) != m_channels.end());
        assert(m_channels[fd] == channel);
        assert(channel->isNoneEvent());
        int index = channel->index();
        assert(index == kAdded || index == kDeleted);
        size_t n = m_channels.erase(fd);
        assert(n == 1);

        if (index == kAdded) {
            update(EPOLL_CTL_DEL, channel);
        }
        channel->setIndex(kNew);
    }
}
