#include "base/include/Thread.h"
#include "base/include/CurrentThread.h"

#include <sys/syscall.h>
#include <unistd.h>

namespace faliks{
    void CurrentThread::cacheTid() {
        if (m_cachedTid == 0) {
            m_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
            m_tidStringLength = snprintf(m_tidString, sizeof(m_tidString), "%5d ", m_cachedTid);
        }
    }
}
