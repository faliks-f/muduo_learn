#include "src/include/Buffer.h"
#include "base/include/fmtlog.h"


#include <string>

using namespace faliks;
using namespace std;

bool passed = true;

template<typename T1, typename T2>
void checkEqual(T1&& a, T2 &&b) {
    if (a == b) {
        logi("checkEqual: {} == {} passed\n", a, b);
    } else {
        loge("checkEqual: {} == {} failed\n", a, b);
        passed = false;
    }
}


void test1() {
    Buffer buffer;
    checkEqual(buffer.readableBytes(), 0);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE);
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND);

    const string str(200, 'x');
    buffer.append(str);
    checkEqual(buffer.readableBytes(), str.size());
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE - str.size());
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND);

    const string str2 = buffer.retrieveAsString(50);
    checkEqual(str.size(), 50);
    checkEqual(buffer.readableBytes(), str.size() - 50);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE - str.size());
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND + 50);
    checkEqual(str2.size(), 50);
    checkEqual(str2, string(50, 'x'));

    buffer.append(str);
    checkEqual(buffer.readableBytes(), 2 * str.size() - 50);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE - 2 * str.size());
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND + 50);

    const string str3 = buffer.retrieveAllAsString();
    checkEqual(str3.size(), 350);
    checkEqual(str3, string(350, 'x'));
    checkEqual(buffer.readableBytes(), 0);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE);
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND);
}

void test2() {
    Buffer buffer;
    buffer.append(string(400, 'y'));
    checkEqual(buffer.readableBytes(), 400);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE - 400);

    buffer.retrieve(50);
    checkEqual(buffer.readableBytes(), 350);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE - 400);
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND + 50);

    buffer.append(string(1000, 'z'));
    checkEqual(buffer.readableBytes(), 1350);
    checkEqual(buffer.writableBytes(), 0);
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND + 50);

    buffer.retrieveAll();
    checkEqual(buffer.readableBytes(), 0);
    checkEqual(buffer.writableBytes(), 1400);
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND);
}

void test3() {
    Buffer buffer;
    buffer.append(string(800, 'y'));
    checkEqual(buffer.readableBytes(), 8000);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE - 800);

    buffer.retrieve(500);
    checkEqual(buffer.readableBytes(), 300);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE - 800);
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND + 500);

    buffer.append(string(300, 'z'));
    checkEqual(buffer.readableBytes(), 600);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE - 600);
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND);
}

void test4() {
    Buffer buffer;
    buffer.append(string(2000, 'y'));
    checkEqual(buffer.readableBytes(), 2000);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE - 2000);

    buffer.retrieve(1500);
    checkEqual(buffer.readableBytes(), 500);
    checkEqual(buffer.writableBytes(), 0);
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND + 1500);

    buffer.shrink(0);
    checkEqual(buffer.readableBytes(), 500);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE - 500);
    checkEqual(buffer.retrieveAllAsString(), string(500, 'y'));
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND);
}

void test5() {
    Buffer buffer;
    buffer.append(string(200, 'y'));
    checkEqual(buffer.readableBytes(), 200);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE - 200);
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND);

    int x = 0;
    buffer.append(&x, sizeof(x));
    checkEqual(buffer.readableBytes(), 204);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE - 200);
    checkEqual(buffer.prependableBytes(), Buffer::CHEAP_PREPEND);
}

void test6() {
    Buffer buffer;
    buffer.append("HTTP");

    checkEqual(buffer.readableBytes(), 4);
    checkEqual(buffer.peekInt8(), 'H');
    int top16 = buffer.peekInt16();
    checkEqual(top16, 'H' * 256 + 'T');
    checkEqual(buffer.peekInt32(), top16 * 65536 + 'T' * 256 + 'P');

    checkEqual(buffer.readInt8(), 'H');
    checkEqual(buffer.readInt16(), 'T' * 256 + 'T');
    checkEqual(buffer.readInt8(), 'P');
    checkEqual(buffer.readableBytes(), 0);
    checkEqual(buffer.writableBytes(), Buffer::INITIAL_SIZE);

    buffer.appendInt8(-1);
    buffer.appendInt16(-2);
    buffer.appendInt32(-3);
    checkEqual(buffer.readableBytes(), 7);
    checkEqual(buffer.readInt8(), -1);
    checkEqual(buffer.readInt16(), -2);
    checkEqual(buffer.readInt32(), -3);
}

void test7() {
    Buffer buffer;
    buffer.append(string(100000, 'x'));
    const char *null = nullptr;
    checkEqual(buffer.findEOL(), null);
    checkEqual(buffer.findEOL(buffer.peek() + 90000), null);
}

void output(Buffer &&buf, const void *inner) {
    Buffer newBuf(std::move(buf));
    checkEqual(inner, newBuf.peek());
}

void test8() {
    Buffer buffer;
    buffer.append("faliks", 6);
    const void *inner = buffer.peek();
    output(std::move(buffer), inner);
}

int main() {
    fmtlog::startPollingThread(1e8);
    test1();
//    test2();
//    test3();
//    test4();
//    test5();
//    test6();
//    test7();
//    test8();
    logi("Test passed: {}", passed);
    return 0;
}