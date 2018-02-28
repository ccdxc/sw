// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/pkt_hdrs.hpp"

namespace hal {
namespace utils {

//------------------------------------------------------------------------------
// form a TCP RST packet given a incoming TCP SYN packet
//------------------------------------------------------------------------------
hal_ret_t
tcp_rst_from_syn_pkt (uint8_t *pkt, uint16_t l2_offset,
                      uint16_t l3_offset, uint16_t l4_offset)
{
    ether_header_t    *eth_hdr;
    ipv4_header_t     *ip_hdr;
    tcp_header_t      *tcp_hdr;
    ipv4_addr_t       v4addr;
    uint16_t          port;

    if (!pkt || (l2_offset == 0) || (l3_offset == 0) || (l4_offset == 0)) {
        return HAL_RET_INVALID_ARG;
    }

    // get the eth type
    eth_hdr = (ether_header_t *)(pkt + l2_offset);

    // fix the IP header
    if (eth_hdr->etype == ETH_TYPE_IPV4) {
        ip_hdr = (ipv4_header_t *)(pkt + l3_offset);
        if (ip_hdr->protocol != IP_PROTO_TCP) {
            return HAL_RET_INVALID_ARG;
        }
        ip_hdr->tot_len = (ip_hdr->ihl << 2) + TCP_HDR_MIN_LEN;
        v4addr = ip_hdr->saddr;
        ip_hdr->saddr = ip_hdr->daddr;
        ip_hdr->daddr = v4addr;
        ip_hdr->id = (uint16_t)rand();
        ip_hdr->frag_off = 0;
        ip_hdr->ttl = IP_DEFAULT_TTL;
        ip_hdr->check = 0;    // let P4 datapath compute checksum
    } else {
        // no IPv6 support
        return HAL_RET_INVALID_ARG;
    }

    // fix the TCP header
    tcp_hdr = (tcp_header_t *)(pkt + l4_offset);
    if (!tcp_hdr->syn) {
        return HAL_RET_INVALID_ARG;
    }
    port = tcp_hdr->sport;
    tcp_hdr->sport = tcp_hdr->dport;
    tcp_hdr->dport = port;
    tcp_hdr->seq = rand();
    tcp_hdr->ack_seq = htonl(ntohl(tcp_hdr->seq) + 1);
    tcp_hdr->doff = 5;    // TCP header size in words
    tcp_hdr->ack = 1;
    tcp_hdr->rst = 1;
    tcp_hdr->syn = 0;
    tcp_hdr->fin = 0;
    tcp_hdr->window = 0;
    tcp_hdr->check = 0;    // let P4 datapath compute checksum
    tcp_hdr->urg_ptr = 0;

    return HAL_RET_OK;
}

}    // utils
}    // hal
