//
// Created by faliks on 23-7-27.
//

#ifndef MUDUO_LEARN_EPOLLPOLLER_H
#define MUDUO_LEARN_EPOLLPOLLER_H

#include "src/include/Poller.h"

#include <vector>
#include <sys/epoll.h>

namespace faliks {
    class EPollPoller : public Poller {
    private:
        typedef std::vector<struct epoll_event> EventList;

        int m_epollFd;
        EventList m_events;

        static constexpr int kInitEventListSize = 16;

        static const char *operationToString(int op);

        void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

        void update(int operation, Channel *channel);

    public:
        explicit EPollPoller(EventLoop *loop);

        ~EPollPoller() override;

        Timestamp poll(int timeoutMs, ChannelList *activeChannels) override;

        void updateChannel(Channel *channel) override;

        void removeChannel(Channel *channel) override;
    };
}


#endif //MUDUO_LEARN_EPOLLPOLLER_H
