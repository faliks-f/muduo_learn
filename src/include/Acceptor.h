#ifndef MUDUO_LEARN_ACCEPTOR_H
#define MUDUO_LEARN_ACCEPTOR_H

#include "src/include/Channel.h"
#include "src/include/Socket.h"

#include <functional>

namespace faliks {
    class EventLoop;

    class InetAddress;

    class Acceptor : NoneCopyable {
    private:
        using NewConnectionCallback = std::function<void(int sockfd, const InetAddress &)>;
        EventLoop *m_loop;
        Socket m_acceptSocket;
        Channel m_acceptChannel;
        NewConnectionCallback m_newConnectionCallback;
        bool m_listening;
        int m_idleFd;

        void handleRead();

    public:
        Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport);

        ~Acceptor();

        void setNewConnectionCallback(const NewConnectionCallback &cb) {
            m_newConnectionCallback = cb;
        }

        void listen();

        [[nodiscard]] bool listening() const { return m_listening; }
    };
}


#endif //MUDUO_LEARN_ACCEPTOR_H
