#include "src/include/TcpConnection.h"
#include "src/include/EventLoop.h"
#include "src/include/InetAddress.h"
#include "src/include/Socket.h"
#include "src/include/Channel.h"
#include "src/include/Buffer.h"
#include "base/include/fmtlog.h"

#include <netinet/tcp.h>

using namespace faliks;
using namespace std;


void TcpConnection::handleRead(Timestamp receiveTime) {
    m_loop->assertInLoopThread();
    int savedErrno = 0;
    ssize_t n = m_inputBuffer.readFd(m_channel->getFd(), &savedErrno);
    if (n > 0) {
        m_messageCallback(shared_from_this(), &m_inputBuffer, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        loge("TcpConnection::handleRead");
        handleError();
    }
}

void TcpConnection::handleWrite() {
    m_loop->assertInLoopThread();

    if (m_channel->isWriting()) {
        ssize_t n = ::write(m_channel->getFd(),
                            m_outputBuffer.peek(),
                            m_outputBuffer.readableBytes());
        if (n > 0) {
            m_outputBuffer.retrieve(n);
            if (m_outputBuffer.readableBytes() == 0) {
                m_channel->disableWriting();
                if (m_writeCompleteCallback) {
                    m_loop->queueInLoop([this, self = shared_from_this()]() {
                        m_writeCompleteCallback(self);
                    });
                }
                if (m_state == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            loge("TcpConnection::handleWrite");
        }
    } else {
        logw("Connection fd = {} is down, no more writing", m_channel->getFd());
    }
}

void TcpConnection::handleClose() {
    m_loop->assertInLoopThread();
    loge("fd = {} state = {}", m_channel->getFd(), stateToString());
    assert(m_state == kConnected || m_state == kDisconnecting);
    setState(kDisconnected);
    m_channel->disableAll();
}

void TcpConnection::handleError() {
    int optval = 0;
    socklen_t optlen = sizeof optval;

    if (getsockopt(m_channel->getFd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        optval = errno;
    }
    loge("TcpConnection::handleError [{}]", strerror(optval));
}

void TcpConnection::sendInLoop(const std::string &message) {
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void *message, size_t len) {
    m_loop->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (m_state == kDisconnecting) {
        logw("disconnected, give up writing");
        return;
    }

    if (!m_channel->isWriting() && m_outputBuffer.readableBytes() == 0) {
        nwrote = ::write(m_channel->getFd(), message, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && m_writeCompleteCallback) {
                m_loop->queueInLoop([this, self = shared_from_this()]() {
                    m_writeCompleteCallback(self);
                });
            }
        } else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                loge("TcpConnection::sendInLoop");
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }

        assert(remaining <= len);
        if (!faultError && remaining > 0) {
            auto oldLen = static_cast<ssize_t>(m_outputBuffer.readableBytes());
            if (oldLen + remaining >= m_highWaterMark
                && oldLen < m_highWaterMark
                && m_highWaterMarkCallback) {
                m_loop->queueInLoop([this, self = shared_from_this(), oldLen, remaining]() {
                    m_highWaterMarkCallback(self, oldLen + remaining);
                });
            }
            m_outputBuffer.append(static_cast<const char *>(message) + nwrote, remaining);
            if (!m_channel->isWriting()) {
                m_channel->enableWriting();
            }
        }
    }
}

void TcpConnection::shutdownInLoop() {
    m_loop->assertInLoopThread();
    if (!m_channel->isWriting()) {
        m_socket->shutdownWrite();
    }
}

void TcpConnection::forceCloseInLoop() {
    m_loop->assertInLoopThread();
    if (m_state == kConnected || m_state == kDisconnecting) {
        handleClose();
    }
}

const char *TcpConnection::stateToString() const {
    constexpr const char *states[] = {
            "kDisconnected", "kConnecting", "kConnected", "kDisconnecting"
    };
    if (m_state < 0) {
        return "unknown state";
    }
    return states[m_state];
}

void TcpConnection::startReadInLoop() {
    m_loop->assertInLoopThread();
    if (!m_reading || !m_channel->isReading()) {
        m_channel->enableReading();
        m_reading = true;
    }
}

void TcpConnection::stopReadInLoop() {
    m_loop->assertInLoopThread();
    if (m_reading || m_channel->isReading()) {
        m_channel->disableReading();
        m_reading = false;
    }
}

TcpConnection::TcpConnection(EventLoop *loop, const string &name, int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
        : m_loop(loop),
          m_name(name),
          m_state(kConnecting),
          m_reading(true),
          m_socket(make_unique<Socket>(sockfd)),
          m_channel(make_unique<Channel>(loop, sockfd)),
          m_localAddr(localAddr),
          m_peerAddr(peerAddr),
          m_highWaterMark(64 * 1024 * 1024) {
    m_channel->setReadCallback([this](Timestamp receiveTime) {
        handleRead(receiveTime);
    });
    m_channel->setWriteCallback([this]() {
        handleWrite();
    });
    m_channel->setCloseCallback([this]() {
        handleClose();
    });
    m_channel->setErrorCallback([this]() {
        handleError();
    });
    logd("TcpConnection::ctor[{}] fd = {}", m_name, sockfd);
    m_socket->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    logd("TcpConnection::dtor[{}] fd = {} state = {}", m_name, m_channel->getFd(), stateToString());
    assert(m_state == kDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info *tcpInfo) const {
    return m_socket->getTcpInfo(tcpInfo);
}

std::string TcpConnection::getTcpInfoString() const {
    static char buf[1024];
    buf[0] = '\0';
    m_socket->getTcpInfoString(buf, sizeof buf);
    return buf;
}

void TcpConnection::send(const void *message, int len) {
    send(string(static_cast<const char *>(message), len));
}

void TcpConnection::send(const string &message) {
    if (m_state == kConnected) {
        if (m_loop->isInLoopThread()) {
            sendInLoop(message);
        } else {
            m_loop->runInLoop([this, self = shared_from_this(), message]() {
                sendInLoop(message);
            });
        }
    }
}

void TcpConnection::send(Buffer *message) {
    if (m_state == kConnected) {
        if (m_loop->isInLoopThread()) {
            sendInLoop(message->peek(), message->readableBytes());
            message->retrieveAll();
        } else {
            m_loop->runInLoop([this, self = shared_from_this(), message]() {
                sendInLoop(message->peek(), message->readableBytes());
                message->retrieveAll();
            });
        }
    }
}

void TcpConnection::shutdown() {
    if (m_state == kConnected) {
        setState(kDisconnecting);
        m_loop->runInLoop([this]() {
            shutdownInLoop();
        });
    }
}

void TcpConnection::forceClose() {
    if (m_state == kConnected || m_state == kDisconnecting) {
        setState(kDisconnecting);
        m_loop->queueInLoop([this]() {
            forceCloseInLoop();
        });
    }
}

void TcpConnection::forceCloseWithDelay(double seconds) {
    if (m_state == kConnected || m_state == kDisconnecting) {
        setState(kDisconnecting);
        m_loop->runAfter(seconds, [this]() {
            forceClose();
        });
    }
}

void TcpConnection::setTcpNoDelay(bool on) {
    m_socket->setTcpNoDelay(on);
}

void TcpConnection::startRead() {
    m_loop->runInLoop([this]() {
        startReadInLoop();
    });
}

void TcpConnection::stopRead() {
    m_loop->runInLoop([this]() {
        stopReadInLoop();
    });
}

void TcpConnection::connectEstablished() {
    m_loop->assertInLoopThread();
    assert(m_state == kConnecting);
    setState(kConnected);
    m_channel->tie(shared_from_this());
    m_channel->enableReading();
    m_connectionCallback(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    m_loop->assertInLoopThread();
    if (m_state == kConnected) {
        setState(kDisconnected);
        m_channel->disableAll();

        m_connectionCallback(shared_from_this());
    }
    m_channel->remove();
}



