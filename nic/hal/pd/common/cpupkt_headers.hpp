#pragma once

#include "nic/include/base.h"
#include <sys/param.h>
#include "nic/include/ip.h"

namespace hal {
namespace pd {

typedef struct p4_to_p4plus_cpu_pkt_s {
    uint16_t   src_lif;

    uint16_t  lif;
    uint8_t   qtype;
    uint32_t  qid;

    uint16_t  lkp_vrf;

#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t   pad      : 2;
    uint8_t   lkp_dir  : 1;
    uint8_t   lkp_inst : 1;
    uint8_t   lkp_type : 4;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t   lkp_type : 4;
    uint8_t   lkp_inst : 1;
    uint8_t   lkp_dir  : 1;
    uint8_t   pad      : 2;
#else
#error "architecture unknown"
#endif

    uint8_t   flags;

    // offsets
    int16_t  l2_offset;
    int16_t  l3_offset;
    int16_t  l4_offset;
    int16_t  payload_offset;

    // tcp
    uint8_t   tcp_flags;
    uint32_t  tcp_seq_num;
    uint32_t  tcp_ack_num;
    uint16_t  tcp_window;
    uint8_t   tcp_options;
    uint16_t  tcp_mss;
    uint8_t   tcp_ws;
} __attribute__ ((__packed__)) p4_to_p4plus_cpu_pkt_t;

typedef struct p4plus_to_p4_header_s {
#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t     p4plus_app_id   : 4;
    uint8_t     pad             : 4;
#else
    uint8_t     pad             : 4;
    uint8_t     p4plus_app_id   : 4;
#endif

    uint8_t     flags;
    uint16_t    ip_id;
    uint16_t    ip_len;
    uint16_t    udp_len;
    uint32_t    tcp_seq_delta;
    uint16_t    vlan_tag;
} __attribute__ ((__packed__)) p4plus_to_p4_header_t;
    
typedef struct cpu_to_p4plus_header_s {
    uint16_t    flags;
    uint16_t    src_lif;
    uint16_t    hw_vlan_id;
    uint16_t    l2_offset;
} __attribute__ ((__packed__)) cpu_to_p4plus_header_t;

} // namespace pd       
} // namespace hal

// These are temporary until skbuf is added to hal
struct ether_header_t
{
    uint8_t  dmac[ETH_ADDR_LEN];      /* destination eth addr */
    uint8_t  smac[ETH_ADDR_LEN];      /* source ether addr    */
    uint16_t etype;                   /* ether type */
} __attribute__ ((__packed__));

struct vlan_header_t
{
    uint8_t  dmac[ETH_ADDR_LEN];      /* destination eth addr */
    uint8_t  smac[ETH_ADDR_LEN];      /* source ether addr    */
    uint16_t tpid;                    /* Tag protocol id*/
    uint16_t vlan_tag;                /* dot1p +cfi + vlan-id */
    uint16_t etype;                   /* ether type */
} __attribute__ ((__packed__));

struct ipv4_header_t {
#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t    version:4;
    uint8_t    ihl:4;
#else
    uint8_t    ihl:4;
    uint8_t    version:4;
#endif
    uint8_t    tos;
    uint16_t   tot_len;
    uint16_t   id;
    uint16_t   frag_off;
    uint8_t    ttl;
    uint8_t    protocol;
    uint16_t   check;
    uint32_t   saddr;
    uint32_t   daddr;
    /*The options start here. */
}__attribute__ ((__packed__));

struct ipv6_header_t {
#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t    version:4;
    uint8_t    priority:4;
#else
    uint8_t    priority:4;
    uint8_t    version:4;
#endif
    uint8_t    flow_lbl[3];

    uint16_t   payload_len;
    uint8_t    nexthdr;
    uint8_t    hop_limit;

    uint8_t    saddr[IP6_ADDR8_LEN];
    uint8_t    daddr[IP6_ADDR8_LEN];
}__attribute__ ((__packed__));
    
struct tcp_header_t {
    uint16_t  sport;
    uint16_t  dport;
    uint32_t  seq;
    uint32_t  ack_seq;
#if __BYTE_ORDER == __BIG_ENDIAN
    uint16_t   doff:4,
        res1:4,
        cwr:1,
        ece:1,
        urg:1,
        ack:1,
        psh:1,
        rst:1,
        syn:1,
        fin:1;
#else
    uint16_t   res1:4,
        doff:4,
        fin:1,
        syn:1,
        rst:1,
        psh:1,
        ack:1,
        urg:1,
        ece:1,
        cwr:1;
#endif
    uint16_t  window;
    uint16_t  check;
    uint16_t  urg_ptr;
}__attribute__ ((__packed__));

struct udp_header_t {
    uint16_t  sport;
    uint16_t  dport;
    uint16_t  len;
    uint16_t  check;
}__attribute__ ((__packed__));

struct icmp_header_t {
    uint8_t          type;
    uint8_t          code;
    uint16_t         checksum;
    union {
        struct {
            uint16_t  id;
            uint16_t  sequence;
        } echo;
        uint32_t  gateway;
        struct {
            uint16_t  __unused;
            uint16_t  mtu;
        } frag;
    };
}__attribute__ ((__packed__));

struct ipsec_esp_header_t {
    uint32_t spi;
    uint32_t seqno;
}__attribute__ ((__packed__));
