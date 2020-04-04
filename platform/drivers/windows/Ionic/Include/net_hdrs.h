/*
 * Copyright (c) 2017 Pensando Systems, Inc.  All rights reserved.
 */

#ifndef _NET_HDRS_H_
#define _NET_HDRS_H_

#define VLAN_ETH_HLEN   18
#define ETH_HLEN 14
#define ETH_ALEN 6
#define ETH_P_IPv4 0x0800
#define ETH_P_IPv6 0x86dd
#define ETH_P_ARP 0x0806

#define ETH_COMPLETE_HDR  22 // Eth hdr + 1Q hdr + FCS

#define ETH_GET_VLAN_PRIORITY(x) (((x) >> 13) & 0x7)
#define ETH_GET_VLAN_ID(x) ((x) & 0xfff)
#define ETH_MAKE_VLAN(pri, cfi, vlan_id) \
  ((((pri) & 0x7) << 13) | (((cfi) & 0x1) << 12) | ((vlan_id) & 0xfff))

#define ETH_VLAN_ID_MAX 0xfff

 /* try and align the data portion at cacheline boundary */
#define ETH_PKT_DATA_ALIGN      (ionic->dma_alignment)

 /* ETH+IP+TCP hdr is 14+20+20 = 54 bytes (w/o options) this makes the data
 * portion of the packet start from a non 8-byte boundary
 * if we add 10 bytes to the start of the hdr portion we can align
 * the data portion at offset 64 (x40)
 *
 * here is a comment from microsoft website
 * Alignment
 * Buffer alignment on a cache line boundary improves performance when copying
 * data from one buffer to another. Most network adapter receive buffers are
 * properly aligned when they are first allocated, but the user data that must
 * eventually be copied into the application buffer is misaligned due to the
 * header space consumed. With TCP data (the most common scenario) the shift
 * due to the TCP, IP and Ethernet headers results in a shift of 0x36 bytes.
 * In order to resolve this problem for the common case, we recommend that a
 * slightly larger buffer be allocated and packet data be inserted at an
 * offset of 0xa bytes. This will ensure that after shifting the buffers by
 * 0x36 bytes for the header, user data will be properly aligned.
 */
#define ETH_HDR_OFFSET          0xA

 /* extra memory to take care of alignment */
#define ETH_EXTRA_FOR_ALIGN         (ETH_PKT_DATA_ALIGN + ETH_HDR_OFFSET)

 /* Get a 8-bytes aligned memory address from a given the memory address.
 * If the given address is not 8-bytes aligned, return the closest bigger
 * memory address which is 8-bytes aligned.
 */
#define DATA_ALIGN(_va)                                         \
        ((PVOID)(((ULONG_PTR)(_va) + (ETH_PKT_DATA_ALIGN - 1)) &    \
          ~((ULONG_PTR)ETH_PKT_DATA_ALIGN - 1)))

#define BYTES_SHIFT(_new_va, _orig_va) ((PUCHAR)(_new_va) - (PUCHAR)(_orig_va))

#define htons(A) ((((A) & 0xff00) >> 8) | \
                          (((A) & 0x00ff) << 8))

#define htonl(A) ((((A) & 0xff000000) >> 24) | \
                          (((A) & 0x00ff0000) >> 8) | \
                          (((A) & 0x0000ff00) << 8) | \
                          (((A) & 0x000000ff) << 24))

#define ntohs htons
#define ntohl htonl


/*
 *      This is an Ethernet frame header.
 */
struct ethhdr
{
        u8 h_dest[6];       /* destination eth addr */
        u8 h_source[6];     /* source ether addr    */
        u16 h_proto;        /* packet type ID field */
};

#define ether_header ethhdr

#define ethhdr_size		14

/*
*      This is TCP header.
*      taken from tcp.h
*/

struct tcphdr {
    u16   source;
    u16   dest;
    u32   seq;
    u32   ack_seq;
    u16   res1 : 4,
        doff : 4,
        fin : 1,
        syn : 1,
        rst : 1,
        psh : 1,
        ack : 1,
        urg : 1,
        ece : 1,
        cwr : 1;
    u16   window;
    u16   check;
    u16   urg_ptr;
};

#define IPPROTO_TCP         6
#define IPv6PROTO_HOPOPTS   0
#define IPv6PROTO_DSTOPTS   60
#define IPv6PROTO_ROUTING   43
#define IPv6PROTO_FRAGMENT  44
#define IPv6PROTO_ESP       50
#define IPv6PROTO_AUTH      51

#define IPv6_FRAGMENT_HDR_LEN 8
#define IPv6_MAX_HEADER_LEN 512

/*
*      This is an IPv4 header.
*      taken from linux/ip.h
*/
struct ipv4hdr {
    u8  ihl : 4,
        version : 4;
    u8 tos;
    u16 tot_len;
    u16 id;
    u16 frag_off;
    u8 ttl;
    u8 protocol;
    u16 check;
    u32 saddr;
    u32 daddr;
    /*The options start here. */
};

/*
*  IPv6 address structure
*  taken from linux/in6.h
*/
struct in6_addr_gen
{
    union {
        u8  u6_addr8[16];
        u16 u6_addr16[8];
        u32 u6_addr32[4];
    } in6_u;
};

/*
*  This is an IPv6 header.
*  taken from linux/ipv6.h
*/
struct ipv6hdr {
    u8      priority : 4,
            version : 4;
    u8      flow_lbl[3];
    u16     payload_len;
    u8      nexthdr;
    u8      hop_limit;
    struct  in6_addr_gen    saddr;
    struct  in6_addr_gen    daddr;
};

#endif /* _NET_HDRS_H_ */
