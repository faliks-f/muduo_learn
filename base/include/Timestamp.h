#ifndef MUDUO_LEARN_TIMESTAMP_H
#define MUDUO_LEARN_TIMESTAMP_H

#include "base/include/Copyable.h"

#include <boost/operators.hpp>

namespace faliks {


    class Timestamp : public Copyable,
                      public boost::equality_comparable<Timestamp>,
                      public boost::less_than_comparable<Timestamp> {
    private:
        int64_t m_microSecondsSinceEpoch;

    public:
        static constexpr int kMicroSecondsPerSecond = 1000 * 1000;

        Timestamp() : m_microSecondsSinceEpoch(0) {}

        explicit Timestamp(int64_t microSecondsSinceEpoch)
                : m_microSecondsSinceEpoch(microSecondsSinceEpoch) {}

        void swap(Timestamp &that) {
            std::swap(m_microSecondsSinceEpoch, that.m_microSecondsSinceEpoch);
        }

        [[nodiscard]] std::string toString() const;

        [[nodiscard]] std::string toFormattedString(bool showMicroseconds = true) const;

        [[nodiscard]] bool valid() const {
            return m_microSecondsSinceEpoch > 0;
        }

        [[nodiscard]] int64_t microSecondsSinceEpoch() const {
            return m_microSecondsSinceEpoch;
        }

        [[nodiscard]] time_t secondsSinceEpoch() const {
            return static_cast<time_t>(m_microSecondsSinceEpoch / kMicroSecondsPerSecond);
        }

        [[nodiscard]] static Timestamp now();

        static Timestamp invalid() {
            return {};
        }

        static Timestamp fromUnixTime(time_t t) {
            return fromUnixTime(t, 0);
        }

        static Timestamp fromUnixTime(time_t t, int microseconds) {
            return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond + microseconds);
        }

        bool operator<(Timestamp rhs) const {
            return m_microSecondsSinceEpoch < rhs.m_microSecondsSinceEpoch;
        }

        bool operator==(Timestamp rhs) const {
            return m_microSecondsSinceEpoch == rhs.m_microSecondsSinceEpoch;
        }
    };

    inline double timeDifference(Timestamp high, Timestamp low) {
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
    }

    inline Timestamp addTime(Timestamp timestamp, double seconds) {
        auto delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
        return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
    }
}


#endif //MUDUO_LEARN_TIMESTAMP_H
