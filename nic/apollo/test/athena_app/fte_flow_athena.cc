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
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/sdk/lib/table/ftl/ftl_base.hpp"
#include "nic/sdk/lib/rte_indexer/rte_indexer.hpp"
#include "fte_athena.hpp"
#include "nic/apollo/api/include/athena/pds_init.h"
#include "nic/apollo/api/include/athena/pds_vnic.h"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
#include "nic/apollo/api/include/athena/pds_flow_session_rewrite.h"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"
#include "nic/apollo/api/include/athena/pds_dnat.h"
#include "gen/p4gen/p4/include/ftl.h"
#include "athena_test.hpp"
#include "app_test_utils.hpp"
#include "json_parser.hpp"

uint32_t num_flows_added = 0;
uint32_t attempted_flows = 0;

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

#define MAX_SESSION_INDEX 0x3D0900   // 4M Seesion IDs
static rte_indexer *g_session_indexer;
uint32_t g_session_rewrite_index = 1;

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


#if 0
static void
fte_flow_dump (void)
{
    pds_flow_info_t flow_info;

    if (pds_flow_cache_entry_read(&dump_flow_key, &flow_info) != PDS_RET_OK) {
        PDS_TRACE_DEBUG("pds_flow_cache_entry_read failed.\n");
        return;
    }

    // TODO: Dump the values
    PDS_TRACE_DEBUG("pds_flow_cache_entry_read success.\n");
    return;
}
#endif

sdk_ret_t
fte_session_indexer_init (void)
{
    g_session_indexer = rte_indexer::factory(MAX_SESSION_INDEX,
                                             true, true);
    if (g_session_indexer == NULL) {
        PDS_TRACE_DEBUG("g_session_indexer init failed.\n");
        return SDK_RET_ERR;
    }
    return SDK_RET_OK;
}

void
fte_session_indexer_destroy (void)
{
    rte_indexer::destroy(g_session_indexer);
}

static inline sdk_ret_t
fte_session_index_alloc (uint32_t *sess_id)
{
    return g_session_indexer->alloc(sess_id);
}

static inline sdk_ret_t
fte_session_index_free (uint32_t sess_id)
{
    return g_session_indexer->free(sess_id);
}

// NAT MAP TABLE APIs
static sdk_ret_t
fte_nmt_get_local_ip (uint16_t vnic_id, uint32_t nat_ip, uint32_t *local_ip)
{
    uint8_t num_nat_mappings;
    nat_map_tbl_t *nat_map_tbl;
    int i = 0;

    num_nat_mappings = g_flow_cache_policy[vnic_id].num_nat_mappings;
    nat_map_tbl = g_flow_cache_policy[vnic_id].nat_map_tbl;

    while (i < num_nat_mappings) {
        if (nat_ip == nat_map_tbl[i].nat_ip) {
            *local_ip = nat_map_tbl[i].local_ip;
            return SDK_RET_OK;
        }
        i++;
    }

    return SDK_RET_ENTRY_NOT_FOUND;
}

static sdk_ret_t
fte_nmt_get_nat_ip (uint16_t vnic_id, uint32_t local_ip, uint32_t *nat_ip)
{
    uint8_t num_nat_mappings;
    nat_map_tbl_t *nat_map_tbl;
    int i = 0;

    num_nat_mappings = g_flow_cache_policy[vnic_id].num_nat_mappings;
    nat_map_tbl = g_flow_cache_policy[vnic_id].nat_map_tbl;

    while (i < num_nat_mappings) {
        if (local_ip == nat_map_tbl[i].local_ip) {
            *nat_ip = nat_map_tbl[i].nat_ip;
            return SDK_RET_OK;
        }
        i++;
    }

    return SDK_RET_ENTRY_NOT_FOUND;
}

