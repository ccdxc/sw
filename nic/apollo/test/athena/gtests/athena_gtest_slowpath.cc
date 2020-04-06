#include <stdint.h>
#include <vector>
#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/api/include/athena/pds_vnic.h"
#include "nic/apollo/api/include/athena/pds_flow_cache.h"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
#include "nic/apollo/api/include/athena//pds_flow_session_rewrite.h"
#include "nic/apollo/api/include/athena/pds_epoch.h"

#include "athena_gtest.hpp"


static uint16_t    g_udp_vnic_id = VNIC_ID_SLOW_PATH;
static uint32_t    g_h2s_udp_vlan = VLAN_ID_SLOW_PATH;

/*
 * Normalized key for UDP flow
 */
static uint32_t    g_h2s_sip = 0x02000001;
static uint32_t    g_h2s_dip = 0xc0000201;
static uint8_t     g_h2s_proto = 0x11;
static uint16_t    g_h2s_sport = 0x03e8;
static uint16_t    g_h2s_dport = 0x2710;

/*
 * Session info rewrite - S2H
 */
static mac_addr_t  ep_smac = {0x00, 0x00, 0xF1, 0xD0, 0xD1, 0xD0};
static mac_addr_t  ep_dmac = {0x00, 0x00, 0x00, 0x40, 0x08, 0x01};

/*
 * Session into rewrite - H2S
 */
static mac_addr_t  substrate_smac = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
static mac_addr_t  substrate_dmac = {0x00, 0x06, 0x07, 0x08, 0x09, 0x0a};
static uint16_t    substrate_vlan = 0x02;
static uint32_t    substrate_sip = 0x04030201;
static uint32_t    substrate_dip = 0x01020304;
static uint32_t    mpls1_label = 0x12345;
static uint32_t    mpls2_label = MPLS_LABEL_SLOW_PATH;


/*
 * Epoch information
 */
static uint16_t     g_epoch_val = 0x1234;



sdk_ret_t
create_epoch_entry(uint32_t epoch_id, uint16_t epoch_val)
{
    pds_epoch_spec_t            spec;

    memset(&spec, 0, sizeof(spec));
    spec.key.epoch_id = epoch_id;
    spec.data.epoch = epoch_val;

    return (sdk_ret_t)pds_epoch_create(&spec);
}


