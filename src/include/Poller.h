#ifndef MUDUO_LEARN_POLLER_H
#define MUDUO_LEARN_POLLER_H

#include "base/include/NoneCopyable.h"
#include "base/include/Timestamp.h"

#include <vector>
#include <map>


namespace faliks {

    class Channel;

    class EventLoop;

    class Poller : public NoneCopyable {
    private:
        EventLoop *m_loop;
    protected:
        typedef std::vector<Channel *> ChannelList;
        typedef std::map<int, Channel *> ChannelMap;
        ChannelMap m_channels;
    public:
        explicit Poller(EventLoop *loop);

        virtual ~Poller() = default;

        virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;

        virtual void updateChannel(Channel *channel) = 0;

        virtual void removeChannel(Channel *channel) = 0;

        virtual bool hasChannel(Channel *channel) const;

        static Poller *newDefaultPoller(EventLoop *loop);

        void assertInLoopThread() const;
    };
}


#endif //MUDUO_LEARN_POLLER_H
