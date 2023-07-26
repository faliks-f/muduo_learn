#include "base/include/CountDownLatch.h"

faliks::CountDownLatch::CountDownLatch(int count)
        : m_mutex(),
          m_count(count),
          m_condition() {
}

void faliks::CountDownLatch::wait() {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_condition.wait(lock, [this] { return m_count == 0; });
}

void faliks::CountDownLatch::countDown() {
    std::unique_lock<std::mutex> lock(m_mutex);
    --m_count;
    if (m_count == 0) {
        m_condition.notify_all();
    }
}

int faliks::CountDownLatch::getCount() const {
    std::scoped_lock<std::mutex> lock(m_mutex);
    return m_count;
}
