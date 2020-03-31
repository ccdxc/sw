//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains Athena FTE flow functionality
///
//----------------------------------------------------------------------------
#include <cstring>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <netinet/in.h>
#include <setjmp.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdbool.h>

#include <rte_mbuf.h>
#include <rte_ether.h>
#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_icmp.h>
#include <rte_mpls.h>

#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/sdk/lib/table/ftl/ftl_base.hpp"
#include "fte_athena.hpp"
#include "nic/apollo/api/include/athena/pds_init.h"
#include "nic/apollo/api/include/athena/pds_vnic.h"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
#include "nic/apollo/api/include/athena/pds_flow_session_rewrite.h"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"
#include "gen/p4gen/p4/include/ftl.h"
#include "athena_test.hpp"
#include "app_test_utils.hpp"

namespace fte_ath {

#define IP_PROTOCOL_TCP 0x06
#define IP_PROTOCOL_UDP 0x11
#define IP_PROTOCOL_ICMP 0x01
#define IP_PROTOCOL_ICMPV6 0x3A

#define IPV4_ADDR_LEN 4
#define IPV6_ADDR_LEN 16
#define IPV6_HDR_LEN 40

// Flow direction bitmask
#define HOST_TO_SWITCH 0x1
#define SWITCH_TO_HOST 0x2

#define MAX_VNIC_ID 2

uint16_t g_vlan_to_vnic[] = { 0x0, 0x64, 0x65 };
uint32_t g_mpls_label_to_vnic[] = { 0x0, 0x3E8, 0x3E9 };

fte_sess_rewrite_t g_sess_rewrite[] = {
    {
        { 0 },
        { 0 }
    },
    {
        { 0x1, {0x00, 0x06, 0x07, 0x08, 0x09, 0x0a},
          {0x00, 0x01, 0x02, 0x03, 0x04, 0x05}, 0x01,
          0x04030201, 0x01020304, 0x12345, 0x3E9 },
        { 0x2, {0x00, 0x00, 0x00, 0x40, 0x08, 0x01},
          {0x00, 0x00, 0xF1, 0xD0, 0xD1, 0xD0}, 0x64 }
    },
    {
        { 0x3, {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},
          {0x00, 0x06, 0x07, 0x08, 0x09, 0x0a}, 0x01,
          0x01020304, 0x04030201, 0x54321, 0x3E8 },
        { 0x4, {0x00, 0x00, 0xF1, 0xD0, 0xD1, 0xD0},
          {0x00, 0x00, 0x00, 0x40, 0x08, 0x01}, 0x65 }
    }
};

uint32_t g_session_index = 1;
uint32_t g_session_rewrite_index = 5;

// H2S specific fields
uint32_t g_h2s_vlan = 0x0002;
uint16_t g_h2s_vnic_id = 0x0001;

// H2S Session info rewrite
mac_addr_t substrate_smac = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
mac_addr_t substrate_dmac = {0x00, 0x06, 0x07, 0x08, 0x09, 0x0a};
uint16_t substrate_vlan = 0x02;
uint32_t substrate_sip = 0x04030201;
uint32_t substrate_dip = 0x01020304;
uint8_t substrate_ip_ttl = 64;
uint16_t substrate_udp_sport = 0xabcd;
uint16_t substrate_udp_dport = 0x1234;
uint32_t mpls1_label = 0x12345;
uint32_t mpls2_label = 0x6789a;

// S2H specific fields
uint32_t g_s2h_mpls1_label = 0x12345;
uint32_t g_s2h_mpls2_label = 0x6789a;

// S2H Session info rewrite
mac_addr_t ep_smac = {0x00, 0x00, 0xF1, 0xD0, 0xD1, 0xD0};
mac_addr_t ep_dmac = {0x00, 0x00, 0x00, 0x40, 0x08, 0x01};
uint16_t  vnic_vlan = 0x01;

// Flow dump vars
pds_flow_key_t dump_flow_key;

// Headers used for Packet Rewrite
// Hardcoded based on rewrite table entries
uint8_t h2s_l2vlan_encap_hdr[] = {
    0x00, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x00, 0x01,
    0x02, 0x03, 0x04, 0x05, 0x81, 0x00, 0x00, 0x02,
    0x08, 0x00
};

// Note: Total length & Checksum to be updated accordingly
uint8_t h2s_ip_encap_hdr[] = {
    0x45, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00,
    0x40, 0x11, 0xA2, 0xA0, 0x04, 0x03, 0x02, 0x01,
    0x01, 0x02, 0x03, 0x04
};

// Note: Length to be updated accordingly
uint8_t h2s_udp_encap_hdr[] = {
    0xE4, 0xE7, 0x19, 0xEB, 0x00, 0x60, 0x00, 0x00
};

// Two MPLS headers
uint8_t h2s_mpls_encap_hdrs[] = {
    0x12, 0x34, 0x50, 0x00,
    0x67, 0x89, 0xA1, 0x00
};

uint8_t s2h_l2vlan_encap_hdr[] = {
    0x00, 0x00, 0x00, 0x40, 0x08, 0x01, 0x00, 0x00,
    0xF1, 0xD0, 0xD1, 0xD0, 0x81, 0x00, 0x00, 0x01,
    0x08, 0x00
};

static void
fte_flow_dump (void)
{
    pds_flow_info_t flow_info;

    if (pds_flow_cache_entry_read(&dump_flow_key, &flow_info) != SDK_RET_OK) {
        PDS_TRACE_DEBUG("pds_flow_cache_entry_read failed.\n");
        return;
    }

    // TODO: Dump the values
    PDS_TRACE_DEBUG("pds_flow_cache_entry_read success.\n");
    return;
}

static uint16_t
fte_get_vnic_from_vlan (uint16_t vlan_id)
{
    uint16_t i;

    for (i = 1; i <= MAX_VNIC_ID; i++) {
        if (g_vlan_to_vnic[i] == vlan_id) {
            return i;
        }
    }
    
    return 0;
}
        
static uint16_t
fte_get_vnic_from_mpls_label (uint32_t mpls_label)
{
    uint16_t i;

    for (i = 1; i <= MAX_VNIC_ID; i++) {
        if (g_mpls_label_to_vnic[i] == mpls_label) {
            return i;
        }
    }
    
    return 0;
}
        
static sdk_ret_t
fte_flow_extract_prog_args (struct rte_mbuf *m, pds_flow_spec_t *spec,
                            uint8_t *dir, uint16_t *ip_off, uint16_t *vnic_id)
{
    struct ether_hdr *eth0;
    struct ipv4_hdr *ip40;
    struct tcp_hdr *tcp0;
    struct udp_hdr *udp0;
    struct mpls_hdr *mpls0;
    uint16_t ip0_offset = 0;
    uint16_t vlan_id = 0;
    uint32_t mpls_label = 0;
    uint8_t protocol = 0;
    uint16_t sport = 0, dport = 0;
    pds_flow_key_t *key = &(spec->key);

    // mbuf data starts at eth header
    eth0 = rte_pktmbuf_mtod(m, struct ether_hdr *);
    ip0_offset += sizeof(struct ether_hdr);

    if ((rte_be_to_cpu_16(eth0->ether_type) != ETHER_TYPE_VLAN) &&
        (rte_be_to_cpu_16(eth0->ether_type) != ETHER_TYPE_IPv4) &&
        (rte_be_to_cpu_16(eth0->ether_type) != ETHER_TYPE_IPv6)) {
        PDS_TRACE_DEBUG("Unsupported ether_type:0x%x \n",
                        rte_be_to_cpu_16(eth0->ether_type));
        return SDK_RET_INVALID_OP;
    }

    if ((rte_be_to_cpu_16(eth0->ether_type) == ETHER_TYPE_VLAN)) {
        struct vlan_hdr *vh = (struct vlan_hdr *)(eth0 + 1);

        if ((rte_be_to_cpu_16(vh->eth_proto) != ETHER_TYPE_IPv4) &&
            (rte_be_to_cpu_16(vh->eth_proto) != ETHER_TYPE_IPv6)) {
            PDS_TRACE_DEBUG("Unsupported VLAN eth_proto:0x%x \n",
                            rte_be_to_cpu_16(vh->eth_proto));
            return SDK_RET_INVALID_OP;
        }

        ip0_offset += (sizeof(struct vlan_hdr));
        vlan_id = (rte_be_to_cpu_16(vh->vlan_tci) & 0x0fff);
    }

    ip40 = rte_pktmbuf_mtod_offset(m, struct ipv4_hdr *, ip0_offset);
    udp0 = (struct udp_hdr *)(ip40 + 1);

    if (((ip40->version_ihl >> 4) == 4) &&
        (ip40->next_proto_id == IP_PROTOCOL_UDP) &&
        (rte_be_to_cpu_16(udp0->dst_port) == 0x19EB)) {
        ip0_offset += (sizeof(struct ipv4_hdr)); 
        ip0_offset += (sizeof(struct udp_hdr));

        mpls0 = (struct mpls_hdr *)(udp0 + 1);
        ip0_offset += (sizeof(struct mpls_hdr));

        if (mpls0->bs == 0) {
            struct mpls_hdr *mpls1;

            mpls1 = (struct mpls_hdr *) (mpls0 + 1);
            ip0_offset += (sizeof(struct mpls_hdr));

            if (mpls1->bs == 0) {
                struct mpls_hdr *mpls2;

                mpls2 = (struct mpls_hdr *) (mpls1 + 1);
                ip0_offset += (sizeof(struct mpls_hdr));

                if (mpls2->bs == 0) {
                    PDS_TRACE_DEBUG("Unsupported: MPLS lables > 3.");
                    return SDK_RET_INVALID_OP;
                }
            }
            mpls_label = ((rte_be_to_cpu_16(mpls1->tag_msb) << 4) | mpls1->tag_lsb);
            *vnic_id = fte_get_vnic_from_mpls_label(mpls_label);
        }
        *dir = SWITCH_TO_HOST;
        *ip_off = ip0_offset;
    } else {
        *dir = HOST_TO_SWITCH;
        *ip_off = ip0_offset;
        *vnic_id = fte_get_vnic_from_vlan(vlan_id);
    }

    if (*vnic_id == 0) {
        PDS_TRACE_DEBUG("vnic_id lookup failed.\n");
        return SDK_RET_ERR;
    }

    ip40 = rte_pktmbuf_mtod_offset(m, struct ipv4_hdr *, ip0_offset);
    if ((ip40->version_ihl >> 4) == 4) {
        uint32_t src_ip, dst_ip;

        protocol = ip40->next_proto_id;
        if ((protocol != IP_PROTOCOL_TCP) &&
            (protocol != IP_PROTOCOL_UDP) && 
            (protocol != IP_PROTOCOL_ICMP)) {
            PDS_TRACE_DEBUG("Unsupported IP Proto:%u\n", protocol);
            return SDK_RET_INVALID_OP;
        }
        
        src_ip = rte_be_to_cpu_32(ip40->src_addr);
        dst_ip = rte_be_to_cpu_32(ip40->dst_addr);
        key->key_type = KEY_TYPE_IPV4;
        if (*dir == HOST_TO_SWITCH) {
            memcpy(key->ip_saddr, &src_ip, IPV4_ADDR_LEN);
            memcpy(key->ip_daddr, &dst_ip, IPV4_ADDR_LEN);
        }
        else {
            memcpy(key->ip_saddr, &dst_ip, IPV4_ADDR_LEN);
            memcpy(key->ip_daddr, &src_ip, IPV4_ADDR_LEN);
        }

        tcp0 = (struct tcp_hdr *) (((uint8_t *) ip40) +
                ((ip40->version_ihl & IPV4_HDR_IHL_MASK) *
                IPV4_IHL_MULTIPLIER));
    } else {
        struct ipv6_hdr *ip60 = (struct ipv6_hdr *)ip40;

        protocol = ip60->proto;
        if ((protocol != IP_PROTOCOL_TCP) &&
            (protocol != IP_PROTOCOL_UDP) && 
            (protocol != IP_PROTOCOL_ICMPV6)) {
            PDS_TRACE_DEBUG("Unsupported IPV6 Proto:%u\n", protocol);
            return SDK_RET_INVALID_OP;
        }

        key->key_type = KEY_TYPE_IPV6;
        if (*dir == HOST_TO_SWITCH) {
            sdk::lib::memrev(key->ip_saddr, ip60->src_addr, IPV6_ADDR_LEN);
            sdk::lib::memrev(key->ip_daddr, ip60->dst_addr, IPV6_ADDR_LEN);
        } else {
            sdk::lib::memrev(key->ip_saddr, ip60->dst_addr, IPV6_ADDR_LEN);
            sdk::lib::memrev(key->ip_daddr, ip60->src_addr, IPV6_ADDR_LEN);
        }

        tcp0 = (struct tcp_hdr *) (((uint8_t *) ip60) +
                    IPV6_HDR_LEN);
    }

    key->ip_proto = protocol;
    if ((protocol == IP_PROTOCOL_ICMP) ||
        (protocol == IP_PROTOCOL_ICMPV6)) {
        struct icmp_hdr *icmph = ((struct icmp_hdr *)tcp0);

        key->l4.icmp.type = icmph->icmp_type;
        key->l4.icmp.code = icmph->icmp_code;
        key->l4.icmp.identifier =
            (rte_be_to_cpu_16(icmph->icmp_ident) & 0xff);
    } else {
        sport = rte_be_to_cpu_16(tcp0->src_port);
        dport = rte_be_to_cpu_16(tcp0->dst_port);
        if (*dir == HOST_TO_SWITCH) {
            key->l4.tcp_udp.sport = sport;
            key->l4.tcp_udp.dport = dport;
        } else {
            key->l4.tcp_udp.sport = dport;
            key->l4.tcp_udp.dport = sport;
        }
    }

    // TODO: To be reomved. Debug purpose
    memcpy(&dump_flow_key, &(spec->key), sizeof(pds_flow_key_t));

    return SDK_RET_OK;
}

static void
fte_flow_h2s_rewrite_mplsoudp (struct rte_mbuf *m, uint16_t ip_offset, uint16_t vnic_id)
{
    uint16_t total_encap_len;
    uint16_t mbuf_prepend_len;
    uint8_t *pkt_start;
    struct ether_hdr *etherh;
    struct vlan_hdr *vlanh;
    struct ipv4_hdr *ip4h;
    struct udp_hdr *udph;
    struct mpls_hdr *mplsh;
    uint16_t ip_tot_len, udp_len;
    fte_h2s_sess_rewrite_t *h2s_sess_rewrite;

    h2s_sess_rewrite = &(g_sess_rewrite[vnic_id].h2s_sess_rewrite);
    total_encap_len = (sizeof(h2s_l2vlan_encap_hdr) +
                        sizeof(h2s_ip_encap_hdr) + 
                        sizeof(h2s_udp_encap_hdr) +
                        sizeof(h2s_mpls_encap_hdrs));

    mbuf_prepend_len = (total_encap_len - ip_offset);
    pkt_start = (uint8_t *)rte_pktmbuf_prepend(m, mbuf_prepend_len);
    etherh = (struct ether_hdr *)pkt_start;
    vlanh = (struct vlan_hdr *)(etherh + 1);

    memcpy(pkt_start, h2s_l2vlan_encap_hdr,
           sizeof(h2s_l2vlan_encap_hdr));
    memcpy(etherh->d_addr.addr_bytes, (uint8_t *)&(h2s_sess_rewrite->substrate_dmac), ETHER_ADDR_LEN);
    memcpy(etherh->s_addr.addr_bytes, (uint8_t *)&(h2s_sess_rewrite->substrate_smac), ETHER_ADDR_LEN);
    vlanh->vlan_tci = (rte_be_to_cpu_16(vlanh->vlan_tci) & 0xf000);
    vlanh->vlan_tci |= (h2s_sess_rewrite->substrate_vlan & 0x0fff);
    vlanh->vlan_tci = rte_cpu_to_be_16(vlanh->vlan_tci);
    pkt_start += sizeof(h2s_l2vlan_encap_hdr);

    memcpy(pkt_start, h2s_ip_encap_hdr, sizeof(h2s_ip_encap_hdr));
    ip4h = (struct ipv4_hdr *)pkt_start;
    ip_tot_len = (m->pkt_len - sizeof(h2s_l2vlan_encap_hdr));
    ip4h->total_length = rte_cpu_to_be_16(ip_tot_len);
    ip4h->src_addr = rte_cpu_to_be_32(h2s_sess_rewrite->substrate_sip);
    ip4h->dst_addr = rte_cpu_to_be_32(h2s_sess_rewrite->substrate_dip);
    pkt_start += sizeof(h2s_ip_encap_hdr);

    memcpy(pkt_start, h2s_udp_encap_hdr, sizeof(h2s_udp_encap_hdr));
    udph = (struct udp_hdr *)pkt_start;
    udp_len = (m->pkt_len - (sizeof(h2s_l2vlan_encap_hdr) +
               sizeof(h2s_ip_encap_hdr)));
    udph->dgram_len = rte_cpu_to_be_16(udp_len); 
    pkt_start += sizeof(h2s_udp_encap_hdr);

    memcpy(pkt_start, h2s_mpls_encap_hdrs,
           sizeof(h2s_mpls_encap_hdrs));
    mplsh = (struct mpls_hdr *)pkt_start;
    mplsh->tag_msb = rte_cpu_to_be_16((h2s_sess_rewrite->mpls1_label >> 4) & 0xffff);
    mplsh->tag_lsb = (h2s_sess_rewrite->mpls1_label & 0xf);
    mplsh += 1;
    mplsh->tag_msb = rte_cpu_to_be_16((h2s_sess_rewrite->mpls2_label >> 4) & 0xffff);
    mplsh->tag_lsb = (h2s_sess_rewrite->mpls2_label & 0xf);

    return;
}

static void
fte_flow_s2h_rewrite (struct rte_mbuf *m, uint16_t ip_offset, uint16_t vnic_id)
{
    uint16_t mbuf_adj_len;
    uint8_t *pkt_start;
    struct ether_hdr *etherh;
    struct vlan_hdr *vlanh;
    struct ipv4_hdr *ipv4h;
    fte_s2h_sess_rewrite_t *s2h_sess_rewrite;

    s2h_sess_rewrite = &(g_sess_rewrite[vnic_id].s2h_sess_rewrite);

    mbuf_adj_len = (ip_offset - sizeof(s2h_l2vlan_encap_hdr));
    pkt_start = (uint8_t *)rte_pktmbuf_adj(m, mbuf_adj_len);
    etherh = (struct ether_hdr *)pkt_start;
    vlanh = (struct vlan_hdr *)(etherh + 1);

    memcpy(pkt_start, s2h_l2vlan_encap_hdr,
           sizeof(s2h_l2vlan_encap_hdr));
    memcpy(etherh->d_addr.addr_bytes, (uint8_t *)&(s2h_sess_rewrite->ep_dmac), ETHER_ADDR_LEN);
    memcpy(etherh->s_addr.addr_bytes, (uint8_t *)&(s2h_sess_rewrite->ep_smac), ETHER_ADDR_LEN);
    vlanh->vlan_tci = (rte_be_to_cpu_16(vlanh->vlan_tci) & 0xf000);
    vlanh->vlan_tci |= (s2h_sess_rewrite->vnic_vlan & 0x0fff);
    vlanh->vlan_tci = rte_cpu_to_be_16(vlanh->vlan_tci);

    ipv4h = (struct ipv4_hdr *)(vlanh + 1); 
    if ((ipv4h->version_ihl >> 4) != 4) {
        vlanh->eth_proto = rte_cpu_to_be_16(ETHER_TYPE_IPv6);
    }

    return;
}

static void
fte_flow_pkt_rewrite (struct rte_mbuf *m, uint8_t dir,
                      uint16_t ip_offset, uint16_t vnic_id)
{
    if (dir == HOST_TO_SWITCH) {
        fte_flow_h2s_rewrite_mplsoudp(m, ip_offset, vnic_id);
    } else {
        fte_flow_s2h_rewrite(m, ip_offset, vnic_id);
    }

    return;
}

static sdk_ret_t
fte_session_info_create (uint8_t dir, uint32_t session_index, uint16_t vnic_id)
{
    pds_flow_session_spec_t spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_info_id = session_index;
    spec.key.direction = dir;

    if (dir == HOST_TO_SWITCH) {
        spec.data.host_to_switch_flow_info.rewrite_id = 
            g_sess_rewrite[vnic_id].h2s_sess_rewrite.sess_rewrite_id; 
    } else {
        spec.data.switch_to_host_flow_info.rewrite_id =
            g_sess_rewrite[vnic_id].s2h_sess_rewrite.sess_rewrite_id; 
    }

    return pds_flow_session_info_create(&spec);
}

sdk_ret_t
fte_flow_create(uint16_t vnic_id, ipv4_addr_t v4_addr_sip, ipv4_addr_t v4_addr_dip,
        uint8_t proto, uint16_t sport, uint16_t dport,
        pds_flow_spec_index_type_t index_type, uint32_t index)
{
    pds_flow_spec_t             spec;


    spec.key.vnic_id = vnic_id;
    spec.key.key_type = KEY_TYPE_IPV4;
    memset(spec.key.ip_saddr, 0, sizeof(spec.key.ip_saddr));
    memcpy(spec.key.ip_saddr, &v4_addr_sip, sizeof(ipv4_addr_t));
    memset(spec.key.ip_daddr, 0, sizeof(spec.key.ip_daddr));
    memcpy(spec.key.ip_daddr, &v4_addr_dip, sizeof(ipv4_addr_t));
    spec.key.ip_proto = proto;
    spec.key.l4.tcp_udp.sport = sport;
    spec.key.l4.tcp_udp.dport = dport;

    spec.data.index_type = index_type;
    spec.data.index = index;

    return pds_flow_cache_entry_create(&spec);
}

sdk_ret_t
fte_flow_create_icmp(uint16_t vnic_id,
        ipv4_addr_t v4_addr_sip, ipv4_addr_t v4_addr_dip,
        uint8_t proto, uint8_t type, uint8_t code, uint16_t identifier,
        pds_flow_spec_index_type_t index_type, uint32_t index)
{
    pds_flow_spec_t             spec;


    spec.key.vnic_id = vnic_id;
    spec.key.key_type = KEY_TYPE_IPV4;
    memset(spec.key.ip_saddr, 0, sizeof(spec.key.ip_saddr));
    memcpy(spec.key.ip_saddr, &v4_addr_sip, sizeof(ipv4_addr_t));
    memset(spec.key.ip_daddr, 0, sizeof(spec.key.ip_daddr));
    memcpy(spec.key.ip_daddr, &v4_addr_dip, sizeof(ipv4_addr_t));
    spec.key.ip_proto = proto;
    spec.key.l4.icmp.type = type;
    spec.key.l4.icmp.code = code;
    spec.key.l4.icmp.identifier = identifier;

    spec.data.index_type = index_type;
    spec.data.index = index;

    return pds_flow_cache_entry_create(&spec);
}

sdk_ret_t
fte_flow_create_v6(uint16_t vnic_id, ipv6_addr_t *v6_addr_sip,
        ipv6_addr_t *v6_addr_dip,
        uint8_t proto, uint16_t sport, uint16_t dport,
        pds_flow_spec_index_type_t index_type, uint32_t index)
{
    pds_flow_spec_t             spec;


    spec.key.vnic_id = vnic_id;
    spec.key.key_type = KEY_TYPE_IPV6;
    sdk::lib::memrev(spec.key.ip_saddr, (uint8_t *)v6_addr_sip,
            sizeof(ipv6_addr_t));
    sdk::lib::memrev(spec.key.ip_daddr, (uint8_t*)v6_addr_dip,
            sizeof(ipv6_addr_t));
    spec.key.ip_proto = proto;
    spec.key.l4.tcp_udp.sport = sport;
    spec.key.l4.tcp_udp.dport = dport;

    spec.data.index_type = index_type;
    spec.data.index = index;

    return pds_flow_cache_entry_create(&spec);
}

sdk_ret_t
fte_flow_create_v6_icmp(uint16_t vnic_id, ipv6_addr_t *v6_addr_sip,
        ipv6_addr_t *v6_addr_dip,
        uint8_t proto, uint8_t type, uint8_t code, uint16_t identifier,
        pds_flow_spec_index_type_t index_type, uint32_t index)
{
    pds_flow_spec_t             spec;


    spec.key.vnic_id = vnic_id;
    spec.key.key_type = KEY_TYPE_IPV6;
    sdk::lib::memrev(spec.key.ip_saddr, (uint8_t *)v6_addr_sip,
            sizeof(ipv6_addr_t));
    sdk::lib::memrev(spec.key.ip_daddr, (uint8_t*)v6_addr_dip,
            sizeof(ipv6_addr_t));
    spec.key.ip_proto = proto;
    spec.key.l4.icmp.type = type;
    spec.key.l4.icmp.code = code;
    spec.key.l4.icmp.identifier = identifier;

    spec.data.index_type = index_type;
    spec.data.index = index;

    return pds_flow_cache_entry_create(&spec);
    spec.key.l4.icmp.type = type;
    spec.key.l4.icmp.code = code;
    spec.key.l4.icmp.identifier = identifier;
}

sdk_ret_t
fte_session_info_create_all(uint32_t session_id, uint32_t conntrack_id,
                uint8_t skip_flow_log, mac_addr_t *host_mac,
                uint16_t h2s_epoch_vnic, uint32_t h2s_epoch_vnic_id,
                uint16_t h2s_epoch_mapping, uint32_t h2s_epoch_mapping_id,
                uint16_t h2s_policer_bw1_id, uint16_t h2s_policer_bw2_id,
                uint16_t h2s_vnic_stats_id, uint8_t *h2s_vnic_stats_mask,
                uint16_t h2s_vnic_histogram_latency_id, uint16_t h2s_vnic_histogram_packet_len_id,
                uint8_t h2s_tcp_flags_bitmap,
                uint32_t h2s_session_rewrite_id,
                uint16_t h2s_allowed_flow_state_bitmask,
                pds_egress_action_t h2s_egress_action,

                uint16_t s2h_epoch_vnic, uint32_t s2h_epoch_vnic_id,
                uint16_t s2h_epoch_mapping, uint32_t s2h_epoch_mapping_id,
                uint16_t s2h_policer_bw1_id, uint16_t s2h_policer_bw2_id,
                uint16_t s2h_vnic_stats_id, uint8_t *s2h_vnic_stats_mask,
                uint16_t s2h_vnic_histogram_latency_id, uint16_t s2h_vnic_histogram_packet_len_id,
                uint8_t s2h_tcp_flags_bitmap,
                uint32_t s2h_session_rewrite_id,
                uint16_t s2h_allowed_flow_state_bitmask,
                pds_egress_action_t s2h_egress_action)
{
    sdk_ret_t                               ret = SDK_RET_OK;
    pds_flow_session_spec_t                 spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_info_id = session_id;
    spec.key.direction = (SWITCH_TO_HOST | HOST_TO_SWITCH);

    spec.data.conntrack_id = conntrack_id;
    spec.data.skip_flow_log = skip_flow_log;
    sdk::lib::memrev(spec.data.host_mac, (uint8_t*)host_mac, sizeof(mac_addr_t));

    /* Host-to-switch */
    spec.data.host_to_switch_flow_info.epoch_vnic = h2s_epoch_vnic;
    spec.data.host_to_switch_flow_info.epoch_vnic_id = h2s_epoch_vnic_id;
    spec.data.host_to_switch_flow_info.epoch_mapping = h2s_epoch_mapping;
    spec.data.host_to_switch_flow_info.policer_bw1_id = h2s_policer_bw1_id;
    spec.data.host_to_switch_flow_info.policer_bw2_id = h2s_policer_bw2_id;
    spec.data.host_to_switch_flow_info.vnic_stats_id = h2s_vnic_stats_id;
    sdk::lib::memrev(spec.data.host_to_switch_flow_info.vnic_stats_mask,
            h2s_vnic_stats_mask, PDS_FLOW_STATS_MASK_LEN);
    spec.data.host_to_switch_flow_info.vnic_histogram_latency_id = h2s_vnic_histogram_latency_id;
    spec.data.host_to_switch_flow_info.vnic_histogram_packet_len_id = h2s_vnic_histogram_packet_len_id;
    spec.data.host_to_switch_flow_info.tcp_flags_bitmap = h2s_tcp_flags_bitmap;
    spec.data.host_to_switch_flow_info.rewrite_id = h2s_session_rewrite_id;
    spec.data.host_to_switch_flow_info.allowed_flow_state_bitmask = h2s_allowed_flow_state_bitmask;
    spec.data.host_to_switch_flow_info.egress_action = h2s_egress_action;

    /* Switch-to-host */
    spec.data.switch_to_host_flow_info.epoch_vnic = s2h_epoch_vnic;
    spec.data.switch_to_host_flow_info.epoch_vnic_id = s2h_epoch_vnic_id;
    spec.data.switch_to_host_flow_info.epoch_mapping = s2h_epoch_mapping;
    spec.data.switch_to_host_flow_info.policer_bw1_id = s2h_policer_bw1_id;
    spec.data.switch_to_host_flow_info.policer_bw2_id = s2h_policer_bw2_id;
    spec.data.switch_to_host_flow_info.vnic_stats_id = s2h_vnic_stats_id;
    sdk::lib::memrev(spec.data.switch_to_host_flow_info.vnic_stats_mask,
            s2h_vnic_stats_mask, PDS_FLOW_STATS_MASK_LEN);
    spec.data.switch_to_host_flow_info.vnic_histogram_latency_id = s2h_vnic_histogram_latency_id;
    spec.data.switch_to_host_flow_info.vnic_histogram_packet_len_id = s2h_vnic_histogram_packet_len_id;
    spec.data.switch_to_host_flow_info.tcp_flags_bitmap = s2h_tcp_flags_bitmap;
    spec.data.switch_to_host_flow_info.rewrite_id = s2h_session_rewrite_id;
    spec.data.switch_to_host_flow_info.allowed_flow_state_bitmask = s2h_allowed_flow_state_bitmask;
    spec.data.switch_to_host_flow_info.egress_action = s2h_egress_action;

    ret = pds_flow_session_info_create(&spec);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program session s2h info : %u\n", ret);
    }
    return ret;
}