static sdk_ret_t
fte_nmt_get_rewrite_id (uint16_t vnic_id, uint32_t local_ip,
                        uint32_t *h2s_rewrite_id,
                        uint32_t *s2h_rewrite_id)
{
    uint8_t num_nat_mappings;
    nat_map_tbl_t *nat_map_tbl;
    int i = 0;

    num_nat_mappings = g_flow_cache_policy[vnic_id].num_nat_mappings;
    nat_map_tbl = g_flow_cache_policy[vnic_id].nat_map_tbl;

    while (i < num_nat_mappings) {
        if (local_ip == nat_map_tbl[i].local_ip) {
            *h2s_rewrite_id = nat_map_tbl[i].h2s_rewrite_id;
            *s2h_rewrite_id = nat_map_tbl[i].s2h_rewrite_id;
            return SDK_RET_OK;
        }
        i++;
    }

    return SDK_RET_ENTRY_NOT_FOUND;
}

static sdk_ret_t
fte_flow_extract_prog_args (struct rte_mbuf *m, pds_flow_spec_t *spec,
                            uint8_t *dir, uint16_t *ip_off, uint16_t *vnic_id)
{
    struct ether_hdr *eth0;
    struct ipv4_hdr *ip40;
    struct tcp_hdr *tcp0;
    struct udp_hdr *udp0;
    struct mpls_hdr *mpls_dst;
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

        mpls_dst = (struct mpls_hdr *)(udp0 + 1); /* 1st label */
        ip0_offset += (sizeof(struct mpls_hdr));

        if (mpls_dst->bs == 0) { 
            mpls_dst = (struct mpls_hdr *) (mpls_dst + 1);  /* 2nd label */
            ip0_offset += (sizeof(struct mpls_hdr));

            if (mpls_dst->bs == 0) {
                struct mpls_hdr *mpls3;

                mpls3 = (struct mpls_hdr *) (mpls_dst + 1);  /* 3rd label */
                ip0_offset += (sizeof(struct mpls_hdr));

                if (mpls3->bs == 0) {
                    PDS_TRACE_DEBUG("Unsupported: MPLS lables > 3.");
                    return SDK_RET_INVALID_OP;
                }
            }
        }
        mpls_label = ((rte_be_to_cpu_16(mpls_dst->tag_msb) << 4) |
                            mpls_dst->tag_lsb);
        *vnic_id = g_mpls_label_to_vnic[mpls_label];
        *dir = SWITCH_TO_HOST;
        *ip_off = ip0_offset;
    } else {
        *dir = HOST_TO_SWITCH;
        *ip_off = ip0_offset;
        *vnic_id = g_vlan_to_vnic[vlan_id];
    }

    if (*vnic_id == 0) {
        PDS_TRACE_DEBUG("vnic_id lookup failed.\n");
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    key->vnic_id = *vnic_id;

    ip40 = rte_pktmbuf_mtod_offset(m, struct ipv4_hdr *, ip0_offset);
    if ((ip40->version_ihl >> 4) == 4) {
        uint32_t src_ip, dst_ip;
        uint32_t dnat_local_ip;

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
            if (g_flow_cache_policy[*vnic_id].nat_enabled) {
                if (fte_nmt_get_local_ip(*vnic_id, dst_ip,
                                         &dnat_local_ip) == SDK_RET_OK) {
                    dst_ip = dnat_local_ip;
                }
            }
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
        key->l4.icmp.identifier = rte_be_to_cpu_16(icmph->icmp_ident);
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
    rewrite_underlay_info_t *rewrite_underlay;
    uint32_t nat_ip;

    // SNAT
    if (g_flow_cache_policy[vnic_id].nat_enabled) {
        ip4h = rte_pktmbuf_mtod_offset(m, struct ipv4_hdr *, ip_offset);
        if (fte_nmt_get_nat_ip(vnic_id, rte_be_to_cpu_32(ip4h->src_addr),
                               &nat_ip) == SDK_RET_OK) {
            ip4h->src_addr = rte_cpu_to_be_32(nat_ip);
        }
    }

    rewrite_underlay = &(g_flow_cache_policy[vnic_id].rewrite_underlay);
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
    memcpy(etherh->d_addr.addr_bytes, rewrite_underlay->substrate_dmac, ETHER_ADDR_LEN);
    memcpy(etherh->s_addr.addr_bytes, rewrite_underlay->substrate_smac, ETHER_ADDR_LEN);
    vlanh->vlan_tci = (rte_be_to_cpu_16(vlanh->vlan_tci) & 0xf000);
    vlanh->vlan_tci |= (rewrite_underlay->substrate_vlan & 0x0fff);
    vlanh->vlan_tci = rte_cpu_to_be_16(vlanh->vlan_tci);
    pkt_start += sizeof(h2s_l2vlan_encap_hdr);

    memcpy(pkt_start, h2s_ip_encap_hdr, sizeof(h2s_ip_encap_hdr));
    ip4h = (struct ipv4_hdr *)pkt_start;
    ip_tot_len = (m->pkt_len - sizeof(h2s_l2vlan_encap_hdr));
    ip4h->total_length = rte_cpu_to_be_16(ip_tot_len);
    ip4h->src_addr = rte_cpu_to_be_32(rewrite_underlay->substrate_sip);
    ip4h->dst_addr = rte_cpu_to_be_32(rewrite_underlay->substrate_dip);
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
    mplsh->tag_msb = rte_cpu_to_be_16((rewrite_underlay->mpls_label1 >> 4) & 0xffff);
    mplsh->tag_lsb = (rewrite_underlay->mpls_label1 & 0xf);
    mplsh += 1;
    mplsh->tag_msb = rte_cpu_to_be_16((rewrite_underlay->mpls_label2 >> 4) & 0xffff);
    mplsh->tag_lsb = (rewrite_underlay->mpls_label2 & 0xf);

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
    rewrite_host_info_t *rewrite_host;
    uint32_t local_ip;

    // DNAT
    if (g_flow_cache_policy[vnic_id].nat_enabled) {
        ipv4h = rte_pktmbuf_mtod_offset(m, struct ipv4_hdr *, ip_offset);
        if (fte_nmt_get_local_ip(vnic_id, rte_be_to_cpu_32(ipv4h->dst_addr),
                               &local_ip) == SDK_RET_OK) {
            ipv4h->dst_addr = rte_cpu_to_be_32(local_ip);
        }
    }

    rewrite_host = &(g_flow_cache_policy[vnic_id].rewrite_host);

    mbuf_adj_len = (ip_offset - sizeof(s2h_l2vlan_encap_hdr));
    pkt_start = (uint8_t *)rte_pktmbuf_adj(m, mbuf_adj_len);
    etherh = (struct ether_hdr *)pkt_start;
    vlanh = (struct vlan_hdr *)(etherh + 1);

    memcpy(pkt_start, s2h_l2vlan_encap_hdr,
           sizeof(s2h_l2vlan_encap_hdr));
    memcpy(etherh->d_addr.addr_bytes, rewrite_host->ep_dmac, ETHER_ADDR_LEN);
    memcpy(etherh->s_addr.addr_bytes, rewrite_host->ep_smac, ETHER_ADDR_LEN);
    vlanh->vlan_tci = (rte_be_to_cpu_16(vlanh->vlan_tci) & 0xf000);
    vlanh->vlan_tci |= (g_flow_cache_policy[vnic_id].vlan_id & 0x0fff);
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
fte_session_info_create (uint32_t session_index, uint16_t vnic_id,
                         uint32_t local_ip)
{
    pds_flow_session_spec_t spec;
    uint32_t h2s_rewrite_id, s2h_rewrite_id;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_info_id = session_index;
    spec.key.direction = (SWITCH_TO_HOST | HOST_TO_SWITCH);

    if (g_flow_cache_policy[vnic_id].nat_enabled) {
        if (fte_nmt_get_rewrite_id(vnic_id, local_ip,
                  &h2s_rewrite_id, &s2h_rewrite_id) == SDK_RET_OK) {
            spec.data.host_to_switch_flow_info.rewrite_id =
                            h2s_rewrite_id;
            spec.data.switch_to_host_flow_info.rewrite_id =
                            s2h_rewrite_id;

            return (sdk_ret_t)pds_flow_session_info_create(&spec);
        }
    }

    spec.data.host_to_switch_flow_info.rewrite_id =
            g_flow_cache_policy[vnic_id].rewrite_underlay.rewrite_id;
    spec.data.switch_to_host_flow_info.rewrite_id =
            g_flow_cache_policy[vnic_id].rewrite_host.rewrite_id;

    return (sdk_ret_t)pds_flow_session_info_create(&spec);
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

    return (sdk_ret_t)pds_flow_cache_entry_create(&spec);
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

    return (sdk_ret_t)pds_flow_cache_entry_create(&spec);
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

    return (sdk_ret_t)pds_flow_cache_entry_create(&spec);
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

    return (sdk_ret_t)pds_flow_cache_entry_create(&spec);
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
    pds_ret_t                               ret = PDS_RET_OK;
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
    if (ret != PDS_RET_OK) {
        PDS_TRACE_ERR("Failed to program session s2h info : %u\n", ret);
    }
    return (sdk_ret_t)ret;
}

sdk_ret_t
fte_flow_prog (struct rte_mbuf *m)
{
    sdk_ret_t ret = SDK_RET_OK;
    pds_flow_spec_t flow_spec;
    uint8_t flow_dir;
    uint16_t ip_offset;
    uint16_t vnic_id = 0;
    uint32_t session_index;

    memset(&flow_spec, 0, sizeof(pds_flow_spec_t));
    ret = fte_flow_extract_prog_args(m, &flow_spec, &flow_dir,
                                     &ip_offset, &vnic_id);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_flow_extract_prog_args failed. \n");
        return ret;
    }
    flow_spec.data.index_type = PDS_FLOW_SPEC_INDEX_SESSION;
    ret = fte_session_index_alloc(&session_index);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_session_index_alloc failed. \n");
        return ret;
    }
    flow_spec.data.index = session_index;

    // PKT Rewrite
    fte_flow_pkt_rewrite(m, flow_dir, ip_offset, vnic_id);

    ret = fte_session_info_create(session_index, vnic_id,
                          (*(uint32_t *)flow_spec.key.ip_saddr));
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_session_info_create failed. \n");
        return ret;
    }

    ret = (sdk_ret_t)pds_flow_cache_entry_create(&flow_spec);
    if ((ret != SDK_RET_OK) && (ret != SDK_RET_ENTRY_EXISTS)) {
        PDS_TRACE_DEBUG("pds_flow_cache_entry_create failed. \n");
        return ret;
    }

    return SDK_RET_OK;    
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

    return (sdk_ret_t)pds_vlan_to_vnic_map_create(&spec);
}

