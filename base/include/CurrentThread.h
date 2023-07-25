//
// Created by faliks on 23-7-25.
//

#ifndef MUDUO_LEARN_CURRENTTHREAD_H
#define MUDUO_LEARN_CURRENTTHREAD_H

#include <cstdint>
#include <string>

namespace faliks {
    namespace CurrentThread {
        extern __thread int m_cachedTid;
        extern __thread char m_tidString[32];
        extern __thread int m_tidStringLength;
        extern __thread const char *m_threadName;

        void cacheTid();

        inline int tid() {
            if (__builtin_expect(m_cachedTid == 0, 0)) {
                cacheTid();
            }
            return m_cachedTid;
        }

        inline const char *tidString() {
            return m_tidString;
        }

        inline int tidStringLength() {
            return m_tidStringLength;
        }

        inline const char *name() {
            return m_threadName;
        }

        bool isMainThread();

        void sleepUs(int64_t us);

        std::string stackTrace(bool demangle);
    }
}


#endif //MUDUO_LEARN_CURRENTTHREAD_H
