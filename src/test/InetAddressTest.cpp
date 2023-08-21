#include "src/include/InetAddress.h"
#include "base/include/fmtlog.h"

#include <string>

using namespace faliks;
using namespace std;

bool passed = true;

template<typename T>
void checkEqual(T &&a, T &&b) {
    if (a == b) {
        logi("checkEqual: {} == {} passed\n", a, b);
    } else {
        loge("checkEqual: {} == {} failed\n", a, b);
        passed = false;
    }
}

template<typename T>
void checkEqual(uint16_t &&a, T &&b) {
    if (static_cast<T>(a) == b) {
        logi("checkEqual: {} == {} passed\n", a, b);
    } else {
        loge("checkEqual: {} == {} failed\n", a, b);
    }
}

void test1() {
    InetAddress addr0(1234);
    checkEqual(addr0.toIp(), std::string("0.0.0.0"));
    checkEqual(addr0.toIpPort(), std::string("0.0.0.0:1234"));
    checkEqual(addr0.port(), 1234);

    InetAddress addr1(4321, true);
    checkEqual(addr1.toIp(), std::string("127.0.0.1"));
    checkEqual(addr1.toIpPort(), std::string("127.0.0.1:4321"));
    checkEqual(addr1.port(), 4321);

    InetAddress addr2("1.2.3.4", 8888);
    checkEqual(addr2.toIp(), std::string("1.2.3.4"));
    checkEqual(addr2.toIpPort(), std::string("1.2.3.4:8888"));
    checkEqual(addr2.port(), 8888);

    InetAddress addr3("255.254.253.252", 65535);
    checkEqual(addr3.toIp(), std::string("255.254.253.252"));
    checkEqual(addr3.toIpPort(), std::string("255.254.253.252:65535"));
    checkEqual(addr3.port(), 65535);
    fmtlog::poll();
}

void test2() {
    InetAddress addr0(1234, false, true);
    checkEqual(addr0.toIp(), string("::"));
    checkEqual(addr0.toIpPort(), string("[::]:1234"));
    checkEqual(addr0.port(), 1234);

    InetAddress addr1(1234, true, true);
    checkEqual(addr1.toIp(), string("::1"));
    checkEqual(addr1.toIpPort(), string("[::1]:1234"));
    checkEqual(addr1.port(), 1234);

    InetAddress addr2("2001:db8::1", 8888, true);
    checkEqual(addr2.toIp(), string("2001:db8::1"));
    checkEqual(addr2.toIpPort(), string("[2001:db8::1]:8888"));
    checkEqual(addr2.port(), 8888);

    InetAddress addr3("fe80::1234:abcd:1", 8888);
    checkEqual(addr3.toIp(), string("fe80::1234:abcd:1"));
    checkEqual(addr3.toIpPort(), string("[fe80::1234:abcd:1]:8888"));
    checkEqual(addr3.port(), 8888);
    fmtlog::poll();
}

void test3() {
    InetAddress addr(80);
    if (InetAddress::resolve("google.com", &addr)) {
        logi("google.com resolved to {}", addr.toIpPort().c_str());
    } else {
        loge("Unable to resolve google.com");
    }
    fmtlog::poll();
}

int main() {
    fmtlog::startPollingThread(1e8);
    test1();
    test2();
    test3();
    logi("InetAddressTest passed: {}", passed)
    return 0;
}