sdk_ret_t
fte_mpls_label_to_vnic_map (uint32_t mpls_label, uint16_t vnic_id)
{
    pds_mpls_label_to_vnic_map_spec_t spec;

    spec.key.mpls_label = mpls_label;
    spec.data.vnic_type = VNIC_TYPE_L3;
    spec.data.vnic_id = vnic_id;

    return (sdk_ret_t)pds_mpls_label_to_vnic_map_create(&spec);
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

    return (sdk_ret_t)pds_flow_session_rewrite_create(&spec);
}

sdk_ret_t
fte_h2s_nat_v4_session_rewrite_mplsoudp (uint32_t session_rewrite_id,
                                     mac_addr_t *substrate_dmac,
                                     mac_addr_t *substrate_smac,
                                     uint16_t substrate_vlan,
                                     uint32_t substrate_sip,
                                     uint32_t substrate_dip,
                                     uint32_t mpls1_label,
                                     uint32_t mpls2_label,
                                     pds_flow_session_rewrite_nat_type_t nat_type,
                                     ipv4_addr_t ipv4_addr)
{
    pds_flow_session_rewrite_spec_t spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = session_rewrite_id;

    spec.data.strip_l2_header = TRUE;
    spec.data.strip_vlan_tag = TRUE;

    spec.data.nat_info.nat_type = nat_type;
    spec.data.nat_info.u.ipv4_addr = (uint32_t) ipv4_addr;

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

    return (sdk_ret_t)pds_flow_session_rewrite_create(&spec);
}

