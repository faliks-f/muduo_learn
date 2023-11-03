#include "src/include/TcpServer.h"
#include "base/include/fmtlog.h"
#include "src/include/EventLoop.h"
#include "src/include/InetAddress.h"

#include "unistd.h"
#include <memory>

using namespace std;
using namespace faliks;

int numThreads = 0;

class EchoServer {
private:
    EventLoop *m_loop;
    TcpServer server;

    void onConnection(const shared_ptr<TcpConnection> &conn) {
        logi("{} -> {} is {}",
             conn->peerAddress().toIpPort(),
             conn->localAddress().toIpPort(),
             conn->connected() ? "UP" : "DOWN");
        logi("{}", conn->getTcpInfoString());

        conn->send("hello\n");
    }

    void onMessage(const shared_ptr<TcpConnection> &conn, Buffer *buf, Timestamp time) {
        string msg(buf->retrieveAllAsString());
        logi("{} recv {} bytes at {}", conn->getName(), msg.size(), time.toString());
        if (msg == "exit\n") {
            conn->send("bye\n");
            conn->shutdown();
        }
        if (msg == "quit\n") {
            m_loop->quit();
        } else {
            conn->send(msg);
        }
    }

public:
    EchoServer(EventLoop *loop, const InetAddress &listenAddr)
            : m_loop(loop),
              server(loop, listenAddr, "EchoServer") {
        server.setConnectionCallback([this](auto &&PH1) { onConnection(std::forward<decltype(PH1)>(PH1)); });
        server.setMessageCallback([this](auto &&PH1, auto &&PH2, auto &&PH3) {
            onMessage(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2),
                      std::forward<decltype(PH3)>(PH3));
        });
        server.setThreadNum(numThreads);
    }

    void start() {
        server.start();
    }
};

int main(int argc, char **argv) {
    fmtlog::startPollingThread(1e8);

    logi("pid = {}", getpid());
    logi("sizeof TcpConnection = {}", sizeof(TcpConnection));
    if (argc > 1) {
        numThreads = atoi(argv[1]);
    }
    bool ipv6 = argc > 2;
    EventLoop loop;
    InetAddress listenAddr(2000, true, ipv6);
    EchoServer server(&loop, listenAddr);

    server.start();
    loop.loop();
}