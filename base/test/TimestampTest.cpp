#include "base/include/Timestamp.h"
#include "base/include/fmtlog.h"

#include <vector>
#include <cstdio>

using namespace std;
using faliks::Timestamp;

void passByConstReference(const Timestamp &x) {
    logi("{}", x.toString().c_str());
}

void passByValue(Timestamp x) {
    logi("{}", x.toString().c_str());
}

void benchmark() {
    constexpr int kNumber = 1000 * 10;

    vector<Timestamp> stamps;
    stamps.reserve(kNumber);
    for (int i = 0; i < kNumber; ++i) {
        stamps.push_back(Timestamp::now());
    }
    logi("start time: {}", stamps.front().toString().c_str());
    logi("finish time: {}", stamps.back().toString().c_str());
    logi( "diff time: {}", timeDifference(stamps.back(), stamps.front()));

    int increments[100] = {0};
    int64_t start = stamps.front().microSecondsSinceEpoch();
    for (int i = 1; i < 100; ++i) {
        int64_t next = stamps[i].microSecondsSinceEpoch();
        int64_t inc = next - start;
        start = next;
        if (inc < 0) {
            logi("reverse!\n");
        } else if (inc < 100) {
            ++increments[inc];
        } else {
            logi("big gap {}", static_cast<int>(inc));
        }
    }

    for (int x = 0; x < 100; ++x) {
        logi("{}: {}", x, increments[x]);
    }
}

int main() {
    fmtlog::startPollingThread(1e8);
    Timestamp now(Timestamp::now());
    logi("%s\n", now.toString().c_str());
    passByValue(now);
    passByConstReference(now);
    benchmark();
    return 0;
}
