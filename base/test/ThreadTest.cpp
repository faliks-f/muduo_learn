#include "base/include//Thread.h"
#include "base/include/CurrentThread.h"

#include <string>
#include <cstdio>
#include <unistd.h>

void mySleep(int seconds) {
    timespec t = {seconds, 0};
    nanosleep(&t, NULL);
}

void threadFunc() {
    printf("tid=%d\n", faliks::CurrentThread::tid());
}

void threadFunc2(int x) {
    printf("tid=%d, x=%d\n", faliks::CurrentThread::tid(), x);
}

void threadFunc3() {
    printf("tid=%d\n", faliks::CurrentThread::tid());
    mySleep(1);
}

class Foo {
private:
    double m_x;
public:
    explicit Foo(double x) : m_x(x) {}

    void memberFunc() {
        printf("tid=%d, Foo::x_=%f\n", faliks::CurrentThread::tid(), m_x);
    }

    void memberFunc2(const std::string &text) {
        printf("tid=%d, Foo::x_=%f, text=%s\n", faliks::CurrentThread::tid(), m_x, text.c_str());
    }
};

int main() {
    printf("pid=%d, tid=%d\n", ::getpid(), faliks::CurrentThread::tid());
    faliks::Thread t1(threadFunc);
    t1.start();
    printf("t1.tid=%d\n", t1.tid());
    t1.join();

    faliks::Thread t2(std::bind(threadFunc2, 42),
                      "thread for free function with argument");
    t2.start();
    printf("t2.tid=%d\n", t2.tid());
    t2.join();

    Foo foo(87.53);
    faliks::Thread t3(std::bind(&Foo::memberFunc, &foo),
                      "thread for member function without argument");
    t3.start();
    t3.join();

    faliks::Thread t4(std::bind(&Foo::memberFunc2, &foo, std::string("Shuo Chen")));
    t4.start();
    t4.join();

    {
        faliks::Thread t5(threadFunc3);
        t5.start();
        // t5 may destruct eariler than thread creation.
    }
    mySleep(2);
    {
        faliks::Thread t6(threadFunc3);
        t6.start();
        mySleep(2);
        // t6 destruct later than thread creation.
    }
    sleep(2);
    printf("number of created threads %ld\n", faliks::Thread::numCreated());
    return 0;
}

