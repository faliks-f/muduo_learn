#include "src/include/EventLoopThread.h"
#include "src/include/EventLoop.h"

using namespace faliks;
using namespace std;

EventLoopThread::EventLoopThread(const EventLoopThread::ThreadInitCallback &cb, const string &name)
        : m_loop(nullptr),
          m_exiting(false),
          m_thread([this] { threadFunc(); }, name),
          m_mutex(),
          m_cond(),
          m_callback(cb) {
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if (m_callback) {
        m_callback(&loop);
    }
    {
        scoped_lock<mutex> lock(m_mutex);
        m_loop = &loop;
        m_cond.notify_one();
    }

    loop.loop();
    scoped_lock<mutex> lock(m_mutex);
    m_loop = nullptr;
}

EventLoopThread::~EventLoopThread() {
    m_exiting = true;
    if (m_loop) {
        m_loop->quit();
        m_thread.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    assert(!m_thread.started());
    m_thread.start();

    EventLoop *loop = nullptr;
    {
        unique_lock<mutex> lock(m_mutex);
        while (!m_loop) {
            m_cond.wait(lock);
        }
        loop = m_loop
    }
}
