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
#include <rte_mpls.h>

#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/sdk/lib/table/ftl/ftl_base.hpp"
#include "nic/apollo/fte/fte.hpp"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"
#include "nic/apollo/api/include/athena/pds_flow_session.h"
#include "gen/p4gen/p4/include/ftl.h"
#include "nic/apollo/api/impl/athena/ftl_dev_impl.hpp"

namespace fte {

#define IP_PROTOCOL_TCP 0x06
#define IP_PROTOCOL_UDP 0x11

uint32_t g_session_id;

// Host to Switch: Session into rewrite
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

// Flow dump vars
pds_flow_key_t dump_flow_key;

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

static sdk_ret_t
fte_flow_extract_prog_args_x1 (struct rte_mbuf *m, pds_flow_spec_t *spec)
{
#if 0 /* Rewrite needed to support new key format */
    struct ether_hdr *eth0;
    struct ipv4_hdr *ip40;
    struct tcp_hdr *tcp0;
    uint16_t ip0_offset = 0;
    pds_flow_key_host_to_switch_t *key_host2sw = NULL;
    pds_flow_key_switch_to_host_t *key_sw2host = NULL;

    // mbuf data starts at eth header
    eth0 = rte_pktmbuf_mtod(m, struct ether_hdr *);
    ip0_offset += sizeof(struct ether_hdr);

    if ((rte_be_to_cpu_16(eth0->ether_type) == ETHER_TYPE_VLAN)) {
        struct vlan_hdr *vh = (struct vlan_hdr *)(eth0 + 1);

        if ((rte_be_to_cpu_16(vh->eth_proto) != ETHER_TYPE_IPv4) &&
            (rte_be_to_cpu_16(vh->eth_proto) != ETHER_TYPE_IPv6)) {
            PDS_TRACE_DEBUG("Unsupported VLAN eth_proto:0x%x \n",
                            rte_be_to_cpu_16(vh->eth_proto));
            return SDK_RET_INVALID_OP;
        }

        spec->key.flow_dir = PDS_FLOW_DIR_HOST_TO_SWITCH;
        key_host2sw = &(spec->key.u.host_to_switch);
        key_host2sw->vlan_id = (rte_be_to_cpu_16(vh->vlan_tci) & 0xfff);
        PDS_TRACE_DEBUG("DBG: VLAN PKT. ID:0x%x\n", key_host2sw->vlan_id);
        ip0_offset += (sizeof(struct vlan_hdr));
    } else if ((rte_be_to_cpu_16(eth0->ether_type) == ETHER_TYPE_IPv4)) {
        struct ipv4_hdr *ip4;
        struct udp_hdr *udp0;
        struct mpls_hdr *mpls0;

        ip4 = rte_pktmbuf_mtod_offset(m, struct ipv4_hdr *, ip0_offset);
        if (ip4->next_proto_id != IP_PROTOCOL_UDP) {
            PDS_TRACE_DEBUG("NOT MPLSoUDP. Unsupported packet.\n");
            return SDK_RET_INVALID_OP;
        }
        ip0_offset += (sizeof(struct ipv4_hdr)); 

        udp0 = (struct udp_hdr *)(ip4 + 1);
        if (rte_be_to_cpu_16(udp0->dst_port) != 0x19EB) {
            PDS_TRACE_DEBUG("NOT MPLSoUDP. Unsupported packet.\n");
            return SDK_RET_INVALID_OP;
        }
        ip0_offset += (sizeof(struct udp_hdr));

        mpls0 = (struct mpls_hdr *)(udp0 + 1);
        
        spec->key.flow_dir = PDS_FLOW_DIR_SWITCH_TO_HOST;
        key_sw2host = &(spec->key.u.switch_to_host);
        key_sw2host->mpls1_label = rte_be_to_cpu_16(mpls0->tag_msb);
        key_sw2host->mpls1_label = ((key_sw2host->mpls1_label << 4) |
                                    mpls0->tag_lsb);
        ip0_offset += (sizeof(struct mpls_hdr));

        if (mpls0->bs == 0) {
            struct mpls_hdr *mpls1;

            mpls1 = (struct mpls_hdr *) (mpls0 + 1);
            key_sw2host->mpls2_label = rte_be_to_cpu_16(mpls1->tag_msb);
            key_sw2host->mpls2_label = ((key_sw2host->mpls2_label << 4) |
                                        mpls1->tag_lsb);
            ip0_offset += (sizeof(struct mpls_hdr));

            if (mpls1->bs == 0) {
                PDS_TRACE_DEBUG("Num. of MPLS lables > 2. NOT Supported.");
                return SDK_RET_INVALID_OP;
            }
        }
    } else {
        PDS_TRACE_DEBUG("Unsupported ether_type:0x%x \n",
                        rte_be_to_cpu_16(eth0->ether_type));
        return SDK_RET_INVALID_OP;
    }

    ip40 = rte_pktmbuf_mtod_offset(m, struct ipv4_hdr *, ip0_offset);
    if ((ip40->version_ihl >> 4) == 4) {
        uint32_t src_ip, dst_ip;
        uint8_t protocol;
        uint16_t sport = 0, dport = 0;
        uint8_t tcp_flags = 0;

        PDS_TRACE_DEBUG("DBG: IPv4 PKT. \n");
        src_ip = rte_be_to_cpu_32(ip40->src_addr);
        dst_ip = rte_be_to_cpu_32(ip40->dst_addr);
        protocol = ip40->next_proto_id;
        if ((protocol == IP_PROTOCOL_TCP) || (protocol == IP_PROTOCOL_UDP)) {
            tcp0 = (struct tcp_hdr *) (((uint8_t *) ip40) +
                    ((ip40->version_ihl & IPV4_HDR_IHL_MASK) *
                    IPV4_IHL_MULTIPLIER));
            sport = rte_be_to_cpu_16(tcp0->src_port);
            dport = rte_be_to_cpu_16(tcp0->dst_port);
            if (protocol == IP_PROTOCOL_TCP) {
                tcp_flags = tcp0->tcp_flags;
            }
        }

        if (key_host2sw) {
            key_host2sw->ip_addr_family = IP_AF_IPV4;
            memcpy(key_host2sw->ip_saddr, &src_ip, sizeof(uint32_t));
            memcpy(key_host2sw->ip_daddr, &dst_ip, sizeof(uint32_t));
            key_host2sw->ip_proto = protocol;
            key_host2sw->l4_sport = sport;
            key_host2sw->l4_dport = dport;
            key_host2sw->tcp_flags = tcp_flags;
        } else {
            key_sw2host->ip_addr_family = IP_AF_IPV4;
            memcpy(key_sw2host->ip_saddr, &src_ip, sizeof(uint32_t));
            memcpy(key_sw2host->ip_daddr, &dst_ip, sizeof(uint32_t));
            key_sw2host->ip_proto = protocol;
            key_sw2host->l4_sport = sport;
            key_sw2host->l4_dport = dport;
            key_sw2host->tcp_flags = tcp_flags;
        }

        // TODO: To be reomved. Debug purpose
        memcpy(&dump_flow_key, &(spec->key), sizeof(pds_flow_key_t));
    } else {
        PDS_TRACE_DEBUG("IPv6 support is yet to be added.\n");
        return SDK_RET_INVALID_OP;
    }
#endif
    return SDK_RET_OK;
}

// TODO: MPLSOUDP is done. Using hardcoded vaules for now.
static void
fte_session_prog (pds_flow_session_spec_t *spec)
{
    pds_flow_session_rewrite_info_t *rewrite_info;
    pds_flow_session_l2_encap_t *l2_encap;
    pds_flow_session_ip_encap_t *ip_encap;
    pds_flow_session_udp_encap_t *udp_encap;

    // TODO: pds_flow_session_data_t fields except rewrite_info is not set
    rewrite_info = &(spec->data.rewrite_info);

    rewrite_info->user_packet_rewrite_type = REWRITE_TYPE_NONE;
    rewrite_info->encap_type = ENCAP_TYPE_MPLSOUDP;

    l2_encap = &(rewrite_info->u.mplsoudp_encap.l2_encap);
    ip_encap = &(rewrite_info->u.mplsoudp_encap.ip_encap);
    udp_encap = &(rewrite_info->u.mplsoudp_encap.udp_encap);

    memcpy(l2_encap->smac, substrate_smac, sizeof(mac_addr_t));
    memcpy(l2_encap->dmac, substrate_dmac, sizeof(mac_addr_t));
    l2_encap->vlan_id = substrate_vlan;

    ip_encap->ip_ttl = substrate_ip_ttl;
    ip_encap->ip_saddr = substrate_sip;
    ip_encap->ip_daddr = substrate_dip;

    udp_encap->udp_sport = substrate_udp_sport;
    udp_encap->udp_dport = substrate_udp_dport;

    rewrite_info->u.mplsoudp_encap.mpls1_label = mpls1_label;
    rewrite_info->u.mplsoudp_encap.mpls2_label = mpls2_label;

    return;
}

void
fte_flow_prog (struct rte_mbuf *m)
{
    pds_flow_spec_t flow_spec;
    pds_flow_session_spec_t sess_spec;

    memset(&flow_spec, 0, sizeof(pds_flow_spec_t));
    if (fte_flow_extract_prog_args_x1(m, &flow_spec) != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_flow_extract_prog_args_x1 failed. \n");
        return;
    }
    // TODO: Use sdk indexer or rte_indexer for session id mgmt
    flow_spec.data.session_info_id = ++g_session_id;

    memset(&sess_spec, 0, sizeof(pds_flow_session_spec_t));
    sess_spec.key.session_info_id = g_session_id;
    fte_session_prog(&sess_spec);
    if (pds_flow_session_info_create(&sess_spec) != SDK_RET_OK) {
        PDS_TRACE_DEBUG("pds_flow_session_info_create failed. \n");
        return;
    }

    if (pds_flow_cache_entry_create(&flow_spec) != SDK_RET_OK) {
        PDS_TRACE_DEBUG("pds_flow_cache_entry_create failed. \n");
        return;
    }

    fte_flow_dump();
    return;    
}

void
fte_ftl_init (unsigned int lcore_id)
{
    if (pds_flow_cache_create(lcore_id) != SDK_RET_OK) {
        PDS_TRACE_DEBUG("pds_flow_cache_create failed. core#:%d \n", lcore_id);
        return;
    }
    PDS_TRACE_DEBUG("pds_flow_cache_create success. core#:%d \n", lcore_id);

    if (ftl_dev_impl::init(lcore_id) != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_ftl_dev::init failed. core#:%d \n", lcore_id);
        return;
    }

    return;     
}

} // namespace fte
