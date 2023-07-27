#include "src/include/Poller.h"
#include "src/include/EventLoop.h"
#include "src/include/Channel.h"

faliks::Poller::Poller(faliks::EventLoop *loop)
        : m_loop(loop) {

}

bool faliks::Poller::hasChannel(faliks::Channel *channel) const {
    assertInLoopThread();
    auto it = m_channels.find(channel->getFd());
    return it != m_channels.end() && it->second == channel;
}

faliks::Poller *faliks::Poller::newDefaultPoller(faliks::EventLoop *loop) {
    return nullptr;
}

void faliks::Poller::assertInLoopThread() const {
//    m_loop->assertInLoopThread();
}
