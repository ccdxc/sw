#ifndef __ETH_H__
#define __ETH_H__

#include <stdio.h>
#include "nic/include/base.h"

#define L2_MIN_HDR_LEN                                 14
#define L2_ETH_HDR_LEN                                 14
#define L2_DOT1Q_HDR_LEN                               18

#define ETH_TYPE_IPV4                                  0x0800
#define ETH_TYPE_ARP                                   0x0806
#define ETH_TYPE_RARP                                  0x8035
#define ETH_TYPE_DOT1Q                                 0x8100
#define ETH_TYPE_IPV6                                  0x86DD

// ethernet header
typedef struct eth_hdr_ {
    mac_addr_t        daddr;
    mac_addr_t        saddr;
    etype_t           etype;
} __PACK__ eth_hdr_t;

// 802.1q header
typedef struct eth_dot1q_hdr_ {
    mac_addr_t        daddr;
    mac_addr_t        saddr;
    etype_t           ethtype;        // set to ETH_TYPE_DOT1Q
    uint16_t          prio:3;
    uint16_t          cfi:1;
    uint16_t          vlan_id:12;
    etype_t           etype;
} __PACK__ eth_dot1q_hdr_t;

#define MAC_TO_UINT64(mac_addr)                                           \
            ((mac_addr[5] & 0xFF)                                       | \
             ((mac_addr[4] & 0xFF) << 8) | ((mac_addr[3] & 0xFF) << 16) | \
             (((uint64_t)(mac_addr[2]) & 0xFF) << 24)                   | \
             ((uint64_t)(mac_addr[1] & 0xFF) << 32ul)                   | \
             ((uint64_t)(mac_addr[0] & 0xFF) << 40ul))

// Mac: 0xaabbccddeeff 
//    mac_uint64: 0xaabbccddeeff
//    mac_addr_t: [0]:aa, [1]:bb, [2]:cc, [3]:dd, [4]:ee, [5]:ff
#define MAC_UINT64_TO_ADDR(mac_addr, mac_uint64)                          \
{                                                                         \
     (mac_addr)[5] = mac_uint64 & 0xFF;                                   \
     (mac_addr)[4] = (mac_uint64 >> 8) & 0xFF;                            \
     (mac_addr)[3] = (mac_uint64 >> 16) & 0xFF;                           \
     (mac_addr)[2] = (mac_uint64 >> 24) & 0xFF;                           \
     (mac_addr)[1] = (mac_uint64 >> 32) & 0xFF;                           \
     (mac_addr)[0] = (mac_uint64 >> 40) & 0xFF;                           \
}

#define IS_MCAST_MAC_ADDR(mac_addr)            ((mac_addr)[0] & 0x1)

// thread safe helper to stringify MAC address
extern char *macaddr2str(const mac_addr_t mac_addr);

static inline void
mac_str_to_addr (char *str, mac_addr_t *mac)
{
    sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static inline void
mac_addr_to_str (mac_addr_t mac, char *str)
{
    snprintf(str, 30, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

//spdlog formatter for ipv6_addr_t
inline std::ostream& operator<<(std::ostream& os, mac_addr_t mac) {
    return os << macaddr2str(mac);
}

#endif    // __ETH_H__

