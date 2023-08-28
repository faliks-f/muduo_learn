#ifndef MUDUO_LEARN_TCPCONNECTION_H
#define MUDUO_LEARN_TCPCONNECTION_H

#include "base/include/NoneCopyable.h"
#include "src/include/InetAddress.h"
#include "src/include/Buffer.h"
#include "base/include/Timestamp.h"


#include <memory>
#include <functional>
#include <string>

struct tcp_info;

namespace faliks {

    class Channel;

    class EventLoop;

    class Socket;

    class TcpConnection : NoneCopyable,
                          public std::enable_shared_from_this<TcpConnection> {
    private:

        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using ConnectionCallback = std::function<void(const TcpConnectionPtr &)>;
        using MessageCallback = std::function<void(const TcpConnectionPtr &, Buffer *, Timestamp)>;
        using WriteCompleteCallback = std::function<void(const TcpConnectionPtr &)>;
        using CloseCallback = std::function<void(const TcpConnectionPtr &)>;
        using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr &, size_t)>;

        enum StateE {
            kDisconnected = 0, kConnecting, kConnected, kDisconnecting
        };

        EventLoop *m_loop;
        const std::string m_name;
        StateE m_state;
        bool m_reading;
        std::unique_ptr<Socket> m_socket;
        std::unique_ptr<Channel> m_channel;
        const InetAddress m_localAddr;
        const InetAddress m_peerAddr;
        ConnectionCallback m_connectionCallback;
        MessageCallback m_messageCallback;
        WriteCompleteCallback m_writeCompleteCallback;
        HighWaterMarkCallback m_highWaterMarkCallback;
        CloseCallback m_closeCallback;
        size_t m_highWaterMark;
        Buffer m_inputBuffer;
        Buffer m_outputBuffer;

        void handleRead(Timestamp receiveTime);

        void handleWrite();

        void handleClose();

        void handleError();

        void sendInLoop(const std::string &message);

        void sendInLoop(const void *message, size_t len);

        void shutdownInLoop();

        void forceCloseInLoop();

        void setState(StateE s) { m_state = s; }

        const char *stateToString() const;

        void startReadInLoop();

        void stopReadInLoop();

    public:
        TcpConnection(EventLoop *loop,
                      const std::string &name,
                      int sockfd,
                      const InetAddress &localAddr,
                      const InetAddress &peerAddr);

        ~TcpConnection();

        EventLoop *getLoop() const { return m_loop; }

        const std::string &getName() const { return m_name; }

        const InetAddress &localAddress() const { return m_localAddr; }

        const InetAddress &peerAddress() const { return m_peerAddr; }

        bool connected() const { return m_state == kConnected; }

        bool disconnected() const { return m_state == kDisconnected; }

        bool getTcpInfo(struct tcp_info *tcpInfo) const;

        std::string getTcpInfoString() const;

        void send(const void *message, int len);

        void send(const std::string &message);

        void send(Buffer *message);

        void shutdown();

        void forceClose();

        void forceCloseWithDelay(double seconds);

        void setTcpNoDelay(bool on);

        void startRead();

        void stopRead();

        bool isReading() const { return m_reading; }

        void setConnectionCallback(const ConnectionCallback &cb) { m_connectionCallback = cb; }

        void setMessageCallback(const MessageCallback &cb) { m_messageCallback = cb; }

        void setWriteCompleteCallback(const WriteCompleteCallback &cb) { m_writeCompleteCallback = cb; }

        void setCloseCallback(const CloseCallback &cb) { m_closeCallback = cb; }

        void setHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark) {
            m_highWaterMarkCallback = cb;
            m_highWaterMark = highWaterMark;
        }

        Buffer *inputBuffer() { return &m_inputBuffer; }

        Buffer *outputBuffer() { return &m_outputBuffer; }

        void connectEstablished();

        void connectDestroyed();
    };

}

#endif //MUDUO_LEARN_TCPCONNECTION_H
