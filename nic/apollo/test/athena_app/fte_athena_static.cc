#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/sdk/lib/table/ftl/ftl_base.hpp"
#include "nic/apollo/api/include/athena/pds_vnic.h"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
#include "nic/apollo/api/include/athena/pds_flow_session_rewrite.h"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"
#include "gen/p4gen/p4/include/ftl.h"
#include "athena_test.hpp"

namespace fte_ath {

/*
 * VNIC 1
 */
static uint16_t  g_vnic1_vlan = 0x003A;
static uint16_t g_vnic1_id = 0x01;
static uint32_t g_vnic1_slot_id = 0x12345;

/* Towards Host */
static mac_addr_t g_vnic1_ep_smac = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
static mac_addr_t g_vnic1_ep_dmac = {0x00, 0xae, 0xcd, 0x01, 0x2c, 0x60};

/* Towards Switch */
static mac_addr_t substrate_smac = {0x00, 0x55, 0x44, 0x33, 0x22, 0x11};
static mac_addr_t substrate_dmac = {0x00, 0xae, 0xcd, 0x01, 0x2c, 0x61};
static uint16_t substrate_vlan = 0x0058;
static uint32_t substrate_sip = 0x01010101;
static uint32_t substrate_dip = 0x01010102;

/*
 * VNIC 2
 */
static uint16_t g_vnic2_vlan = 0x0058;
static uint16_t g_vnic2_id = 0x02;
static uint32_t g_vnic2_slot_id = 0x54321;

/* Towards Host */
static mac_addr_t g_vnic2_ep_smac = {0x00, 0x66, 0x77, 0x88, 0x99, 0xaa};
static mac_addr_t g_vnic2_ep_dmac = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

/* Towards Switch - Use reversed rewrites from VNIC1 */


/*
 * Static Normalized key for UDP flow
 */
static uint32_t    g_h2s_sip = 0x02000001;
static uint32_t    g_h2s_dip = 0x02000002;
static uint8_t     g_h2s_proto = 0x11;
static uint16_t    g_h2s_sport = 0x2001;
static uint16_t    g_h2s_dport = 0x2002;

static uint8_t      g_h2s_icmpv6_type = 0x80;
static uint8_t      g_h2s_icmpv6_code = 0x0;
static uint16_t     g_h2s_icmpv6_identifier = 0x1234;

static ipv6_addr_t  g_h2s_sipv6 = 
                                {
                                    0x02, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x01,
                                };
static ipv6_addr_t  g_h2s_dipv6 = 
                                {
                                    0x02, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x02,
                                };
static uint8_t  g_h2s_proto_tcp = 0x06;
//static uint8_t  g_h2s_proto_icmp = 0x01;
static uint8_t  g_h2s_proto_icmpv6 = 0x3A;
    

sdk_ret_t
fte_setup_static_flows (void)
{
    sdk_ret_t       ret = SDK_RET_OK;
    mac_addr_t      host_mac;
    uint8_t         vnic_stats_mask[PDS_FLOW_STATS_MASK_LEN];
    uint32_t        vnic1_s2h_session_rewrite_id;
    uint32_t        vnic1_h2s_session_rewrite_id;
    uint32_t        vnic2_s2h_session_rewrite_id;
    uint32_t        vnic2_h2s_session_rewrite_id;
    uint32_t        vnic1_session_index;
    uint32_t        vnic2_session_index;

    /* 
     * VNIC1
     */
    // Setup VLAN->VNIC mapping
    ret = fte_vlan_to_vnic_map(g_vnic1_vlan, g_vnic1_id);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_vlan_to_vnic_map failed.\n");
        return ret;
    }