sdk_ret_t
fte_flow_prog (struct rte_mbuf *m)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_flow_spec_t flow_spec;
    uint8_t flow_dir;
    uint16_t ip_offset;
    uint16_t vnic_id = 0;

    memset(&flow_spec, 0, sizeof(pds_flow_spec_t));
    flow_spec.key.vnic_id = g_h2s_vnic_id;
    ret = fte_flow_extract_prog_args(m, &flow_spec, &flow_dir,
                                     &ip_offset, &vnic_id);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_flow_extract_prog_args failed. \n");
        return ret;
    }
    flow_spec.data.index_type = PDS_FLOW_SPEC_INDEX_SESSION;
    flow_spec.data.index = g_session_index;

    // PKT Rewrite
    fte_flow_pkt_rewrite(m, flow_dir, ip_offset, vnic_id);

    ret = fte_session_info_create(flow_dir, g_session_index, vnic_id);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_session_info_create failed. \n");
        return ret;
    }

    ret = pds_flow_cache_entry_create(&flow_spec);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("pds_flow_cache_entry_create failed. \n");
        return ret;
    }

    g_session_index++;
    fte_flow_dump();
    return ret;    
}

void
fte_thread_init (unsigned int core_id)
{
    PDS_TRACE_DEBUG("Thread init on core#:%u\n", core_id);
    pds_thread_init(core_id);
}

