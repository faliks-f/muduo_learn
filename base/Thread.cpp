#include "base/include/Thread.h"
#include "base/include/CurrentThread.h"

#include <sys/syscall.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <exception>
#include <utility>

namespace faliks {

    std::atomic<int64_t> faliks::Thread::m_numCreated(0);

    class ThreadData {
    public:
        using ThreadFunc = Thread::ThreadFunc;
        ThreadFunc m_func;
        std::string m_name;
        pid_t *m_tid;
        CountDownLatch *m_latch;

        ThreadData(ThreadFunc func, std::string name, pid_t *tid, CountDownLatch *latch)
                : m_func(std::move(func)),
                  m_name(std::move(name)),
                  m_tid(tid),
                  m_latch(latch) {}

        void runInThread() {
            *m_tid = CurrentThread::tid();
            m_tid = nullptr;
            m_latch->countDown();
            m_latch = nullptr;
            CurrentThread::m_threadName = m_name.empty() ? "Thread" : m_name.c_str();
            ::prctl(PR_SET_NAME, CurrentThread::m_threadName);
            try {
                m_func();
                CurrentThread::m_threadName = "finished";
            } catch (const std::exception &e) {
                CurrentThread::m_threadName = "crashed";
                fprintf(stderr, "exception caught in Thread %s\n", m_name.c_str());
                fprintf(stderr, "reason: %s\n", e.what());
                abort();
            } catch (...) {
                CurrentThread::m_threadName = "crashed";
                fprintf(stderr, "unknown exception caught in Thread %s\n", m_name.c_str());
                throw;
            }
        }
    };

    void *startThread(void *obj) {
        auto *data = static_cast<ThreadData *>(obj);
        data->runInThread();
        delete data;
        return nullptr;
    }


    void CurrentThread::cacheTid() {
        if (m_cachedTid == 0) {
            m_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
            m_tidStringLength = snprintf(m_tidString, sizeof(m_tidString), "%5d ", m_cachedTid);
        }
    }

    Thread::Thread(Thread::ThreadFunc func, const std::string &name)
            : m_started(false),
              m_joined(false),
              m_pthreadId(0),
              m_tid(0),
              m_func(std::move(func)),
              m_name(name),
              m_latch(1) {
        setDefaultName();
    }

    void Thread::setDefaultName() {
        int num = static_cast<int>(m_numCreated.fetch_add(1));
        if (m_name.empty()) {
            char buf[32];
            snprintf(buf, sizeof(buf), "Thread%d", num);
            m_name = buf;
        }
    }

    Thread::~Thread() {
        if (m_started && !m_joined) {
            pthread_detach(m_pthreadId);
        }
    }

    void Thread::start() {
        assert(!m_started);
        m_started = true;
        auto *data = new ThreadData(m_func, m_name, &m_tid, &m_latch);
        if (pthread_create(&m_pthreadId, nullptr, &startThread, data)) {
            m_started = false;
            delete data;
            fprintf(stderr, "Failed in pthread_create\n");
        } else {
            m_latch.wait();
            assert(m_tid > 0);
        }
    }

    int Thread::join() {
        assert(m_started);
        assert(!m_joined);
        m_joined = true;
        return pthread_join(m_pthreadId, nullptr);
    }
}
