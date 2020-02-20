//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/fte/fte.hpp"
#include "nic/fte/fte_flow.hpp"
#include "nic/include/pkt_hdrs.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"

using namespace fte;

typedef struct vxlan_header_s {
    uint64_t  flags:8;
    uint64_t  rsvd:24;
    uint64_t  vni:24;
    uint8_t   rsvd1:8;
} __PACK__ vxlan_header_t;

#define TCP_IPV4_PKT_SZ  (sizeof(ether_header_t)+sizeof(ipv4_header_t)+\
                          sizeof(tcp_header_t))
#define ICMP_IPV4_PKT_SZ (sizeof(ether_header_t)+sizeof(ipv4_header_t)+\
                          sizeof(icmp_header_t))
#define UDP_IPV4_PKT_SZ (sizeof(ether_header_t)+sizeof(ipv4_header_t)+\
                          sizeof(udp_header_t))
#define ICMP_DATA_SZ     64
#define DOT1Q_HDR_SZ     4 
#define VXLAN_HDR_SZ     (sizeof(ether_header_t)+sizeof(ipv4_header_t)+\
                          sizeof(udp_header_t)+sizeof(vxlan_header_t))

namespace hal {
namespace plugins {
namespace sfw {

static uint32_t
net_sfw_build_vxlan_hdr (uint8_t *pkt, const header_push_info_t push_info)
{
    uint32_t        offset = 0;
    vxlan_header_t *vxlan_hdr = NULL;
    vlan_header_t  *outer_vlan_hdr = NULL;
    ether_header_t *outer_eth_hdr = NULL;
    ipv4_header_t  *outer_ip_hdr = NULL;
    udp_header_t   *outer_udp_hdr = NULL;

    if (push_info.valid_flds.vlan_id) {
        outer_vlan_hdr = (vlan_header_t *)pkt;
        memcpy(outer_vlan_hdr->smac, push_info.ether.smac.ether_addr_octet, ETH_ADDR_LEN);
        memcpy(outer_vlan_hdr->dmac, push_info.ether.dmac.ether_addr_octet, ETH_ADDR_LEN);
        outer_vlan_hdr->etype = htons(ETH_TYPE_IPV4);
        outer_vlan_hdr->tpid = htons(ETH_TYPE_DOT1Q);
        outer_vlan_hdr->vlan_tag = htons(push_info.ether.vlan_id);
        offset += sizeof(vlan_header_t);
    } else {
        outer_eth_hdr = (ether_header_t *)pkt;
        memcpy(outer_eth_hdr->smac, push_info.ether.smac.ether_addr_octet, ETH_ADDR_LEN);
        memcpy(outer_eth_hdr->dmac, push_info.ether.dmac.ether_addr_octet, ETH_ADDR_LEN);
        outer_eth_hdr->etype = htons(ETH_TYPE_IPV4);
        offset += sizeof(vlan_header_t);
    }

    outer_ip_hdr = (ipv4_header_t *)(pkt + offset);
    outer_ip_hdr->version = 4;
    outer_ip_hdr->ihl = sizeof(ipv4_header_t)/4;
    outer_ip_hdr->tot_len = htons((outer_ip_hdr->ihl << 2) + TCP_HDR_MIN_LEN);
    outer_ip_hdr->saddr = htonl(push_info.ipv4.sip);
    outer_ip_hdr->daddr = htonl(push_info.ipv4.dip);
    outer_ip_hdr->id = htons((uint16_t)rand());
    outer_ip_hdr->frag_off = 0;
    outer_ip_hdr->ttl = IP_DEFAULT_TTL;
    outer_ip_hdr->protocol = IP_PROTO_UDP;
    outer_ip_hdr->check = 0;    // let P4 datapath compute checksum
    offset += sizeof(ipv4_header_t);

    outer_udp_hdr = (udp_header_t *)(pkt + offset);
    outer_udp_hdr->sport = htons((uint16_t)rand());
    outer_udp_hdr->dport = htons(UDP_PORT_VXLAN);
    offset += sizeof(udp_header_t);

    vxlan_hdr = (vxlan_header_t *)(pkt + offset);
    vxlan_hdr->vni = push_info.vxlan.vrf_id;
    offset += sizeof(vxlan_header_t);

    return offset;
}

static uint32_t
net_sfw_build_eth_hdr (ctx_t& ctx, uint8_t *pkt, const header_rewrite_info_t rewrite_info, 
                       hal::flow_role_t role=hal::FLOW_ROLE_RESPONDER)
{
    ether_header_t   *pkt_eth_hdr = NULL, *eth_hdr = NULL;
    uint32_t          offset = 0;
    vlan_header_t    *vlan_hdr = NULL, *pkt_vlan_hdr = NULL;
    uint8_t           etype = 0;
    hal::flow_key_t   key = ctx.get_key(role);
    uint16_t          l2_offset = (ctx.cpu_rxhdr())->l2_offset;

    etype = htons((key.flow_type == FLOW_TYPE_V4)?ETH_TYPE_IPV4:ETH_TYPE_IPV6);
    if (ctx.cpu_rxhdr()->flags&CPU_FLAGS_VLAN_VALID) {
        pkt_vlan_hdr = (vlan_header_t *)(ctx.pkt() + l2_offset);
        vlan_hdr = (vlan_header_t *)pkt;
        vlan_hdr->tpid = htons(ETH_TYPE_DOT1Q);
        vlan_hdr->vlan_tag = pkt_vlan_hdr->vlan_tag;
        vlan_hdr->etype = etype;
        offset = sizeof(vlan_header_t);
        eth_hdr = (ether_header_t *)vlan_hdr;
    } else {
        pkt_eth_hdr = (ether_header_t *)(ctx.pkt() + l2_offset);
        eth_hdr = (ether_header_t *)(pkt);
        eth_hdr->etype = etype;
        offset = sizeof(ether_header_t);
    }

    pkt_eth_hdr = (ether_header_t *)(ctx.pkt() + l2_offset);
    if (role == hal::FLOW_ROLE_RESPONDER) {
        memcpy(eth_hdr->smac, pkt_eth_hdr->dmac, ETH_ADDR_LEN);
        memcpy(eth_hdr->dmac, pkt_eth_hdr->smac, ETH_ADDR_LEN);
    } else {
        memcpy(eth_hdr->smac, pkt_eth_hdr->smac, ETH_ADDR_LEN);
        memcpy(eth_hdr->dmac, pkt_eth_hdr->dmac, ETH_ADDR_LEN);
    }

    return offset;
}

//-------------------------
// form a TCP packet
//-------------------------
uint32_t
net_sfw_build_tcp_rst (ctx_t& ctx, uint8_t **pkt_p,
                       const header_rewrite_info_t rewrite_info,
                       const header_push_info_t push_info)
{
    uint16_t           l4_offset = ctx.cpu_rxhdr()->l4_offset;
    ipv4_header_t     *ip_hdr = NULL;
    tcp_header_t      *tcp_hdr = NULL, *pkt_tcp_hdr = NULL;
    hal::flow_key_t    key = ctx.get_key(hal::FLOW_ROLE_RESPONDER);
    //bool               vxlan_valid = false;
    uint32_t           offset = 0, pkt_len = 0;
    uint8_t           *pkt = NULL;

    /*
     * TBD: Move this to use shared mem when driver
     * has the support
     */
    pkt_len = TCP_IPV4_PKT_SZ;

#if 0
    if (valid_tunnel_headers(push_info.valid_hdrs) &&
        ((push_info.valid_hdrs&FTE_ENCAP_HEADERS) == FTE_HEADER_vxlan)) {
         vxlan_valid = true;
         pkt_len += VXLAN_HDR_SZ;
         // Outer Dot1q
         if (push_info.valid_flds.vlan_id) {
             pkt_len += DOT1Q_HDR_SZ;
         }
    }

    // Inner Dot1Q
    if (rewrite_info.valid_flds.vlan_id) {
        pkt_len += DOT1Q_HDR_SZ;
    }
#endif

    if (ctx.cpu_rxhdr()->flags&CPU_FLAGS_VLAN_VALID) {
        pkt_len += DOT1Q_HDR_SZ;
    }

    pkt = (uint8_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_SFW, pkt_len);
    if (!pkt)
        return 0;

#if 0
    // Get the outer header
    if (vxlan_valid)
        offset += net_sfw_build_vxlan_hdr(pkt, push_info);
#endif

    //get the eth type
    offset += net_sfw_build_eth_hdr(ctx, (pkt + offset), 
                                    rewrite_info);

    // fix the IP header
    if (key.flow_type == FLOW_TYPE_V4) {
        ip_hdr = (ipv4_header_t *)(pkt + offset);
        ip_hdr->version = 4;
        ip_hdr->ihl = sizeof(ipv4_header_t)/4;
        ip_hdr->tot_len = htons((ip_hdr->ihl << 2) + TCP_HDR_MIN_LEN);
        if ((rewrite_info.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
            ip_hdr->saddr = htonl(rewrite_info.ipv4.sip);
            ip_hdr->daddr = htonl(rewrite_info.ipv4.dip);
        } else {
            ip_hdr->saddr = htonl(key.sip.v4_addr);
            ip_hdr->daddr = htonl(key.dip.v4_addr);
        }
        ip_hdr->id = htons((uint16_t)rand());
        ip_hdr->frag_off = 0;
        ip_hdr->ttl = IP_DEFAULT_TTL;
        ip_hdr->protocol = IP_PROTO_TCP;
        ip_hdr->check = 0;    // let P4 datapath compute checksum
        offset += sizeof(ipv4_header_t);
    } else {
        // no IPv6 support
        return HAL_RET_INVALID_ARG;
    }

    // fix the TCP header
    pkt_tcp_hdr = (tcp_header_t *)(ctx.pkt() + l4_offset);
    tcp_hdr = (tcp_header_t *)(pkt + offset);
    if ((rewrite_info.valid_hdrs&FTE_L4_HEADERS) == FTE_HEADER_tcp) {
        tcp_hdr->sport = htons(rewrite_info.tcp.sport);
        tcp_hdr->dport = htons(rewrite_info.tcp.dport);
    } else {
        tcp_hdr->sport = htons(key.sport);
        tcp_hdr->dport = htons(key.dport);
    }
    tcp_hdr->seq = htonl(rand());
    tcp_hdr->ack_seq = htonl(ntohl(pkt_tcp_hdr->seq) + 1);
    tcp_hdr->doff = 5;    // TCP header size in words
    tcp_hdr->res1 = 0;
    tcp_hdr->cwr = 0;
    tcp_hdr->ece = 0;
    tcp_hdr->urg = 0;
    tcp_hdr->ack = 0;
    tcp_hdr->psh = 0;
    tcp_hdr->rst = 1;
    tcp_hdr->syn = 0;
    tcp_hdr->fin = 0;

    *pkt_p = pkt;
    return pkt_len;
}

//-------------------------
// form a UDP packet
//-------------------------
uint32_t
net_sfw_build_udp_pkt (ctx_t& ctx, uint8_t *pkt, uint32_t len, 
                       const header_rewrite_info_t rewrite_info,
                       const header_push_info_t push_info)
{
    uint16_t           l3_offset = ctx.cpu_rxhdr()->l3_offset;
    uint16_t           l4_offset = ctx.cpu_rxhdr()->l4_offset;
    ipv4_header_t     *ip_hdr = NULL, *pkt_ip_hdr = NULL;
    udp_header_t      *udp_hdr = NULL, *pkt_udp_hdr = NULL;
    hal::flow_key_t    key = ctx.get_key(hal::FLOW_ROLE_INITIATOR);
    //bool               vxlan_valid = false;
    uint32_t           offset = 0, pkt_len = 0;
    uint32_t           payload_sz = (ctx.pkt_len()-ctx.cpu_rxhdr()->payload_offset);
    uint8_t            *payload = NULL;

    /*
     * TBD: Move this to use shared mem when driver
     * has the support
     */
    pkt_len = UDP_IPV4_PKT_SZ;

#if 0
    if (valid_tunnel_headers(push_info.valid_hdrs) &&
        ((push_info.valid_hdrs&FTE_ENCAP_HEADERS) == FTE_HEADER_vxlan)) {
         vxlan_valid = true;
         pkt_len += VXLAN_HDR_SZ;
         // Outer Dot1q
         if (push_info.valid_flds.vlan_id) {
             pkt_len += DOT1Q_HDR_SZ;
         }
    }

    // Inner Dot1Q
    if (rewrite_info.valid_flds.vlan_id) {
        pkt_len += DOT1Q_HDR_SZ;
    }
#endif

    if (ctx.cpu_rxhdr()->flags&CPU_FLAGS_VLAN_VALID) {
        pkt_len += DOT1Q_HDR_SZ;
    }

    pkt_len += payload_sz;
    if (pkt_len > len) {
        return 0;
    }

#if 0
    // Get the outer header
    if (vxlan_valid)
        offset += net_sfw_build_vxlan_hdr(pkt, push_info);
#endif

    //get the eth type
    offset += net_sfw_build_eth_hdr(ctx, (pkt + offset), rewrite_info,
                                    hal::FLOW_ROLE_INITIATOR);

    // fix the IP header
    if (key.flow_type == FLOW_TYPE_V4) {
        ip_hdr = (ipv4_header_t *)(pkt + offset);
        pkt_ip_hdr = (ipv4_header_t *)(ctx.pkt() + l3_offset);
        memcpy(ip_hdr, pkt_ip_hdr, sizeof(ipv4_header_t));
        if ((rewrite_info.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
            ip_hdr->saddr = htonl(rewrite_info.ipv4.sip);
            ip_hdr->daddr = htonl(rewrite_info.ipv4.dip);
        }
        ip_hdr->check = 0; // let P4 datapath compute checksum
        offset += sizeof(ipv4_header_t);
    } else {
        // no IPv6 support
        return HAL_RET_INVALID_ARG;
    }

    // fix the UDP header
    udp_hdr = (udp_header_t *)(pkt + offset);
    pkt_udp_hdr = (udp_header_t *)(ctx.pkt() + l4_offset);
    memcpy(udp_hdr, pkt_udp_hdr, sizeof(udp_header_t));
    if ((rewrite_info.valid_hdrs&FTE_L4_HEADERS) == FTE_HEADER_udp) {
        udp_hdr->sport = htons(rewrite_info.udp.sport);
        udp_hdr->dport = htons(rewrite_info.udp.dport);
    }
    offset += sizeof(udp_header_t);

    payload = (uint8_t *)(ctx.pkt()+ctx.cpu_rxhdr()->payload_offset);
    memcpy((pkt+offset), payload, payload_sz); 

    return pkt_len;
}

//-------------------------
// form a TCP packet
//-------------------------
uint32_t
net_sfw_build_tcp_pkt (ctx_t& ctx, uint8_t *pkt, uint32_t len,
                       const header_rewrite_info_t rewrite_info,
                       const header_push_info_t push_info)
{
    uint16_t           l3_offset = ctx.cpu_rxhdr()->l3_offset;
    uint16_t           l4_offset = ctx.cpu_rxhdr()->l4_offset;
    ipv4_header_t     *ip_hdr = NULL, *pkt_ip_hdr = NULL;
    tcp_header_t      *tcp_hdr = NULL, *pkt_tcp_hdr = NULL;
    hal::flow_key_t    key = ctx.get_key(hal::FLOW_ROLE_INITIATOR);
    //bool               vxlan_valid = false;
    uint32_t           offset = 0, pkt_len = 0;
    uint32_t           payload_sz = (ctx.pkt_len()-ctx.cpu_rxhdr()->payload_offset);
    uint8_t            *payload = NULL;

    /*
     * TBD: Move this to use shared mem when driver
     * has the support
     */
    pkt_len = TCP_IPV4_PKT_SZ;

#if 0
    if (valid_tunnel_headers(push_info.valid_hdrs) &&
        ((push_info.valid_hdrs&FTE_ENCAP_HEADERS) == FTE_HEADER_vxlan)) {
         vxlan_valid = true;
         pkt_len += VXLAN_HDR_SZ;
         // Outer Dot1q
         if (push_info.valid_flds.vlan_id) {
             pkt_len += DOT1Q_HDR_SZ;
         }
    }

    // Inner Dot1Q
    if (rewrite_info.valid_flds.vlan_id) {
        pkt_len += DOT1Q_HDR_SZ;
    }
#endif

    if (ctx.cpu_rxhdr()->flags&CPU_FLAGS_VLAN_VALID) {
        pkt_len += DOT1Q_HDR_SZ;
    }

    pkt_tcp_hdr = (tcp_header_t *)(ctx.pkt() + l4_offset);
    pkt_len += ((pkt_tcp_hdr->doff*4)-sizeof(tcp_header_t));
    pkt_len += payload_sz;
    if (pkt_len > len) {
        return 0;
    }

#if 0
    // Get the outer header
    if (vxlan_valid)
        offset += net_sfw_build_vxlan_hdr(pkt, push_info);
#endif

    //get the eth type
    offset += net_sfw_build_eth_hdr(ctx, (pkt + offset), 
                                    rewrite_info, hal::FLOW_ROLE_INITIATOR);

    // fix the IP header
    if (key.flow_type == FLOW_TYPE_V4) {
        ip_hdr = (ipv4_header_t *)(pkt + offset);
        pkt_ip_hdr = (ipv4_header_t *)(ctx.pkt()+l3_offset);
        memcpy(ip_hdr, pkt_ip_hdr, sizeof(ipv4_header_t));
        if ((rewrite_info.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
            ip_hdr->saddr = htonl(rewrite_info.ipv4.sip);
            ip_hdr->daddr = htonl(rewrite_info.ipv4.dip);
        }
        ip_hdr->check = 0; // let P4 datapath compute checksum
        offset += sizeof(ipv4_header_t);
    } else {
        // no IPv6 support
        return HAL_RET_INVALID_ARG;
    }

    // fix the TCP header
    tcp_hdr = (tcp_header_t *)(pkt + offset);
    pkt_tcp_hdr = (tcp_header_t *)(ctx.pkt() + l4_offset);
    memcpy(tcp_hdr, pkt_tcp_hdr, (pkt_tcp_hdr->doff*4));
    if ((rewrite_info.valid_hdrs&FTE_L4_HEADERS) == FTE_HEADER_tcp) {
        tcp_hdr->sport = htons(rewrite_info.udp.sport);
        tcp_hdr->dport = htons(rewrite_info.udp.dport);
    }
    tcp_hdr->check = 0;
    offset += (pkt_tcp_hdr->doff*4);

    payload = (uint8_t *)(ctx.pkt()+ctx.cpu_rxhdr()->payload_offset);
    memcpy((pkt+offset), payload, payload_sz);

    return pkt_len;
}    

uint32_t
net_sfw_build_icmp_error(ctx_t& ctx, uint8_t **pkt_p,
                         const header_rewrite_info_t rewrite_info,
                         const header_push_info_t push_info)
{
    hal::flow_key_t    key = ctx.get_key(hal::FLOW_ROLE_RESPONDER);
    uint16_t           l3_offset = (ctx.cpu_rxhdr())->l3_offset;
    ipv4_header_t     *ip_hdr = NULL;
    icmp_header_t     *icmp_hdr = NULL;
    uint8_t           *icmp_data = NULL, *pkt = NULL;
    uint8_t            icmp_data_sz;
    bool               vxlan_valid = false;
    uint32_t           offset = 0, pkt_len = 0;

    icmp_data_sz = ((ctx.pkt_len()-l3_offset)>ICMP_DATA_SZ)?\
               ICMP_DATA_SZ:(ctx.pkt_len()-l3_offset);
    pkt_len = ICMP_IPV4_PKT_SZ + icmp_data_sz;

    if (valid_tunnel_headers(push_info.valid_hdrs) &&
        ((push_info.valid_hdrs&FTE_ENCAP_HEADERS) == FTE_HEADER_vxlan)) {
         vxlan_valid = true;
         pkt_len += VXLAN_HDR_SZ;
         // Outer Dot1q
         if (push_info.valid_flds.vlan_id) {
             pkt_len += DOT1Q_HDR_SZ;
         }
    }

    // Inner Dot1Q
    if (rewrite_info.valid_flds.vlan_id) {
        pkt_len += DOT1Q_HDR_SZ;
    }
    pkt = (uint8_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_SFW, pkt_len);
    if (!pkt)
        return 0;

    // Get the outer header
    if (vxlan_valid)
        offset += net_sfw_build_vxlan_hdr(pkt, push_info);

    //get the eth type
    offset += net_sfw_build_eth_hdr(ctx, (pkt+offset), rewrite_info);

    // fix the IP header
    if (key.flow_type == FLOW_TYPE_V4) {
        ip_hdr = (ipv4_header_t *)(pkt + offset);
        ip_hdr->version = 4;
        ip_hdr->ihl = sizeof(ipv4_header_t)/4;
        ip_hdr->tot_len = htons((ip_hdr->ihl << 2) + \
                      (sizeof(icmp_header_t) + icmp_data_sz));
        if ((rewrite_info.valid_hdrs&FTE_L3_HEADERS) == FTE_HEADER_ipv4) {
            ip_hdr->saddr = htonl(rewrite_info.ipv4.sip);
            ip_hdr->daddr = htonl(rewrite_info.ipv4.dip);
        } else {
            ip_hdr->saddr = htonl(key.sip.v4_addr);
            ip_hdr->daddr = htonl(key.dip.v4_addr);
        }
        ip_hdr->id = htons((uint16_t)rand());
        ip_hdr->frag_off = 0;
        ip_hdr->ttl = IP_DEFAULT_TTL;
        ip_hdr->protocol = IP_PROTO_ICMP;
        ip_hdr->check = 0;    // let P4 datapath compute checksum
        offset += sizeof(ipv4_header_t);
    } else {
        // no IPv6 support
        return HAL_RET_INVALID_ARG;
    }

    icmp_hdr = (icmp_header_t *)(pkt + offset);
    icmp_hdr->type = 3; // Destination unrecheable
    icmp_hdr->code = 9; // Communication with Destination Network is Administratively Prohibited
    icmp_hdr->checksum = 0;
    offset += sizeof(icmp_header_t);

    icmp_data = (uint8_t *)(pkt + offset);
    memcpy(icmp_data, (ctx.pkt() + l3_offset), icmp_data_sz);

    *pkt_p = pkt;
    return pkt_len;
}

//------------------------------------------------------------------------------
// Free the packet buffer once the packet is queued
//------------------------------------------------------------------------------
void
net_sfw_free_reject_pkt (uint8_t *pkt)
{
    HAL_FREE(hal::HAL_MEM_ALLOC_SFW, pkt);
}

}
}
}
