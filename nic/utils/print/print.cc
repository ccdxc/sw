#include "nic/include/base.h"
#include "nic/include/eth.h"
#include "nic/include/ip.h"

//------------------------------------------------------------------------------
// thread safe helper to stringify MAC address
//------------------------------------------------------------------------------
char *
macaddr2str (const mac_addr_t mac_addr)
{
    static thread_local char       macaddr_str[4][20];
    static thread_local uint8_t    macaddr_str_next = 0;
    char                           *buf;

    buf = macaddr_str[macaddr_str_next++ & 0x3];
    snprintf(buf, 20, "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2],
             mac_addr[3], mac_addr[4], mac_addr[5]);
    return buf;
}

//------------------------------------------------------------------------------
// thread safe helper to stringify IPv4 address
// NOTE: IP address is expected to be in host order
//------------------------------------------------------------------------------
char *
ipv4addr2str (ipv4_addr_t v4_addr)
{
    static thread_local char       ipaddr_str[4][16];
    static thread_local uint8_t    ipaddr_str_next = 0;
    char                           *buf;

    buf = ipaddr_str[ipaddr_str_next++ & 0x3];
    snprintf(buf, 16, "%d.%d.%d.%d",
             ((v4_addr >> 24) & 0xff), ((v4_addr >> 16) & 0xff),
             ((v4_addr >> 8) & 0xff), (v4_addr & 0xff));
    return buf;
}

//------------------------------------------------------------------------------
// thread safe helper to stringify IPv6 address
//------------------------------------------------------------------------------
char *
ipv6addr2str (ipv6_addr_t v6_addr)
{
    static thread_local char       ipaddr_str[4][40];
    static thread_local uint8_t    ipaddr_str_next = 0;
    char                           *buf;

    buf = ipaddr_str[ipaddr_str_next++ & 0x3];
    sprintf(buf, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
                 "%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                 v6_addr.addr8[0], v6_addr.addr8[1],
                 v6_addr.addr8[2], v6_addr.addr8[3],
                 v6_addr.addr8[4], v6_addr.addr8[5],
                 v6_addr.addr8[6], v6_addr.addr8[7],
                 v6_addr.addr8[8], v6_addr.addr8[9],
                 v6_addr.addr8[10], v6_addr.addr8[11],
                 v6_addr.addr8[12], v6_addr.addr8[13],
                 v6_addr.addr8[14], v6_addr.addr8[15]);

    return buf;
}

//------------------------------------------------------------------------------
// thread safe helper to stringify IP address
//------------------------------------------------------------------------------
char *
ipaddr2str (const ip_addr_t *ip_addr)
{
    switch (ip_addr->af) {
    case IP_AF_IPV4:
        return ipv4addr2str(ip_addr->addr.v4_addr);
        break;

    case IP_AF_IPV6:
        return ipv6addr2str(ip_addr->addr.v6_addr);
        break;

    default:
        return NULL;
        break;
    }
}

//------------------------------------------------------------------------------
// thread safe helper to stringify IP prefix
//------------------------------------------------------------------------------
char *
ippfx2str (const ip_prefix_t *ip_pfx) 
{
    static thread_local char       ippfx_str[4][44];
    static thread_local uint8_t    ippfx_str_next = 0;
    char                           *buf, *ip_addr_buf;

    buf = ippfx_str[ippfx_str_next++ & 0x3];

    ip_addr_buf = ipaddr2str(&ip_pfx->addr);
    sprintf(buf, "%s/%d", ip_addr_buf, ip_pfx->len);

    return buf;
}
