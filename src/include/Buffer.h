
#ifndef MUDUO_LEARN_BUFFER_H
#define MUDUO_LEARN_BUFFER_H

#include "base/include/Copyable.h"

#include <vector>
#include <cstdint>
#include <cstddef>
#include <string>

namespace faliks {
    class Buffer : public Copyable {
    private:
        std::vector<char> m_buffer;
        size_t m_readIndex;
        size_t m_writeIndex;

        static const char kCRLF[];

        [[nodiscard]] char *begin() {
            return &(*m_buffer.begin());
        }

        [[nodiscard]] const char *begin() const {
            return &(*m_buffer.begin());
        }

        void makeSpace(size_t len);

    public:
        constexpr static size_t CHEAP_PREPEND = 8;
        constexpr static size_t INITIAL_SIZE = 1024;

        explicit Buffer(size_t initialSize = INITIAL_SIZE);

//        Buffer(const Buffer &) = delete;
//
//        Buffer &operator=(const Buffer &) = delete;
//
//        Buffer(Buffer &&) = delete;

        void swap(Buffer &rhs);

        [[nodiscard]] size_t readableBytes() const;

        [[nodiscard]] size_t writableBytes() const;

        [[nodiscard]] size_t prependableBytes() const;

        [[nodiscard]] const char *peek() const;

        [[nodiscard]] const char *findCRLF() const;

        [[nodiscard]] const char *findCRLF(const char *start) const;

        [[nodiscard]] const char *findEOL() const;

        [[nodiscard]] const char *findEOL(const char *start) const;

        [[nodiscard]] char *beginWrite();

        [[nodiscard]] const char *beginWrite() const;

        void retrieve(size_t len);

        void retrieveUntil(const char *end);

        void retrieveAll();

        void retrieveInt64();

        void retrieveInt32();

        void retrieveInt16();

        void retrieveInt8();

        std::string retrieveAsString(size_t len);

        std::string retrieveAllAsString();

        std::string toString() const;

        void append(const char *data, size_t len);

        void append(const std::string &str);

        void append(const void *data, size_t len);

        void ensureWritableBytes(size_t len);

        void hasWritten(size_t len);

        void unWrite(size_t len);

        void appendInt64(int64_t x);

        void appendInt32(int32_t x);

        void appendInt16(int16_t x);

        void appendInt8(int8_t x);

        [[nodiscard]] int64_t peekInt64() const;

        [[nodiscard]] int64_t readInt64();

        [[nodiscard]] int32_t peekInt32() const;

        [[nodiscard]] int32_t readInt32();

        [[nodiscard]] int16_t peekInt16() const;

        [[nodiscard]] int16_t readInt16();

        [[nodiscard]] int8_t peekInt8() const;

        [[nodiscard]] int8_t readInt8();

        void prepend(const void *data, size_t len);

        void shrink(size_t reserve);

        [[nodiscard]] size_t internalCapacity() const;

        ssize_t readFd(int fd, int *savedErrno);
    };

}

#endif //MUDUO_LEARN_BUFFER_H
