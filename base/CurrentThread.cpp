#include "include/CurrentThread.h"

#include <execinfo.h>
#include <cxxabi.h>

namespace faliks {
    namespace CurrentThread {

        using std::string;

        __thread int m_cachedTid = 0;
        __thread char m_tidString[32];
        __thread int m_tidStringLength = 6;
        __thread const char *m_threadName = "unknown";

        string stackTrace(bool demangle) {
            string stack;
            const int max_frames = 200;
            void *frame[max_frames];
            int nptrs = ::backtrace(frame, max_frames);
            char **strings = ::backtrace_symbols(frame, nptrs);
            if (strings) {
                size_t len = 256;
                char *demangled = demangle ? static_cast<char *>(::malloc(len)) : nullptr;
                for (int i = 1; i < nptrs; ++i) {
                    if (demangle) {
                        char *left_par = nullptr;
                        char *plus = nullptr;
                        for (char *p = strings[i]; *p; ++p) {
                            if (*p == '(') {
                                left_par = p;
                            } else if (*p == '+') {
                                plus = p;
                            }

                            if (left_par && plus) {
                                *plus = '\0';
                                int status = 0;
                                char *ret = abi::__cxa_demangle(left_par + 1, demangled, &len, &status);
                                *plus = '+';
                                if (status == 0) {
                                    demangled = ret;
                                    stack.append(strings[i], left_par + 1);
                                    stack.append(demangled);
                                    stack.append(plus);
                                    stack.push_back('\n');
                                    continue;
                                }
                            }
                        }
                        stack.append(strings[i]);
                        stack.push_back('\n');
                    }

                }
                free(demangled);
                free(strings);
            }
            return stack;
        }
    }
}
