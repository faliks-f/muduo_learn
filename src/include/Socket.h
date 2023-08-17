#ifndef MUDUO_LEARN_SOCKET_H
#define MUDUO_LEARN_SOCKET_H

#include "base/include/NoneCopyable.h"

struct tcp_info;

namespace faliks {

    class InetAddress;

    class Socket : public NoneCopyable {
    private:
        const int m_sockFd;
    public:
        explicit Socket(int sockFd) : m_sockFd(sockFd) {}

        ~Socket();

        [[nodiscard]] int fd() const { return m_sockFd; }

        bool getTcpInfo(struct tcp_info *) const;

        bool getTcpInfoString(char *buf, int len) const;

        void bindAddress(const InetAddress &localAddr) const;

        void listen();

        int accept(InetAddress *peeraddr) const;

        void shutdownWrite();

        void setTcpNoDelay(bool on) const;

        void setReuseAddr(bool on) const;

        void setReusePort(bool on) const;

        void setKeepAlive(bool on);
    };

} // faliks

#endif //MUDUO_LEARN_SOCKET_H
