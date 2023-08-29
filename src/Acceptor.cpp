#include "src/include/Acceptor.h"
#include "src/include/EventLoop.h"
#include "src/include/InetAddress.h"
#include "base/include/fmtlog.h"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

using namespace faliks;

namespace faliks {
    inline void close(int fd) {
        if (::close(fd) < 0) {
            loge("close error: {}", strerror(errno));
        }
    }

    inline int createNonblockingOrDie(sa_family_t family) {
        int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
        if (sockfd < 0) {
            loge("createNonblockingOrDie error: {}", strerror(errno));
        }
        return sockfd;
    }
}

void Acceptor::handleRead() {
    m_loop->assertInLoopThread();

    InetAddress peerAddr;

    int connfd = m_acceptSocket.accept(&peerAddr);
    if (connfd > 0) {
        if (m_newConnectionCallback) {
            m_newConnectionCallback(connfd, peerAddr);
        } else {
            close(connfd);
        }
    } else {
        loge("accept error: {}", strerror(errno));
        if (errno == EMFILE) {
            close(m_idleFd);
            m_idleFd = accept(m_acceptSocket.fd(), nullptr, nullptr);
            close(m_idleFd);
            m_idleFd = open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reuseport)
        : m_loop(loop),
          m_acceptSocket(createNonblockingOrDie(listenAddr.family())),
          m_acceptChannel(loop, m_acceptSocket.fd()),
          m_listening(false),
          m_idleFd(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(m_idleFd >= 0);
    m_acceptSocket.setReuseAddr(true);
    m_acceptSocket.setReusePort(reuseport);
    m_acceptSocket.bindAddress(listenAddr);
    m_acceptChannel.setReadCallback([this](Timestamp) {
        handleRead();
    });
}

Acceptor::~Acceptor() {
    m_acceptChannel.disableAll();
    m_acceptChannel.remove();
    close(m_idleFd);
}

void Acceptor::listen() {
    m_loop->assertInLoopThread();
    m_listening = true;
    m_acceptSocket.listen();
    m_acceptChannel.enableReading();
}