sdk_ret_t
fte_create_dnat_map_ipv4(uint16_t vnic_id, ipv4_addr_t v4_nat_dip, 
        ipv4_addr_t v4_orig_dip, uint16_t dnat_epoch)
{
    pds_dnat_mapping_spec_t         spec;

    memset(&spec, 0, sizeof(spec));

    spec.key.vnic_id = vnic_id;
    spec.key.key_type = IP_AF_IPV4;
    memcpy(spec.key.addr, &v4_nat_dip, sizeof(ipv4_addr_t));

    spec.data.addr_type = IP_AF_IPV4;
    memcpy(spec.data.addr, &v4_orig_dip, sizeof(ipv4_addr_t));
    spec.data.epoch = dnat_epoch;

    return (sdk_ret_t)pds_dnat_map_entry_create(&spec);
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

    return (sdk_ret_t)pds_flow_session_rewrite_create(&spec);
}

sdk_ret_t
fte_s2h_nat_v4_session_rewrite (uint32_t session_rewrite_id,
                            mac_addr_t *ep_dmac, mac_addr_t *ep_smac,
                            uint16_t vnic_vlan,
                            pds_flow_session_rewrite_nat_type_t nat_type,
                            ipv4_addr_t ipv4_addr)
{
    pds_flow_session_rewrite_spec_t spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.session_rewrite_id = session_rewrite_id;

    spec.data.strip_encap_header = TRUE;
    spec.data.strip_l2_header = TRUE;
    spec.data.strip_vlan_tag = TRUE;

    spec.data.nat_info.nat_type = nat_type;
    spec.data.nat_info.u.ipv4_addr = (uint32_t) ipv4_addr;

    spec.data.encap_type = ENCAP_TYPE_L2;
    sdk::lib::memrev(spec.data.u.l2_encap.dmac, (uint8_t*)ep_dmac,
                     sizeof(mac_addr_t));
    sdk::lib::memrev(spec.data.u.l2_encap.smac, (uint8_t*)ep_smac,
                     sizeof(mac_addr_t));
    spec.data.u.l2_encap.insert_vlan_tag = TRUE;
    spec.data.u.l2_encap.vlan_id = vnic_vlan;

    return (sdk_ret_t)pds_flow_session_rewrite_create(&spec);
}

