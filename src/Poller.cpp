#include "src/include/Poller.h"
#include "src/include/EventLoop.h"
#include "src/include/Channel.h"
#include "src/include/EPollPoller.h"

namespace faliks {
    Poller::Poller(EventLoop *loop)
            : m_loop(loop) {

    }

    bool Poller::hasChannel(Channel *channel) const {
        assertInLoopThread();
        auto it = m_channels.find(channel->getFd());
        return it != m_channels.end() && it->second == channel;
    }

    Poller *Poller::newDefaultPoller(EventLoop *loop) {
        return new EPollPoller(loop);
    }

    void Poller::assertInLoopThread() const {
        m_loop->assertInLoopThread();
    }
}