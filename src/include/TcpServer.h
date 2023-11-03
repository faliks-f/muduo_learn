#ifndef MUDUO_LEARN_TCPSERVER_H
#define MUDUO_LEARN_TCPSERVER_H

#include "src/include/TcpConnection.h"

#include <atomic>
#include <memory>
#include <string>
#include <functional>
#include <map>

namespace faliks {

    class Acceptor;

    class EventLoop;

    class ThreadPool;

    class TcpServer : NoneCopyable {
    private:
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
        using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
        using ThreadInitCallback = std::function<void(EventLoop *)>;
        using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)>;
        using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
        EventLoop *m_loop;
        const std::string m_ipPort;
        const std::string m_name;
        std::unique_ptr<Acceptor> m_acceptor;
        std::shared_ptr<ThreadPool> m_threadPool;
        ConnectionCallback m_connectionCallback;
        WriteCompleteCallback m_writeCompleteCallback;
        ThreadInitCallback m_threadInitCallback;
        MessageCallback m_messageCallback;

        std::atomic<int32_t> m_started;
        int m_nextConnId;
        ConnectionMap m_connections;

        void newConnection(int sockfd, const InetAddress &peerAddr);

        void removeConnection(const TcpConnectionPtr &conn);

        void removeConnectionInLoop(const TcpConnectionPtr &conn);

    public:
        enum class Option {
            kNoReusePort,
            kReusePort
        };

        TcpServer(EventLoop *loop, const InetAddress &listenAddr, const std::string &nameArg,
                  Option option = Option::kNoReusePort);

        ~TcpServer();

        [[nodiscard]] const std::string &ipPort() const { return m_ipPort; }

        [[nodiscard]] const std::string &name() const { return m_name; }

        [[nodiscard]] EventLoop *getLoop() const { return m_loop; }

        void setThreadNum(int numThreads);

        void setThreadInitCallback(const ThreadInitCallback &cb) { m_threadInitCallback = cb; }

        std::shared_ptr<ThreadPool> threadPool() { return m_threadPool; }

        void start();

        void setConnectionCallback(const ConnectionCallback &cb) { m_connectionCallback = cb; }

        void setMessageCallback(const MessageCallback &cb) { m_messageCallback = cb; }
    };
}


#endif //MUDUO_LEARN_TCPSERVER_H
