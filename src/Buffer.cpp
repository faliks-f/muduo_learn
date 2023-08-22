#include "src/include/Buffer.h"

#include <cassert>
#include <cstring>
#include <algorithm>

#include <sys/uio.h>

using namespace faliks;

const char Buffer::kCRLF[] = "\r\n";

Buffer::Buffer(size_t initialSize)
        : m_buffer(CHEAP_PREPEND + initialSize),
          m_readIndex(CHEAP_PREPEND),
          m_writeIndex(CHEAP_PREPEND) {
    assert(readableBytes() == 0);
    assert(writableBytes() == initialSize);
    assert(prependableBytes() == CHEAP_PREPEND);
}

void Buffer::makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < len + CHEAP_PREPEND) {
        m_buffer.resize(m_writeIndex + len);
    } else {
        assert(CHEAP_PREPEND < m_readIndex);
        size_t readable = readableBytes();
        std::copy(begin() + m_readIndex, begin() + m_writeIndex, begin() + CHEAP_PREPEND);
        m_readIndex = CHEAP_PREPEND;
        m_writeIndex = m_readIndex + readable;
        assert(readable == readableBytes());
    }
}

size_t Buffer::readableBytes() const {
    return m_writeIndex - m_readIndex;
}

size_t Buffer::writableBytes() const {
    return m_buffer.size() - m_writeIndex;
}

size_t Buffer::prependableBytes() const {
    return m_readIndex;
}

void Buffer::swap(Buffer &rhs) {
    m_buffer.swap(rhs.m_buffer);
    std::swap(m_readIndex, rhs.m_readIndex);
    std::swap(m_writeIndex, rhs.m_writeIndex);
}

const char *Buffer::peek() const {
    return begin() + m_readIndex;
}

const char *Buffer::findCRLF() const {
    const char *crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
}

const char *Buffer::findCRLF(const char *start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());

    const char *crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
    return crlf == beginWrite() ? nullptr : crlf;
}

const char *Buffer::findEOL() const {
    const void *eol = memchr(peek(), '\n', readableBytes());
    return static_cast<const char *>(eol);
}

const char *Buffer::findEOL(const char *start) const {
    assert(peek() <= start);
    assert(start <= beginWrite());
    const void *eol = memchr(start, '\n', beginWrite() - start);
    return static_cast<const char *>(eol);
}

char *Buffer::beginWrite() {
    return begin() + m_writeIndex;
}

const char *Buffer::beginWrite() const {
    return begin() + m_writeIndex;
}

void Buffer::retrieve(size_t len) {
    assert(len <= readableBytes());
    if (len < readableBytes()) {
        m_readIndex += len;
    } else {
        retrieveAll();
    }
}

void Buffer::retrieveAll() {
    m_readIndex = CHEAP_PREPEND;
    m_writeIndex = CHEAP_PREPEND;
}

void Buffer::retrieveUntil(const char *end) {
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
}

void Buffer::retrieveInt64() {
    retrieve(sizeof(int64_t));
}

void Buffer::retrieveInt32() {
    return retrieve(sizeof(int32_t));
}

void Buffer::retrieveInt16() {
    return retrieve(sizeof(int16_t));
}

void Buffer::retrieveInt8() {
    return retrieve(sizeof(int8_t));
}

std::string Buffer::retrieveAsString(size_t len) {
    assert(len <= readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
}

std::string Buffer::retrieveAllAsString() {
    return retrieveAsString(readableBytes());
}

std::string Buffer::toString() const {
    return {peek(), readableBytes()};
}


void Buffer::append(const char *data, size_t len) {
    ensureWritableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const std::string &str) {
    append(str.data(), str.length());
}

void Buffer::append(const void *data, size_t len) {
    append(static_cast<const char *>(data), len);
}

void Buffer::ensureWritableBytes(size_t len) {
    if (writableBytes() < len) {
        makeSpace(len);
    }
    assert(writableBytes() >= len);
}

void Buffer::hasWritten(size_t len) {
    assert(len <= writableBytes());
    m_writeIndex += len;
}

void Buffer::unWrite(size_t len) {
    assert(len <= readableBytes());
    m_writeIndex -= len;
}

void Buffer::appendInt64(int64_t x) {
    int64_t be64 = htobe64(x);
    append(&be64, sizeof(be64));
}

void Buffer::appendInt32(int32_t x) {
    int32_t be32 = htobe32(x);
    append(&be32, sizeof(be32));
}

void Buffer::appendInt16(int16_t x) {
    int16_t be16 = htobe16(x);
    append(&be16, sizeof(be16));
}

void Buffer::appendInt8(int8_t x) {
    append(&x, sizeof(x));
}

int64_t Buffer::peekInt64() const {
    assert(readableBytes() >= sizeof(int64_t));
    int64_t be64 = 0;
    ::memcpy(&be64, peek(), sizeof(be64));
    return be64toh(be64);
}


int64_t Buffer::readInt64() {
    int64_t result = peekInt64();
    retrieveInt64();
    return result;
}

int32_t Buffer::peekInt32() const {
    assert(readableBytes() >= sizeof(int32_t));
    int32_t be32 = 0;
    ::memcpy(&be32, peek(), sizeof(be32));
    return be32toh(be32);
}

int32_t Buffer::readInt32() {
    int32_t result = peekInt32();
    retrieveInt32();
    return result;
}

int16_t Buffer::peekInt16() const {
    assert(readableBytes() >= sizeof(int16_t));
    int16_t be16 = 0;
    ::memcpy(&be16, peek(), sizeof(be16));
    return be16toh(be16);
}

int16_t Buffer::readInt16() {
    int16_t result = peekInt16();
    retrieveInt16();
    return result;
}

int8_t Buffer::peekInt8() const {
    assert(readableBytes() >= sizeof(int8_t));
    int8_t x = *peek();
    return x;
}

int8_t Buffer::readInt8() {
    int8_t result = peekInt8();
    retrieveInt8();
    return result;
}

void Buffer::prepend(const void *data, size_t len) {
    assert(len <= prependableBytes());
    m_readIndex -= len;
    const char *d = static_cast<const char *>(data);
    std::copy(d, d + len, begin() + m_readIndex);
}

void Buffer::shrink(size_t reserve) {
    Buffer other;
    other.ensureWritableBytes(readableBytes() + reserve);
    other.append(toString());
    swap(other);
}

size_t Buffer::internalCapacity() const {
    return m_buffer.capacity();
}

ssize_t Buffer::readFd(int fd, int *savedErrno) {
    char extraBuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;
    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);

    const int iovcnt = (writable < sizeof(extraBuf)) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);

    if (n < 0) {
        *savedErrno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        m_writeIndex += n;
    } else {
        m_writeIndex = m_buffer.size();
        append(extraBuf, n - writable);
    }
    return n;
}