static sdk_ret_t 
fte_setup_dnat_flow (flow_cache_policy_info_t *policy)
{
    sdk_ret_t ret = SDK_RET_OK;
    rewrite_underlay_info_t *rewrite_underlay;
    rewrite_host_info_t *rewrite_host;
    nat_info_t *nat_info;
    nat_map_tbl_t *nat_map_tbl;
    uint32_t local_ip, nat_ip;
    uint8_t map_cnt = 0;

    rewrite_underlay = &(policy->rewrite_underlay);
    rewrite_host = &(policy->rewrite_host);
    nat_info = &(policy->nat_info);
    nat_map_tbl = policy->nat_map_tbl;

    local_ip = nat_info->local_ip_lo;
    nat_ip = nat_info->nat_ip_lo;

    while (local_ip <= nat_info->local_ip_hi) {
        /* DNAT mapping */
        ret = fte_create_dnat_map_ipv4(policy->vnic_id, nat_ip,
                                       local_ip, 0);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_DEBUG("fte_create_dnat_map_ipv4 failed. \n");
            return ret;
        }

        nat_map_tbl[map_cnt].local_ip = local_ip;
        nat_map_tbl[map_cnt].nat_ip = nat_ip;

        nat_map_tbl[map_cnt].h2s_rewrite_id = g_session_rewrite_index++;
        if (rewrite_underlay->encap_type == ENCAP_MPLSOUDP) {
            ret = fte_h2s_nat_v4_session_rewrite_mplsoudp(
                        nat_map_tbl[map_cnt].h2s_rewrite_id,
                        (mac_addr_t *)rewrite_underlay->substrate_dmac,
                        (mac_addr_t *)rewrite_underlay->substrate_smac,
                        rewrite_underlay->substrate_vlan,
                        rewrite_underlay->substrate_sip,
                        rewrite_underlay->substrate_dip,
                        rewrite_underlay->mpls_label1,
                        rewrite_underlay->mpls_label2,
                        REWRITE_NAT_TYPE_IPV4_SNAT,
                        (ipv4_addr_t)nat_ip);
                            
            if (ret != SDK_RET_OK) {
                PDS_TRACE_DEBUG("fte_h2s_nat_v4_session_rewrite_mplsoudp "
                                "failed.\n");
                return ret;
            }
        } else {
            PDS_TRACE_DEBUG("Unsupported encap_type:%u \n",
                            rewrite_underlay->encap_type);
            return SDK_RET_INVALID_OP;
        }

        nat_map_tbl[map_cnt].s2h_rewrite_id = g_session_rewrite_index++;
        ret = fte_s2h_nat_v4_session_rewrite(
                    nat_map_tbl[map_cnt].s2h_rewrite_id,
                    (mac_addr_t *)rewrite_host->ep_dmac,
                    (mac_addr_t *)rewrite_host->ep_smac,
                    policy->vlan_id,
                    REWRITE_NAT_TYPE_IPV4_DNAT,
                    (ipv4_addr_t)local_ip);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_DEBUG("fte_s2h_nat_v4_session_rewrite failed.\n");
            return ret;
        }

        local_ip++;
        nat_ip++;
        map_cnt++;
    }

    policy->num_nat_mappings = map_cnt;
    return SDK_RET_OK;
}

