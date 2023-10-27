#include "src/include/Socket.h"
#include "base/include/fmtlog.h"
#include "src/include/InetAddress.h"

#include <unistd.h>
#include <netinet/tcp.h>

namespace faliks {
    Socket::~Socket() {
        if (close(m_sockFd) < 0) {
            loge("close socket failed");
        }
    }

    bool Socket::getTcpInfo(struct tcp_info *tcpi) const {
        socklen_t len = sizeof(*tcpi);
        memset(tcpi, 0, len);
        return ::getsockopt(m_sockFd, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
    }

    bool Socket::getTcpInfoString(char *buf, int len) const {
        struct tcp_info tcpi;
        bool ok = getTcpInfo(&tcpi);
        if (ok) {
            snprintf(buf, len, "unrecovered=%u "
                               "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                               "lost=%u retrans=%u rtt=%u rttvar=%u "
                               "sshthresh=%u cwnd=%u total_retrans=%u",
                     tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
                     tcpi.tcpi_rto,          // Retransmit timeout in usec
                     tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
                     tcpi.tcpi_snd_mss,
                     tcpi.tcpi_rcv_mss,
                     tcpi.tcpi_lost,         // Lost packets
                     tcpi.tcpi_retrans,      // Retransmitted packets out
                     tcpi.tcpi_rtt,          // Smoothed round trip time in usec
                     tcpi.tcpi_rttvar,       // Medium deviation
                     tcpi.tcpi_snd_ssthresh,
                     tcpi.tcpi_snd_cwnd,
                     tcpi.tcpi_total_retrans);  // Total retransmits for entire connection

        }
        return ok;
    }

    void Socket::bindAddress(const InetAddress &localAddr) const {
        if (::bind(m_sockFd, localAddr.getSockAddr(), sizeof(sockaddr_in6)) < 0) {
            loge("bind socket failed");
        }
    }

    void Socket::listen() {
        int ret = ::listen(m_sockFd, SOMAXCONN);
        if (ret < 0) {
            loge("listen socket failed");
        }
    }

    int Socket::accept(InetAddress *peeraddr) const {
        struct sockaddr_in6 addr{};
        memset(&addr, 0, sizeof addr);
        auto addrLen = static_cast<socklen_t>(sizeof addr);
        int connfd = ::accept4(m_sockFd, (sockaddr *) &addr, &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (connfd < 0) {
            int savedErrno = errno;
            loge("accept socket failed");
            switch (savedErrno) {
                case EAGAIN:
                case ECONNABORTED:
                case EINTR:
                case EPROTO: // ???
                case EPERM:
                case EMFILE: // per-process limit of open file desctiptor ???
                    // expected errors
                    errno = savedErrno;
                    break;
                case EBADF:
                case EFAULT:
                case EINVAL:
                case ENFILE:
                case ENOBUFS:
                case ENOMEM:
                case ENOTSOCK:
                case EOPNOTSUPP:
                    // unexpected errors
                    loge("unexpected error of ::accept {}", savedErrno);
                    break;
                default:
                    loge("unknown error of ::accept {}", savedErrno);
                    break;
            }
        } else {
            peeraddr->setSockAddrInet6(addr);
        }
        return connfd;
    }

    void Socket::shutdownWrite() {
        if (::shutdown(m_sockFd, SHUT_WR) < 0) {
            loge("shutdown socket failed");
        }
    }

    void Socket::setTcpNoDelay(bool on) const {
        int optVal = on ? 1 : 0;
        setsockopt(m_sockFd, IPPROTO_TCP, TCP_NODELAY, &optVal, static_cast<socklen_t>(sizeof optVal));
    }

    void Socket::setReuseAddr(bool on) const {
        int optVal = on ? 1 : 0;
        setsockopt(m_sockFd, SOL_SOCKET, SO_REUSEPORT, &optVal, static_cast<socklen_t>(sizeof optVal));
    }

    void Socket::setReusePort(bool on) const {
        int optVal = on ? 1 : 0;
        int ret = setsockopt(m_sockFd, SOL_SOCKET, SO_REUSEPORT, &optVal, static_cast<socklen_t>(sizeof optVal));

        if (ret < 0 && on) {
            loge("set reuse port failed");
        }
    }

    void Socket::setKeepAlive(bool on) {
        int optVal = on ? 1 : 0;
        int ret = setsockopt(m_sockFd, SOL_SOCKET, SO_KEEPALIVE, &optVal, static_cast<socklen_t>(sizeof optVal));
        if (ret < 0 && on) {
            loge("set keep alive failed");
        }
    }


} // faliks