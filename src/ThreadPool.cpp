#include "src/include/ThreadPool.h"
#include "src/include/EventLoop.h"
#include "src/include/EventLoopThread.h"

using namespace faliks;
using namespace std;

ThreadPool::ThreadPool(EventLoop *baseLoop, const string &nameArg) :
        m_baseLoop(baseLoop),
        m_name(nameArg),
        m_started(false),
        m_numThreads(0),
        m_next(0) {

}

ThreadPool::~ThreadPool() {

}

void ThreadPool::start(const ThreadInitCallback &cb) {
    assert(!m_started);
    m_baseLoop->assertInLoopThread();

    m_started = true;

    for (int i = 0; i < m_numThreads; ++i) {
        char buf[m_name.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", m_name.c_str(), i);
        EventLoopThread *t = new EventLoopThread(cb, buf);
        m_threads.push_back(std::unique_ptr<EventLoopThread>(t));
        m_loops.push_back(t->startLoop());
    }
    if (m_numThreads == 0 && cb) {
        cb(m_baseLoop);
    }
}

EventLoop *ThreadPool::getNextLoop() {
    m_baseLoop->assertInLoopThread();
    assert(m_started);

    EventLoop *loop = m_baseLoop;

    if (!m_loops.empty()) {
        loop = m_loops[m_next];
        ++m_next;
        if (static_cast<size_t>(m_next) >= m_loops.size()) {
            m_next = 0;
        }
    }
    return loop;
}

EventLoop *ThreadPool::getLoopForHash(size_t hashCode) {
    m_baseLoop->assertInLoopThread();
    auto loop = m_baseLoop;
    if (!m_loops.empty()) {
        loop = m_loops[hashCode % m_loops.size()];
    }
    return loop;
}

std::vector<EventLoop *> ThreadPool::getAllLoops() {
    m_baseLoop->assertInLoopThread();
    assert(m_started);
    if (m_loops.empty()) {
        return std::vector<EventLoop *>(1, m_baseLoop);
    } else {
        return m_loops;
    }
}


