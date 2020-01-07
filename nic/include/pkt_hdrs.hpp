// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __PKT_HDRS_HPP__
#define __PKT_HDRS_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"

typedef struct ether_header_s {
    uint8_t  dmac[ETH_ADDR_LEN];      // destination eth addr
    uint8_t  smac[ETH_ADDR_LEN];      // source ether addr
    uint16_t etype;                   // ether type
} __PACK__ ether_header_t;

typedef struct vlan_header_s {
    uint8_t  dmac[ETH_ADDR_LEN];      // destination eth addr
    uint8_t  smac[ETH_ADDR_LEN];      // source ether addr
    uint16_t tpid;                    // Tag protocol id
    uint16_t vlan_tag;                // dot1p +cfi + vlan-id
    uint16_t etype;                   // ether type
} __PACK__ vlan_header_t;

typedef struct ipv4_header_s {
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
} __PACK__ ipv4_header_t;

// minimum IPv4 header length
#define IPV4_HDR_MIN_LEN                             20
#define IPV4_HDR_MIN_LEN_IN_WORDS                    5

typedef struct ipv6_header_s {
#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t    version:4;
    uint8_t    tc_high:4;
    uint8_t    tc_low:4;
    uint8_t    flow_lbl_high:4;
#else
    uint8_t    tc_high:4;
    uint8_t    version:4;
    uint8_t    flow_lbl_high:4;
    uint8_t    tc_low:4;
#endif
    uint8_t    flow_lbl[2];
    uint16_t   payload_len;
    uint8_t    nexthdr;
    uint8_t    hop_limit;

    uint8_t    saddr[IP6_ADDR8_LEN];
    uint8_t    daddr[IP6_ADDR8_LEN];
} __PACK__ ipv6_header_t;

typedef struct tcp_header_s {
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
} __PACK__ tcp_header_t;

// minimum TCP header length
#define TCP_HDR_MIN_LEN                             20
#define TCP_DEFAULT_MSS                             546    // RFC6691
#define TCP_DEFAULT_WINDOW_SIZE                     512

typedef struct udp_header_s {
    uint16_t  sport;
    uint16_t  dport;
    uint16_t  len;
    uint16_t  check;
} __PACK__ udp_header_t;

#define ICMP_MCAST_LISTENER_QUERY      130
#define ICMP_MCAST_LISTENER_REPORT     131
#define ICMP_MCAST_LISTENER_DONE       132
#define ICMP_ROUTER_ADVERTISEMENT      134
#define ICMP_NEIGHBOR_SOLICITATION     135
#define ICMP_NEIGHBOR_ADVERTISEMENT    136

#define DHCP_CLIENT_PORT               67
#define DHCP_SERVER_PORT               68

#define NETBIOS_NS_PORT                137
#define NETBIOS_DS_PORT                138                

#define ICMP_TYPE_ECHO_REQUEST         8
#define ICMP_CODE_ECHO_REQUEST         0
#define ICMP_TYPE_ECHO_RESPONSE        0
#define ICMP_CODE_ECHO_RESPONSE        0
#define ICMPV6_TYPE_ECHO_REQUEST       128
#define ICMPV6_TYPE_ECHO_RESPONSE      129

#define DHCPV6_MCAST_PORT              546
#define DHCPV6_RELAY_PORT              547

typedef struct icmp_header_s {
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
} __PACK__ icmp_header_t;

typedef struct ipsec_esp_header_s {
    uint32_t spi;
    uint32_t seqno;
} __PACK__ ipsec_esp_header_t;

typedef struct tcp_ts_option_s {
    uint8_t   pad1;
    uint8_t   pad2;
    uint8_t   kind;
    uint8_t   length;
    uint32_t  ts_value;
    uint32_t  echo_ts;
} __PACK__ tcp_ts_option_t;

#define TCP_TS_OPTION_LEN 12 

#endif    // __PKT_HDRS_HPP__