    // Setup MPLS label->VNIC mapping
    ret = fte_mpls_label_to_vnic_map(g_vnic1_slot_id,
                                     g_vnic1_id);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_mpls_label_to_vnic_map failed.\n");
        return ret;
    }

    vnic1_h2s_session_rewrite_id = g_session_rewrite_index++;
    ret = fte_h2s_v4_session_rewrite_mplsoudp(vnic1_h2s_session_rewrite_id,
                                              &substrate_dmac,
                                              &substrate_smac,
                                              substrate_vlan,
                                              substrate_sip,
                                              substrate_dip,
                                              g_vnic2_slot_id,
                                              0);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_h2s_v4_session_rewrite_mplsoudp "
                        "failed.\n");
        return ret;
    }

    vnic1_s2h_session_rewrite_id = g_session_rewrite_index++;
    ret = fte_s2h_v4_session_rewrite(vnic1_s2h_session_rewrite_id,
                                     (mac_addr_t *)g_vnic1_ep_dmac,
                                     (mac_addr_t *)g_vnic1_ep_smac,
                                     g_vnic1_vlan);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_s2h_v4_session_rewrite failed.\n");
        return ret;
    }

    vnic1_session_index = g_session_index++;
    memset(&host_mac, 0, sizeof(host_mac));
    ret = fte_session_info_create_all(vnic1_session_index, /*conntrack_id*/0,
                /*skip_flow_log*/ FALSE, /*host_mac*/ &host_mac,

                /*h2s_epoch_vnic*/ 0, /*h2s_epoch_vnic_id*/ 0,
                /*h2s_epoch_mapping*/0, /*h2s_epoch_mapping_id*/0,
                /*h2s_policer_bw1_id*/0, /*h2s_policer_bw2_id*/0,
                /*h2s_vnic_stats_id*/0, /*h2s_vnic_stats_mask*/ vnic_stats_mask,
                /*h2s_vnic_histogram_latency_id*/0, /*h2s_vnic_histogram_packet_len_id*/0,
                /*h2s_tcp_flags_bitmap*/0,
                /*h2s_session_rewrite_id*/ vnic1_h2s_session_rewrite_id,
                /*h2s_allowed_flow_state_bitmask*/0,
                /*h2s_egress_action*/EGRESS_ACTION_NONE,

                /*s2h_epoch_vnic*/ 0, /*s2h_epoch_vnic_id*/ 0,
                /*s2h_epoch_mapping*/0, /*s2h_epoch_mapping_id*/0,
                /*s2h_policer_bw1_id*/0, /*s2h_policer_bw2_id*/0,
                /*s2h_vnic_stats_id*/0, /*s2h_vnic_stats_mask*/ vnic_stats_mask,
                /*s2h_vnic_histogram_latency_id*/0, /*s2h_vnic_histogram_packet_len_id*/0,
                /*s2h_tcp_flags_bitmap*/0,
                /*s2h_session_rewrite_id*/ vnic1_s2h_session_rewrite_id,
                /*s2h_allowed_flow_state_bitmask*/0,
                /*s2h_egress_action*/EGRESS_ACTION_NONE
                );
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // Setup Normalized Flow entry
    ret = fte_flow_create(g_vnic1_id, g_h2s_sip, g_h2s_dip,
            g_h2s_proto, g_h2s_sport, g_h2s_dport,
            PDS_FLOW_SPEC_INDEX_SESSION, vnic1_session_index);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_flow_create failed.\n");
        return ret;
    }
    // Setup v6 UDP Normalized Flow entry
    ret = fte_flow_create_v6(g_vnic1_id, &g_h2s_sipv6, &g_h2s_dipv6,
            g_h2s_proto, g_h2s_sport, g_h2s_dport,
            PDS_FLOW_SPEC_INDEX_SESSION, vnic1_session_index);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_flow_create failed.\n");
        return ret;
    }

    // Setup v6 TCP Normalized Flow entry
    ret = fte_flow_create_v6(g_vnic1_id, &g_h2s_sipv6, &g_h2s_dipv6,
            g_h2s_proto_tcp, g_h2s_sport, g_h2s_dport,
            PDS_FLOW_SPEC_INDEX_SESSION, vnic1_session_index);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_flow_create failed.\n");
        return ret;
    }

    // Setup v6 ICMP Normalized Flow entry
    ret = fte_flow_create_v6_icmp(g_vnic1_id, &g_h2s_sipv6, &g_h2s_dipv6,
            g_h2s_proto_icmpv6, g_h2s_icmpv6_type, g_h2s_icmpv6_code,
            g_h2s_icmpv6_identifier,
            PDS_FLOW_SPEC_INDEX_SESSION, vnic1_session_index);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_flow_create failed.\n");
        return ret;
    }

    /* 
     * VNIC2
     */
    // Setup VLAN->VNIC mapping
    ret = fte_vlan_to_vnic_map(g_vnic2_vlan, g_vnic2_id);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_vlan_to_vnic_map failed.\n");
        return ret;
    }

    // Setup MPLS label->VNIC mapping
    ret = fte_mpls_label_to_vnic_map(g_vnic2_slot_id,
                                     g_vnic2_id);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_mpls_label_to_vnic_map failed.\n");
        return ret;
    }

    vnic2_h2s_session_rewrite_id = g_session_rewrite_index++;
    ret = fte_h2s_v4_session_rewrite_mplsoudp(vnic2_h2s_session_rewrite_id,
                                              &substrate_smac,
                                              &substrate_dmac,
                                              substrate_vlan,
                                              substrate_dip,
                                              substrate_sip,
                                              g_vnic1_slot_id,
                                              0);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_h2s_v4_session_rewrite_mplsoudp "
                        "failed.\n");
        return ret;
    }

    vnic2_s2h_session_rewrite_id = g_session_rewrite_index++;
    ret = fte_s2h_v4_session_rewrite(vnic2_s2h_session_rewrite_id,
                                     (mac_addr_t *)g_vnic2_ep_dmac,
                                     (mac_addr_t *)g_vnic2_ep_smac,
                                     g_vnic2_vlan);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_s2h_v4_session_rewrite failed.\n");
        return ret;
    }

    vnic2_session_index = g_session_index++;
    memset(&host_mac, 0, sizeof(host_mac));
    ret = fte_session_info_create_all(vnic2_session_index, /*conntrack_id*/0,
                /*skip_flow_log*/ FALSE, /*host_mac*/ &host_mac,

                /*h2s_epoch_vnic*/ 0, /*h2s_epoch_vnic_id*/ 0,
                /*h2s_epoch_mapping*/0, /*h2s_epoch_mapping_id*/0,
                /*h2s_policer_bw1_id*/0, /*h2s_policer_bw2_id*/0,
                /*h2s_vnic_stats_id*/0, /*h2s_vnic_stats_mask*/ vnic_stats_mask,
                /*h2s_vnic_histogram_latency_id*/0, /*h2s_vnic_histogram_packet_len_id*/0,
                /*h2s_tcp_flags_bitmap*/0,
                /*h2s_session_rewrite_id*/ vnic2_h2s_session_rewrite_id,
                /*h2s_allowed_flow_state_bitmask*/0,
                /*h2s_egress_action*/EGRESS_ACTION_NONE,

                /*s2h_epoch_vnic*/ 0, /*s2h_epoch_vnic_id*/ 0,
                /*s2h_epoch_mapping*/0, /*s2h_epoch_mapping_id*/0,
                /*s2h_policer_bw1_id*/0, /*s2h_policer_bw2_id*/0,
                /*s2h_vnic_stats_id*/0, /*s2h_vnic_stats_mask*/ vnic_stats_mask,
                /*s2h_vnic_histogram_latency_id*/0, /*s2h_vnic_histogram_packet_len_id*/0,
                /*s2h_tcp_flags_bitmap*/0,
                /*s2h_session_rewrite_id*/ vnic2_s2h_session_rewrite_id,
                /*s2h_allowed_flow_state_bitmask*/0,
                /*s2h_egress_action*/EGRESS_ACTION_NONE
                );
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // Setup Normalized Flow entry
    ret = fte_flow_create(g_vnic2_id, g_h2s_dip, g_h2s_sip,
            g_h2s_proto, g_h2s_dport, g_h2s_sport,
            PDS_FLOW_SPEC_INDEX_SESSION, vnic2_session_index);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_flow_create failed.\n");
        return ret;
    }
    // Setup v6 UDP Normalized Flow entry
    ret = fte_flow_create_v6(g_vnic2_id, &g_h2s_dipv6, &g_h2s_sipv6,
            g_h2s_proto, g_h2s_dport, g_h2s_sport,
            PDS_FLOW_SPEC_INDEX_SESSION, vnic2_session_index);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_flow_create failed.\n");
        return ret;
    }
    // Setup v6 TCP Normalized Flow entry
    ret = fte_flow_create_v6(g_vnic2_id, &g_h2s_dipv6, &g_h2s_sipv6,
            g_h2s_proto_tcp, g_h2s_dport, g_h2s_sport,
            PDS_FLOW_SPEC_INDEX_SESSION, vnic2_session_index);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_flow_create failed.\n");
        return ret;
    }

    // Setup v6 ICMP Normalized Flow entry
    ret = fte_flow_create_v6_icmp(g_vnic2_id, &g_h2s_dipv6, &g_h2s_sipv6,
            g_h2s_proto_icmpv6, g_h2s_icmpv6_type, g_h2s_icmpv6_code,
            g_h2s_icmpv6_identifier,
            PDS_FLOW_SPEC_INDEX_SESSION, vnic2_session_index);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_DEBUG("fte_flow_create failed.\n");
        return ret;
    }

    return ret;
}


}
