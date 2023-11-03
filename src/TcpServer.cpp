#include "src/include/TcpServer.h"
#include "src/include/Acceptor.h"
#include "src/include/EventLoop.h"
#include "src/include/ThreadPool.h"
#include "src/include/TcpConnection.h"
#include "src/include/InetAddress.h"
#include "src/include/Buffer.h"
#include "base/include/fmtlog.h"

using namespace faliks;

struct sockaddr_in6 getLocalAddr(int sockfd) {
    struct sockaddr_in6 localAddr{};
    memset(&localAddr, 0, sizeof localAddr);
    socklen_t addrlen = sizeof localAddr;
    auto *addr = reinterpret_cast<struct sockaddr *>(&localAddr);
    if (::getsockname(sockfd, addr, &addrlen) < 0) {
        loge("getLocalAddr");
    }
    return localAddr;
}

void defaultConnectionCallback(const std::shared_ptr<TcpConnection> &conn) {
    logi("{} -> {} is {}", conn->localAddress().toIpPort(), conn->peerAddress().toIpPort(),
         (conn->connected() ? "UP" : "DOWN"));
}

void defaultMessageCallback(const std::shared_ptr<TcpConnection> &conn, Buffer *buffer, Timestamp receiveTime) {
    buffer->retrieveAll();
}


void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    m_loop->assertInLoopThread();
    EventLoop *ioLoop = m_threadPool->getNextLoop();
    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", m_ipPort.c_str(), m_nextConnId);
    ++m_nextConnId;
    std::string connName = m_name + buf;

    logi("TcpServer::newConnection [{}] - new connection [{}] from {}",
         m_name, connName, peerAddr.toIpPort());

    InetAddress localAddr(getLocalAddr(sockfd));
    TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop, connName, sockfd, localAddr, peerAddr);

    m_connections[connName] = conn;
    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    ioLoop->runInLoop([conn]() mutable { conn->connectEstablished(); });

}

void TcpServer::removeConnection(const faliks::TcpServer::TcpConnectionPtr &conn) {
    m_loop->runInLoop([this, conn] { removeConnectionInLoop(conn); });
}

void TcpServer::removeConnectionInLoop(const TcpServer::TcpConnectionPtr &conn) {
    m_loop->assertInLoopThread();
    logi("TcpServer::removeConnectionInLoop [{}] - connection {}", m_name, conn->getName());
    size_t n = m_connections.erase(conn->getName());
    assert(n == 1);
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop([conn] { conn->connectDestroyed(); });
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg,
                     TcpServer::Option option)
        : m_loop(loop),
          m_ipPort(listenAddr.toIpPort()),
          m_name(nameArg),
          m_acceptor(new Acceptor(loop, listenAddr, option == Option::kReusePort)),
          m_threadPool(new ThreadPool(loop, nameArg)),
          m_connectionCallback(defaultConnectionCallback),
          m_messageCallback(defaultMessageCallback),
          m_nextConnId(1) {

}

TcpServer::~TcpServer() {
    m_loop->assertInLoopThread();
    logi("TcpServer::~TcpServer [%s] destructing", m_name);

    for (auto &item: m_connections) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop([conn] { conn->connectDestroyed(); });
    }
}

void TcpServer::setThreadNum(int numThreads) {
    assert(0 <= numThreads);
    m_threadPool->setThreadNum(numThreads);
}

void TcpServer::start() {
    if (m_started.exchange(1) == 0) {
        m_threadPool->start(m_threadInitCallback);

        assert(!m_acceptor->listening());
        m_loop->runInLoop([this] { m_acceptor->listen(); });
    }
}