sdk_ret_t 
fte_setup_v4_flows_json (void)
{
    sdk_ret_t ret;
    uint8_t v4_flows_cnt = 0;
    v4_flows_info_t *v4_flows;
    uint16_t vnic_id;
    uint32_t sip, dip;
    uint8_t proto;
    uint16_t sport, dport;
    uint32_t session_index;

    while (v4_flows_cnt < g_num_v4_flows) {
        v4_flows = &g_v4_flows[v4_flows_cnt];
        vnic_id = v4_flows->vnic_lo;
        proto = v4_flows->proto;
        PDS_TRACE_DEBUG("v4_flows[%u] VNIC: Lo-%u Hi-%u, SIP: Lo-0x%x Hi-0x%x "
               "DIP: Lo-0x%x Hi-0x%x SPORT: Lo-%u Hi-%u, DPORT: Lo-%u Hi-%u\n PROTO: %u\n",
               v4_flows_cnt, v4_flows->vnic_lo, v4_flows->vnic_hi,
               v4_flows->sip_lo, v4_flows->sip_hi, v4_flows->dip_lo, v4_flows->dip_hi,
               v4_flows->sport_lo, v4_flows->sport_hi, v4_flows->dport_lo, v4_flows->dport_hi,
               v4_flows->proto);
        while (vnic_id <= v4_flows->vnic_hi) {
            if (g_flow_cache_policy[vnic_id].vnic_id == 0) {
                vnic_id++;
                continue;
            }
            sip = v4_flows->sip_lo;
            while (sip <= v4_flows->sip_hi) {
                dip = v4_flows->dip_lo;
                while (dip <= v4_flows->dip_hi) {
                    sport = v4_flows->sport_lo;
                    while (sport <= v4_flows->sport_hi) {
                        dport = v4_flows->dport_lo;
                        while (dport <= v4_flows->dport_hi) {
                            ret = fte_session_index_alloc(
                                    &session_index);
                            if (ret != SDK_RET_OK) {
                                PDS_TRACE_DEBUG(
                                    "fte_session_index_alloc failed.\n");
                                return ret;
                            }

                            ret = fte_session_info_create(
                                    session_index, vnic_id, sip);
                            if (ret != SDK_RET_OK) {
                                PDS_TRACE_DEBUG(
                                    "fte_session_info_create failed.\n");
                                return ret;
                            }

                            ret = fte_flow_create(vnic_id, sip, dip,
                                    proto, sport, dport,
                                    PDS_FLOW_SPEC_INDEX_SESSION,
                                    session_index);
                            attempted_flows++;
                            if (ret != SDK_RET_OK) {
                                PDS_TRACE_DEBUG("Flow Create Fail: SrcIP:0x%x DstIP:0x%x "
                                    "Dport:%u Sport:%u Proto:%u "
                                    "VNICID:%u index:%u\n",
                                    sip, dip, dport, sport, proto,
                                    vnic_id, session_index);
                                // Even on collision/flow insert fail,
                                // continue the flow creation
                                // return ret;
                                dport++;
                                continue;
                            } else {
                                //PDS_TRACE_DEBUG("Created: SrcIP:0x%x DstIP:0x%x "
                                //    "Dport:%u Sport:%u Proto:%u "
                                //    "VNICID:%u index:%u\n",
                                //    sip, dip, dport, sport, proto,
                                //    vnic_id, session_index);
                            }
                            num_flows_added++;
                            dport++;
                        }
                        sport++;
                    }
                    dip++;
                }
                sip++;
            }
            vnic_id++;
        }
        v4_flows_cnt++;
    }

    PDS_TRACE_DEBUG("fte_setup_v4_flows_json: num_flows_added:%u,"
                    " attempted_flows:%u\n", num_flows_added, attempted_flows);
    return SDK_RET_OK;
}

