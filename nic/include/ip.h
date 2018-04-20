#ifndef __IP_H__
#define __IP_H__

#include "nic/include/base.h"
#include "nic/include/bitmap.hpp"

//------------------------------------------------------------------------------
// IP address family
//------------------------------------------------------------------------------
#define IP_AF_IPV4                                   0
#define IP_AF_IPV6                                   1

//------------------------------------------------------------------------------
// IPv4 address length
//------------------------------------------------------------------------------
#define IP4_ADDR8_LEN                                4

//------------------------------------------------------------------------------
// default IP TTL to be used
//------------------------------------------------------------------------------
#define IP_DEFAULT_TTL                               64

//------------------------------------------------------------------------------
// minimum IPv4 header length
//------------------------------------------------------------------------------
#define IPV4_MIN_HDR_LEN                             20

//------------------------------------------------------------------------------
// IPv6 address length in terms of bytes, shorts and 32 bit words
//------------------------------------------------------------------------------
#define IP6_ADDR8_LEN                                16
#define IP6_ADDR16_LEN                               8
#define IP6_ADDR32_LEN                               4
#define IP6_ADDR64_LEN                               2

/*****************************************************************************/
/* IP protocol types                                                         */
/*****************************************************************************/
#define IP_PROTO_ICMP                  1
#define IP_PROTO_IGMP                  2
#define IP_PROTO_IPV4                  4
#define IP_PROTO_TCP                   6
#define IP_PROTO_UDP                   17
#define IP_PROTO_IPV6                  41
#define IP_PROTO_GRE                   47
#define IP_PROTO_IPSEC_ESP             50
#define IP_PROTO_IPSEC_AH              51
#define IP_PROTO_ICMPV6                58
#define IP_PROTO_EIGRP                 88
#define IP_PROTO_OSPF                  89
#define IP_PROTO_PIM                   103
#define IP_PROTO_VRRP                  112

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
// IPv4 address range definition
//------------------------------------------------------------------------------
typedef struct ipv4_range_s {
    ipv4_addr_t    ip_lo;
    ipv4_addr_t    ip_hi;
} __PACK__ ipv4_range_t;

//------------------------------------------------------------------------------
// IPv6 address range definition
//------------------------------------------------------------------------------
typedef struct ipv6_range_s {
    ipv6_addr_t    ip_lo;
    ipv6_addr_t    ip_hi;
} __PACK__ ipv6_range_t;

//------------------------------------------------------------------------------
// generic IP address range type to be used in memory optimized implementation
//------------------------------------------------------------------------------
typedef struct ip_range_s {
    uint8_t            af;
    union {
        ipv4_range_t    v4_range;
        ipv6_range_t    v6_range;
    } __PACK__ vx_range[0];
} __PACK__ ip_range_t;

//------------------------------------------------------------------------------
// generic IP address range type that can be used where memory usage need not be
// optimized
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

// spdlog formatter for ip_addr_t
static inline std::ostream& operator<<(std::ostream& os, const ip_addr_t& ip) {
    return os << ipaddr2str(&ip);
}

// spdlog formatter for ipv6_addr_t
static inline std::ostream& operator<<(std::ostream& os, const ipv6_addr_t& ip)
{
    return os << ipv6addr2str(ip);
}

static inline bool
ip_addr_is_equal (ip_addr_t *ip_addr1, ip_addr_t *ip_addr2)
{
    if (!ip_addr1 || !ip_addr2) {
        return false;
    }

    return memcmp(ip_addr1, ip_addr2, sizeof(ip_addr_t)) ? false : true;
}

static inline bool
ip_addr_is_zero (ip_addr_t *addr)
{
    return (addr->af == IP_AF_IPV4) ? addr->addr.v4_addr == 0 :
        (addr->addr.v6_addr.addr64[0] & addr->addr.v6_addr.addr64[1]) == 0;
}

static inline bool
ip_prefix_is_equal (ip_prefix_t *ip_prefix1, ip_prefix_t *ip_prefix2)
{
    if (!ip_prefix1 || !ip_prefix2) {
        return false;
    }

    return memcmp(ip_prefix1, ip_prefix2, sizeof(ip_prefix_t)) ? false : true;
}


static inline ipv4_addr_t
ipv4_prefix_len_to_mask (uint8_t len)
{
    if (len > 32) {
        return 0;
    }
    return len == 0 ? 0: ~((1<<(32-len))-1);
}

static inline int
ipv4_mask_to_prefix_len (ipv4_addr_t v4_addr)
{
    ipv4_addr_t iv4_addr = ~v4_addr;

    if (!v4_addr) {
        return 0;
    }
    // Check to see that set/reset bits are together (it is a power2 - 1)
    if (iv4_addr & (iv4_addr + 1)) {
        // Error
        return -1;
    }

    return 32 - hal::utils::bitmap::log2_floor(iv4_addr + 1);
}

static inline void
ipv6_prefix_len_to_mask (ipv6_addr_t *v6_addr, uint8_t len)
{
    uint8_t    wp = 0;

    v6_addr->addr64[0] = 0;
    v6_addr->addr64[1] = 0;
    if (len > 128) {
        return;
    }
    // Using addr32/addr64 is tricky here because the v6 addresses
    // are stored in big-endian format. So for example, for prefix len of 20
    // the mask should be 0xff 0xff 0xf0 0x00 ...
    // If read it as addr32 it translates to 0x00f0ffff . Forming this is tricky
    while (len) {
        v6_addr->addr8[wp++] = (len >= 8) ? 0xff: (0xff & (~((1<<(8-len))-1)));
        len = (len >= 8) ? len-8 : 0;
    }
}

static inline int
ipv6_mask_to_prefix_len (ipv6_addr_t *v6_addr)
{
    int prefix_len = 0;
    unsigned inv;
    unsigned i;
    for (i = 0; v6_addr->addr8[i] && (i < HAL_ARRAY_SIZE(v6_addr->addr8)); i++) {
        if (v6_addr->addr8[i] == 0xff) {
            prefix_len += 8;
        } else {
            inv = ~v6_addr->addr8[i] & 0xff;
            if (inv & (inv + 1)) {
                // Error
                prefix_len = -1;
            } else {
                prefix_len += 8 - hal::utils::bitmap::log2_floor(inv + 1);
            }
            i++;
            break;
        }
    }
    for (; i < HAL_ARRAY_SIZE(v6_addr->addr8); i++) {
        // If any other bits are set, it's not a valid prefix
        if (v6_addr->addr8[i]) {
            prefix_len = -1;
            break;
        }
    }
    return prefix_len;
}

static inline bool
ipv4_addr_is_multicast (ipv4_addr_t *ipv4_addr)
{
    return (((*ipv4_addr) & 0xF0000000) == 0xE0000000);
}

static inline bool
ipv6_addr_is_multicast (ipv6_addr_t *ipv6_addr)
{
    return (ipv6_addr->addr8[0]  == 0xFF);
}

static inline bool
ip_addr_is_multicast (ip_addr_t *ip_addr)
{
    if (ip_addr->af == IP_AF_IPV4) {
        return ipv4_addr_is_multicast(&(ip_addr->addr.v4_addr));
    } else {
        return ipv6_addr_is_multicast(&(ip_addr->addr.v6_addr));
    }
}

#endif    // __IP_H__

