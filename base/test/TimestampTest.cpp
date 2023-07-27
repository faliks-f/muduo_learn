#include "base/include/Timestamp.h"

#include <vector>
#include <cstdio>

using namespace std;
using faliks::Timestamp;

void passByConstReference(const Timestamp &x) {
    printf("%s\n", x.toString().c_str());
}

void passByValue(Timestamp x) {
    printf("%s\n", x.toString().c_str());
}

void benchmark() {
    constexpr int kNumber = 1000 * 10;

    vector<Timestamp> stamps;
    stamps.reserve(kNumber);
    for (int i = 0; i < kNumber; ++i) {
        stamps.push_back(Timestamp::now());
    }
    printf("start time: %s\n", stamps.front().toString().c_str());
    printf("finish time: %s\n", stamps.back().toString().c_str());
    printf( "diff time: %f\n", timeDifference(stamps.back(), stamps.front()));

    int increments[100] = {0};
    int64_t start = stamps.front().microSecondsSinceEpoch();
    for (int i = 1; i < 100; ++i) {
        int64_t next = stamps[i].microSecondsSinceEpoch();
        int64_t inc = next - start;
        start = next;
        if (inc < 0) {
            printf("reverse!\n");
        } else if (inc < 100) {
            ++increments[inc];
        } else {
            printf("big gap %d\n", static_cast<int>(inc));
        }
    }

    for (int x = 0; x < 100; ++x) {
        printf("%2d: %d\n", x, increments[x]);
    }
}

int main() {
    Timestamp now(Timestamp::now());
    printf("%s\n", now.toString().c_str());
    passByValue(now);
    passByConstReference(now);
    benchmark();
    return 0;
}
