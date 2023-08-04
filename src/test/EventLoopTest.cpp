#include "src/include/EventLoop.h"
#include "base/include/Thread.h"
#include "base/include/fmtlog.h"

#include <cassert>
#include <cstdio>
#include <unistd.h>

using namespace faliks;

EventLoop *g_loop;

void callback() {
    logi("callback(): pid = {}, tid = {}\n", getpid(), CurrentThread::tid());
    EventLoop anotherLoop;
}

void threadFunc() {
    logi("threadFunc(): pid = {}, tid = {}\n", getpid(), CurrentThread::tid());

    assert(EventLoop::getEventLoopOfCurrentThread() == nullptr);
    EventLoop loop;
    assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
    loop.runAfter(1.0, callback);
    loop.loop();
}

int main() {
    fmtlog::startPollingThread(1e8);
    logi("main(): pid = {}, tid = {}\n", getpid(), CurrentThread::tid());

    assert(EventLoop::getEventLoopOfCurrentThread() == nullptr);
    EventLoop loop;
    assert(EventLoop::getEventLoopOfCurrentThread() == &loop);

    Thread thread(threadFunc);
    thread.start();

    loop.loop();
}