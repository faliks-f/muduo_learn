#ifndef MUDUO_LEARN_INETADDRESS_H
#define MUDUO_LEARN_INETADDRESS_H

#include "base/include/Copyable.h"

#include <netinet/in.h>
#include <string>

namespace faliks {
    class InetAddress : public Copyable {
    private:
        union {
            struct sockaddr_in m_sockaddrIn;
            struct sockaddr_in6 m_sockaddrIn6;
        };

    public:
        explicit InetAddress(uint16_t port = 0, bool localhost = false, bool ipv6 = false);

        InetAddress(const char *ip, uint16_t port, bool ipv6 = false);

        explicit InetAddress(const struct sockaddr_in &sockaddrIn) : m_sockaddrIn(sockaddrIn) {}

        explicit InetAddress(const struct sockaddr_in6 &sockaddrIn6) : m_sockaddrIn6(sockaddrIn6) {}

        [[nodiscard]] sa_family_t family() const { return m_sockaddrIn.sin_family; }

        [[nodiscard]] std::string toIp() const;

        [[nodiscard]] std::string toIpPort() const;

        [[nodiscard]] uint16_t port() const;

        [[nodiscard]] const struct sockaddr* getSockAddr() const;

        void setSockAddrInet6(const struct sockaddr_in6 &sockaddrIn6) { m_sockaddrIn6 = sockaddrIn6; }

        [[nodiscard]] uint32_t ipv4NetEndian() const;

        [[nodiscard]] uint16_t portNetEndian() const { return m_sockaddrIn.sin_port; }

        static bool resolve(const char *hostname, InetAddress *result);

        void setScopedId(uint32_t scope_id);
    };
}


#endif //MUDUO_LEARN_INETADDRESS_H