static sdk_ret_t
setup_flow_v4_udp_epoch_mismatch(void)
{
    sdk_ret_t       ret = SDK_RET_OK;
    mac_addr_t      host_mac;
    uint8_t         vnic_stats_mask[PDS_FLOW_STATS_MASK_LEN];
    uint32_t        s2h_session_rewrite_id;
    uint32_t        h2s_session_rewrite_id;
    uint32_t        h2s_epoch1_id;

    // Setup VNIC Mappings
    ret = vlan_to_vnic_map(g_h2s_udp_vlan, g_udp_vnic_id);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // Setup VNIC Mappings
    ret = mpls_label_to_vnic_map(mpls2_label, g_udp_vnic_id);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    s2h_session_rewrite_id = g_session_rewrite_index++;

    ret = create_s2h_session_rewrite(s2h_session_rewrite_id,
                    (mac_addr_t*)ep_dmac, (mac_addr_t*)ep_smac, g_h2s_udp_vlan);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    h2s_session_rewrite_id = g_session_rewrite_index++;

    ret = create_h2s_session_rewrite_mplsoudp(h2s_session_rewrite_id,
        &substrate_dmac, &substrate_smac,
        substrate_vlan,
        substrate_sip, substrate_dip,
        mpls1_label, mpls2_label);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    h2s_epoch1_id = g_epoch_index++;
    ret = create_epoch_entry(h2s_epoch1_id, g_epoch_val + 1);
    if (ret != SDK_RET_OK) {
        return ret;
    }

    memset(&host_mac, 0, sizeof(host_mac));
    ret = create_session_info_all(g_session_index, /*conntrack_id*/0,
                /*skip_flow_log*/ FALSE, /*host_mac*/ &host_mac,

                /*h2s_epoch_vnic*/ g_epoch_val, /*h2s_epoch_vnic_id*/ h2s_epoch1_id,
                /*h2s_epoch_mapping*/0, /*h2s_epoch_mapping_id*/0,
                /*h2s_policer_bw1_id*/0, /*h2s_policer_bw2_id*/0,
                /*h2s_vnic_stats_id*/0, /*h2s_vnic_stats_mask*/ vnic_stats_mask,
                /*h2s_vnic_histogram_latency_id*/0, /*h2s_vnic_histogram_packet_len_id*/0,
                /*h2s_tcp_flags_bitmap*/0,
                /*h2s_session_rewrite_id*/ h2s_session_rewrite_id,
                /*h2s_allowed_flow_state_bitmask*/0,
                /*h2s_egress_action*/EGRESS_ACTION_NONE,

                /*s2h_epoch_vnic*/ 0, /*s2h_epoch_vnic_id*/ 0,
                /*s2h_epoch_mapping*/0, /*s2h_epoch_mapping_id*/0,
                /*s2h_policer_bw1_id*/0, /*s2h_policer_bw2_id*/0,
                /*s2h_vnic_stats_id*/0, /*s2h_vnic_stats_mask*/ vnic_stats_mask,
                /*s2h_vnic_histogram_latency_id*/0, /*s2h_vnic_histogram_packet_len_id*/0,
                /*s2h_tcp_flags_bitmap*/0,
                /*s2h_session_rewrite_id*/ s2h_session_rewrite_id,
                /*s2h_allowed_flow_state_bitmask*/0,
                /*s2h_egress_action*/EGRESS_ACTION_NONE
                );
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // Setup Normalized Flow entry
    ret = create_flow_v4_tcp_udp(g_udp_vnic_id, g_h2s_sip, g_h2s_dip,
            g_h2s_proto, g_h2s_sport, g_h2s_dport,
            PDS_FLOW_SPEC_INDEX_SESSION, g_session_index);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    g_session_index++;

    return ret;
}

sdk_ret_t
athena_gtest_setup_flows_slowpath(void)
{
    sdk_ret_t       ret = SDK_RET_OK;

    ret = setup_flow_v4_udp_epoch_mismatch();

    if (ret != SDK_RET_OK) {
        return ret;
    }

    return ret;

}

/*
 * Host to Switch UDP flow: Packet to be sent
 */
static uint8_t g_snd_pkt_h2s[] = {
    0x00, 0x00, 0xF1, 0xD0, 0xD1, 0xD0, 0x00, 0x00,
    0x00, 0x40, 0x08, 0x01, 0x81, 0x00, 0x00, 0x03,
    0x08, 0x00, 0x45, 0x00, 0x00, 0x50, 0x00, 0x01,
    0x00, 0x00, 0x40, 0x11, 0xB6, 0x9A, 0x02, 0x00,
    0x00, 0x01, 0xC0, 0x00, 0x02, 0x01, 0x03, 0xE8,
    0x27, 0x10, 0x00, 0x3C, 0x00, 0x00, 0x61, 0x62,
    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A,
    0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x7A, 0x78, 0x79,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6A, 0x6C, 0x6B, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x7A,
    0x78, 0x79
};

sdk_ret_t
athena_gtest_test_flows_slowpath(void)
{
    sdk_ret_t           ret = SDK_RET_OK;

    ret = send_packet("UDP-IPv4-epoch-mismatch: h2s pkt", g_snd_pkt_h2s, sizeof(g_snd_pkt_h2s), g_h_port,
                       NULL, 0, 0);
    if (ret != SDK_RET_OK) {
        return ret;
    }

#if 0
    ret = send_packet("TCP-IPv4: s2h pkt", g_snd_pkt_s2h, sizeof(g_snd_pkt_s2h), g_s_port,
                       g_rcv_pkt_s2h, sizeof(g_rcv_pkt_s2h), g_h_port);
    if (ret != SDK_RET_OK) {
        return ret;
    }
#endif
    return ret;
}