sdk_ret_t
fte_vlan_to_vnic_map (uint16_t vlan_id, uint16_t vnic_id)
{
    pds_vlan_to_vnic_map_spec_t spec;

    spec.key.vlan_id = vlan_id;
    spec.data.vnic_type = VNIC_TYPE_L3;
    spec.data.vnic_id = vnic_id;

    return pds_vlan_to_vnic_map_create(&spec);
}

sdk_ret_t
fte_mpls_label_to_vnic_map (uint32_t mpls_label, uint16_t vnic_id)
{
    pds_mpls_label_to_vnic_map_spec_t spec;

    spec.key.mpls_label = mpls_label;
    spec.data.vnic_type = VNIC_TYPE_L3;
    spec.data.vnic_id = vnic_id;

    return pds_mpls_label_to_vnic_map_create(&spec);
}

sdk_ret_t
fte_h2s_v4_session_rewrite_mplsoudp (uint32_t session_rewrite_id,
                                     mac_addr_t *substrate_dmac,
                                     mac_addr_t *substrate_smac,
                                     uint16_t substrate_vlan,
                                     uint32_t substrate_sip,
                                     uint32_t substrate_dip,
                                     uint32_t mpls1_label,
                                     uint32_t mpls2_label)
{
    pds_flow_session_rewrite_spec_t spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = session_rewrite_id;

    spec.data.strip_l2_header = TRUE;
    spec.data.strip_vlan_tag = TRUE;

    spec.data.nat_info.nat_type = REWRITE_NAT_TYPE_NONE;

    spec.data.encap_type = ENCAP_TYPE_MPLSOUDP;
    sdk::lib::memrev(spec.data.u.mplsoudp_encap.l2_encap.dmac,
                     (uint8_t*)substrate_dmac, sizeof(mac_addr_t));
    sdk::lib::memrev(spec.data.u.mplsoudp_encap.l2_encap.smac,
                     (uint8_t*)substrate_smac, sizeof(mac_addr_t));
    spec.data.u.mplsoudp_encap.l2_encap.insert_vlan_tag = TRUE;
    spec.data.u.mplsoudp_encap.l2_encap.vlan_id = substrate_vlan;

    spec.data.u.mplsoudp_encap.ip_encap.ip_saddr = substrate_sip;
    spec.data.u.mplsoudp_encap.ip_encap.ip_daddr = substrate_dip;

    spec.data.u.mplsoudp_encap.mpls1_label = mpls1_label;
    spec.data.u.mplsoudp_encap.mpls2_label = mpls2_label;

    return pds_flow_session_rewrite_create(&spec);
}