static sdk_ret_t
fte_setup_flow (void)
{
    sdk_ret_t ret = SDK_RET_OK;
    flow_cache_policy_info_t *policy;
    rewrite_underlay_info_t *rewrite_underlay;
    rewrite_host_info_t *rewrite_host;
    uint16_t vnic_id;
    uint16_t i;

    for (i = 0; i < g_num_policies; i++) {
        vnic_id = g_vnic_id_list[i];
        policy = &(g_flow_cache_policy[vnic_id]);
        // Setup VNIC Mappings
        ret = fte_vlan_to_vnic_map(policy->vlan_id, vnic_id);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_DEBUG("fte_vlan_to_vnic_map failed.\n");
            return ret;
        }

        // Setup VNIC Mappings
        ret = fte_mpls_label_to_vnic_map(policy->src_slot_id, vnic_id);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_DEBUG("fte_mpls_label_to_vnic_map failed.\n");
            return ret;
        }

        if (policy->nat_enabled) {
            ret = fte_setup_dnat_flow(policy);
            if (ret != SDK_RET_OK) {
                PDS_TRACE_DEBUG("fte_setup_dnat_flow failed.\n");
                return ret;
           }
        }

        rewrite_underlay = &(policy->rewrite_underlay);
        rewrite_underlay->rewrite_id = g_session_rewrite_index++;
        if (rewrite_underlay->encap_type == ENCAP_MPLSOUDP) {
            ret = fte_h2s_v4_session_rewrite_mplsoudp(
                            rewrite_underlay->rewrite_id,
                            (mac_addr_t *)rewrite_underlay->substrate_dmac,
                            (mac_addr_t *)rewrite_underlay->substrate_smac,
                            rewrite_underlay->substrate_vlan,
                            rewrite_underlay->substrate_sip,
                            rewrite_underlay->substrate_dip,
                            rewrite_underlay->mpls_label1,
                            rewrite_underlay->mpls_label2);
                            
            if (ret != SDK_RET_OK) {
                PDS_TRACE_DEBUG("fte_h2s_v4_session_rewrite_mplsoudp "
                                "failed.\n");
                return ret;
            }
        } else {
            PDS_TRACE_DEBUG("Unsupported encap_type:%u \n",
                            rewrite_underlay->encap_type);
            return SDK_RET_INVALID_OP;
        }

        rewrite_host = &(policy->rewrite_host);
        rewrite_host->rewrite_id = g_session_rewrite_index++;
        ret = fte_s2h_v4_session_rewrite(
                        rewrite_host->rewrite_id,
                        (mac_addr_t *)rewrite_host->ep_dmac,
                        (mac_addr_t *)rewrite_host->ep_smac,
                        policy->vlan_id);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_DEBUG("fte_s2h_v4_session_rewrite failed.\n");
            return ret;
        }
    }

    ret = fte_setup_v4_flows_json();
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_setup_v4_flows_json failed.\n");
        return ret;
    }

    return ret;
}

sdk_ret_t
fte_flows_init ()
{
    sdk_ret_t sdk_ret = SDK_RET_OK;

    sdk_ret = fte_session_indexer_init();
    if (sdk_ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_session_indexer_init failed.\n");
        return sdk_ret;
    }

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
