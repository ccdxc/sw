#ifndef __IP_H__
#define __IP_H__

#include <base.h>

//------------------------------------------------------------------------------
// IP address family
//------------------------------------------------------------------------------
#define IP_AF_IPV4                                   0
#define IP_AF_IPV6                                   1

//------------------------------------------------------------------------------
// IPv6 address length in terms of bytes, shorts and 32 bit words
//------------------------------------------------------------------------------
#define IP6_ADDR8_LEN                                16
#define IP6_ADDR16_LEN                               8
#define IP6_ADDR32_LEN                               4
#define IP6_ADDR64_LEN                               2

//------------------------------------------------------------------------------
// TCP Flags 
//------------------------------------------------------------------------------
#define TCP_FLAG_CWR                   0x80
#define TCP_FLAG_ECE                   0x40
#define TCP_FLAG_URG                   0x20
#define TCP_FLAG_ACK                   0x10
#define TCP_FLAG_PSH                   0x08
#define TCP_FLAG_RST                   0x04
#define TCP_FLAG_SYN                   0x02
#define TCP_FLAG_FIN                   0x01

//------------------------------------------------------------------------------
// IPv4 and IPv6 addresses
//------------------------------------------------------------------------------
typedef uint32_t ipv4_addr_t;

typedef struct ipv6_addr_s {
    union {
        uint8_t       addr8[IP6_ADDR8_LEN];
        uint16_t      addr16[IP6_ADDR16_LEN];
        uint32_t      addr32[IP6_ADDR32_LEN];
        uint64_t      addr64[IP6_ADDR64_LEN];
    };
} __PACK__ ipv6_addr_t;

//------------------------------------------------------------------------------
// unified IP address
//------------------------------------------------------------------------------
typedef union ipvx_addr_u {
    ipv4_addr_t    v4_addr;
    ipv6_addr_t    v6_addr;
} __PACK__ ipvx_addr_t;

//------------------------------------------------------------------------------
// IP address range
//------------------------------------------------------------------------------
typedef struct ipvx_range_s {
    uint8_t            af;
    ipvx_addr_t        ip_lo;
    ipvx_addr_t        ip_hi;
} __PACK__ ipvx_range_t;

//------------------------------------------------------------------------------
// generic IP address structure
//------------------------------------------------------------------------------
typedef struct ip_addr_s {
    uint8_t            af;
    ipvx_addr_t        addr;
} __PACK__ ip_addr_t;

//------------------------------------------------------------------------------
// IPv4 prefix
//------------------------------------------------------------------------------
typedef struct ipv4_prefix_s {
    uint8_t            len;
    ipv4_addr_t        v4_addr;
} __PACK__ ipv4_prefix_t;

//------------------------------------------------------------------------------
// IPv6 prefix
//------------------------------------------------------------------------------
typedef struct ipv6_prefix_s {
    uint8_t            len;
    ipv6_addr_t        v6_addr;
} __PACK__ ipv6_prefix_t;

//------------------------------------------------------------------------------
// unified IP prefix
//------------------------------------------------------------------------------
typedef struct ipvx_prefix_s {
    uint8_t            len;
    ipvx_addr_t        v6_addr;
} __PACK__ ipvx_prefix_t;

//------------------------------------------------------------------------------
// generic IP prefix
//------------------------------------------------------------------------------
typedef struct ip_prefix_s {
    ip_addr_t          addr;    // prefix
    uint8_t            len;     // prefix length
} __PACK__ ip_prefix_t;

extern char *ipv4addr2str(ipv4_addr_t v4_addr);
extern char *ipv6addr2str(ipv6_addr_t v6_addr);
extern char *ipaddr2str(const ip_addr_t *ip_addr);
extern char *ippfx2str(const ip_prefix_t *ip_pfx);

//spdlog formatter for ip_addr_t
inline std::ostream& operator<<(std::ostream& os, const ip_addr_t& ip) {
    return os << ipaddr2str(&ip);
}

// TODO(goli) conflicts with another << operator overload for unisigned int
//spdlog formatter for ipv4_addr_t
//inline std::ostream& operator<<(std::ostream& os, const ipv4_addr_t& ip) {
//    return os << ipv4addr2str(ip);
//}

//spdlog formatter for ipv6_addr_t
inline std::ostream& operator<<(std::ostream& os, const ipv6_addr_t& ip) {
    return os << ipv6addr2str(ip);
}

inline ipv4_addr_t ipv4_prefix_len_to_mask(uint8_t len) {
    if (len > 32) {
        return 0;
    }
    return len == 0 ? 0: ~((1<<(32-len))-1);
}

inline void ipv6_prefix_len_to_mask(ipv6_addr_t *v6_addr, uint8_t len) {
    uint8_t wp = 0;
    if (len > 128) {
        v6_addr->addr64[0] = 0;
        v6_addr->addr64[1] = 0;
    }
    while (len) {
        v6_addr->addr8[wp++] = len >= 8 ? 0xff : 0xff & (~((1<<(8-len))-1));
        len -= 8;
    }
}

#endif    // __IP_H__