sdk_ret_t
fte_s2h_v4_session_rewrite (uint32_t session_rewrite_id,
                            mac_addr_t *ep_dmac, mac_addr_t *ep_smac,
                            uint16_t vnic_vlan)
{
    pds_flow_session_rewrite_spec_t spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = session_rewrite_id;

    spec.data.strip_encap_header = TRUE;
    spec.data.strip_l2_header = TRUE;
    spec.data.strip_vlan_tag = TRUE;

    spec.data.nat_info.nat_type = REWRITE_NAT_TYPE_NONE;

    spec.data.encap_type = ENCAP_TYPE_L2;
    sdk::lib::memrev(spec.data.u.l2_encap.dmac, (uint8_t*)ep_dmac,
                     sizeof(mac_addr_t));
    sdk::lib::memrev(spec.data.u.l2_encap.smac, (uint8_t*)ep_smac,
                     sizeof(mac_addr_t));
    spec.data.u.l2_encap.insert_vlan_tag = TRUE;
    spec.data.u.l2_encap.vlan_id = vnic_vlan;

    return pds_flow_session_rewrite_create(&spec);
}

static sdk_ret_t
fte_setup_flow (void)
{
    sdk_ret_t       ret = SDK_RET_OK;
    fte_h2s_sess_rewrite_t *h2s_sess_rewrite;
    fte_s2h_sess_rewrite_t *s2h_sess_rewrite;
     int i;

    for (i = 1; i <= MAX_VNIC_ID; i++) {
    // Setup VNIC Mappings
    ret = fte_vlan_to_vnic_map(g_vlan_to_vnic[i], i);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_vlan_to_vnic_map failed.\n");
        return ret;
    }

    // Setup VNIC Mappings
    ret = fte_mpls_label_to_vnic_map(g_mpls_label_to_vnic[i], i);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_mpls_label_to_vnic_map failed.\n");
        return ret;
    }

    h2s_sess_rewrite = &(g_sess_rewrite[i].h2s_sess_rewrite);
    ret = fte_h2s_v4_session_rewrite_mplsoudp(h2s_sess_rewrite->sess_rewrite_id,
                                              &h2s_sess_rewrite->substrate_dmac,
                                              &h2s_sess_rewrite->substrate_smac,
                                              h2s_sess_rewrite->substrate_vlan,
                                              h2s_sess_rewrite->substrate_sip,
                                              h2s_sess_rewrite->substrate_dip,
                                              h2s_sess_rewrite->mpls1_label,
                                              h2s_sess_rewrite->mpls2_label);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_h2s_v4_session_rewrite_mplsoudp "
                        "failed.\n");
        return ret;
    }

    s2h_sess_rewrite = &(g_sess_rewrite[i].s2h_sess_rewrite);
    ret = fte_s2h_v4_session_rewrite(s2h_sess_rewrite->sess_rewrite_id,
                                     (mac_addr_t *)&s2h_sess_rewrite->ep_dmac,
                                     (mac_addr_t *)&s2h_sess_rewrite->ep_smac,
                                     s2h_sess_rewrite->vnic_vlan);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_s2h_v4_session_rewrite failed.\n");
        return ret;
    }
    }

    ret = fte_setup_static_flows();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_setup_static_flows failed.\n");
        return ret;
    }

    return ret;
}

sdk_ret_t
fte_flows_init ()
{
    sdk_ret_t sdk_ret = SDK_RET_OK;

    if (!skip_fte_flow_prog()) {
        if ((sdk_ret = fte_setup_flow()) != SDK_RET_OK) {
            PDS_TRACE_DEBUG("fte_setup_flow failed.\n");
            return sdk_ret;
        } else {
            PDS_TRACE_DEBUG("fte_setup_flow success.\n");
        }
    }

    return sdk_ret;
}

} // namespace fte
