#include "src/include/InetAddress.h"
#include "base/include/fmtlog.h"

#include <cstddef>
#include <cstring>
#include <endian.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cassert>
#include <netdb.h>

namespace faliks {

    constexpr in_addr_t kInaddrAny = INADDR_ANY;
    constexpr in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

    static __thread char t_resolveBuffer[64 * 1024];


    InetAddress::InetAddress(uint16_t port, bool localhost, bool ipv6) {
        static_assert(offsetof(InetAddress, m_sockaddrIn6) == 0, "m_sockaddrIn6 offset 0");
        static_assert(offsetof(InetAddress, m_sockaddrIn) == 0, "m_sockaddrIn offset 0");
        if (ipv6) {
            memset(&m_sockaddrIn6, 0, sizeof m_sockaddrIn6);
            m_sockaddrIn6.sin6_family = AF_INET6;
            in6_addr ip = localhost ? in6addr_loopback : in6addr_any;
            m_sockaddrIn6.sin6_addr = ip;
            m_sockaddrIn6.sin6_port = htobe16(port);
        } else {
            memset(&m_sockaddrIn, 0, sizeof m_sockaddrIn);
            m_sockaddrIn.sin_family = AF_INET;
            in_addr_t ip = localhost ? kInaddrLoopback : kInaddrAny;
            m_sockaddrIn.sin_addr.s_addr = htobe32(ip);
            m_sockaddrIn.sin_port = htobe16(port);
        }
    }

    InetAddress::InetAddress(const char *ip, uint16_t port, bool ipv6) {
        if (ipv6 || strchr(ip, ':')) {
            memset(&m_sockaddrIn6, 0, sizeof m_sockaddrIn6);
            m_sockaddrIn6.sin6_family = AF_INET6;
            m_sockaddrIn6.sin6_port = htobe16(port);
            if (::inet_pton(AF_INET6, ip, &m_sockaddrIn6.sin6_addr) <= 0) {
                loge("inet_pton error for {}", ip);
            }
        } else {
            memset(&m_sockaddrIn, 0, sizeof m_sockaddrIn);
            m_sockaddrIn.sin_family = AF_INET;
            m_sockaddrIn.sin_port = htobe16(port);
            if (::inet_pton(AF_INET, ip, &m_sockaddrIn.sin_addr) <= 0) {
                loge("inet_pton error for {}", ip);
            }
        }
    }

    std::string InetAddress::toIp() const {
        char buf[64] = "";
        if (m_sockaddrIn.sin_family == AF_INET) {
            ::inet_ntop(AF_INET, &m_sockaddrIn.sin_addr, buf, sizeof buf);
        } else if (m_sockaddrIn6.sin6_family == AF_INET6) {
            ::inet_ntop(AF_INET6, &m_sockaddrIn6.sin6_addr, buf, sizeof buf);
        }
        return buf;
    }

    std::string InetAddress::toIpPort() const {
        char buf[64] = "";
        if (m_sockaddrIn.sin_family == AF_INET) {
            ::inet_ntop(AF_INET, &m_sockaddrIn.sin_addr, buf, sizeof buf);
            size_t end = ::strlen(buf);
            uint16_t port = be16toh(m_sockaddrIn.sin_port);
            snprintf(buf + end, sizeof buf - end, ":%u", port);
        } else if (m_sockaddrIn6.sin6_family == AF_INET6) {
            buf[0] = '[';
            ::inet_ntop(AF_INET6, &m_sockaddrIn6.sin6_addr, buf + 1, sizeof buf - 1);
            size_t end = ::strlen(buf);
            uint16_t port = be16toh(m_sockaddrIn6.sin6_port);
            snprintf(buf + end, sizeof buf - end, "]:%u", port);
        }
        return buf;
    }

    uint16_t InetAddress::port() const {
        return be16toh(m_sockaddrIn.sin_port);
    }

    const struct sockaddr *InetAddress::getSockAddr() const {
        return reinterpret_cast<const struct sockaddr *>(&m_sockaddrIn6);
    }

    uint32_t InetAddress::ipv4NetEndian() const {
        assert(family() == AF_INET);
        return m_sockaddrIn.sin_addr.s_addr;
    }

    bool InetAddress::resolve(const char *hostname, InetAddress *result) {
        assert(result != nullptr);
        struct hostent h;
        struct hostent *hp = nullptr;
        int herrno = 0;
        memset(&h, 0, sizeof h);

        int ret = gethostbyname_r(hostname, &h, t_resolveBuffer, sizeof t_resolveBuffer, &hp, &herrno);
        if (ret == 0 && hp != nullptr) {
            assert(h.h_addrtype == AF_INET && h.h_length == sizeof(uint32_t));
            result->m_sockaddrIn.sin_addr = *reinterpret_cast<struct in_addr *>(h.h_addr);
            return true;
        } else {
            if (ret) {
                loge("InetAddress::resolve");
            }
            return false;
        }
    }

    void InetAddress::setScopedId(uint32_t scope_id) {
        if (family() == AF_INET6) {
            m_sockaddrIn6.sin6_scope_id = scope_id;
        }
    }
}
