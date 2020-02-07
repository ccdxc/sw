//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session.h"
#include "nic/apollo/test/athena/api/flow_session/utils.hpp"

// Data templates
uint8_t smac[ETH_ADDR_LEN] = { 0x22, 0x33, 0x44, 0x0, 0x0, 0x0 };
uint8_t dmac[ETH_ADDR_LEN] = { 0x55, 0x66, 0x77, 0x0, 0x0, 0x0 };
uint16_t port = 1024;
uint32_t ip_saddr = 0x14000000;
uint32_t ip_daddr = 0x1E000000;
uint32_t mpls_label = 100;
uint8_t pkt_rewrite_ip[IP6_ADDR8_LEN] = { 0x88, 0x99, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0 }; 
uint16_t epoch1 = 20;
uint16_t epoch2 = 30;

void
fill_key (pds_flow_session_key_t *key, uint32_t index, uint8_t direction)
{
     memset(key, 0, sizeof(pds_flow_session_key_t));
     key->direction = direction;
     key->session_info_id = index;
     return;
}

static void
fill_rewrite_info (pds_flow_session_rewrite_info_t *rewrite_info,
                   uint32_t index, uint32_t ru_index)
{
    uint8_t user_pkt_rewrite_ip[IP6_ADDR8_LEN] = { 0 };

    rewrite_info->strip_encap_header = ru_index % 2;
    rewrite_info->strip_l2_header = ru_index % 2;
    rewrite_info->strip_vlan_tag = ru_index % 2;

    memcpy(user_pkt_rewrite_ip, pkt_rewrite_ip, IP6_ADDR8_LEN);
    user_pkt_rewrite_ip[2] = ru_index & 0xFF;
    user_pkt_rewrite_ip[15] = ru_index & 0xFF;
    memcpy(rewrite_info->user_packet_rewrite_ip, user_pkt_rewrite_ip,
           IP6_ADDR8_LEN);

    rewrite_info->encap_type =
        (pds_flow_session_encap_t)(ENCAP_TYPE_MPLSOUDP + (index % 2));
    if (rewrite_info->encap_type == ENCAP_TYPE_MPLSOUDP) {
        pds_flow_session_mplsoudp_encap_t *mplsoudp_encap =
            &rewrite_info->u.mplsoudp_encap;
        memcpy(mplsoudp_encap->l2_encap.smac, smac, ETH_ADDR_LEN);
        memcpy(mplsoudp_encap->l2_encap.dmac, dmac, ETH_ADDR_LEN);
        mplsoudp_encap->l2_encap.insert_vlan_tag = index % 2;
        mplsoudp_encap->l2_encap.vlan_id = ru_index & 0xFFF;
        mplsoudp_encap->ip_encap.ip_saddr = ip_saddr + ru_index;
        mplsoudp_encap->ip_encap.ip_daddr = ip_daddr + ru_index;
        mplsoudp_encap->udp_encap.udp_sport = port + ru_index;
        mplsoudp_encap->udp_encap.udp_dport = port - ru_index;
        mplsoudp_encap->mpls1_label = mpls_label + 1;
        mplsoudp_encap->mpls2_label = mpls_label + 2;
    } else if (rewrite_info->encap_type == ENCAP_TYPE_MPLSOGRE) {
        pds_flow_session_mplsogre_encap_t *mplsogre_encap =
            &rewrite_info->u.mplsogre_encap;
        memcpy(mplsogre_encap->l2_encap.smac, smac, ETH_ADDR_LEN);
        memcpy(mplsogre_encap->l2_encap.dmac, dmac, ETH_ADDR_LEN);
        mplsogre_encap->l2_encap.insert_vlan_tag = index % 2;
        mplsogre_encap->l2_encap.vlan_id = ru_index & 0xFFF;
        mplsogre_encap->ip_encap.ip_saddr = ip_saddr + ru_index;
        mplsogre_encap->ip_encap.ip_daddr = ip_daddr + ru_index;
        mplsogre_encap->mpls1_label = mpls_label + ru_index + 1;
        mplsogre_encap->mpls2_label = mpls_label + ru_index + 2;
        mplsogre_encap->mpls3_label = mpls_label + ru_index + 3;
    }
    return;
}

void
fill_data (pds_flow_session_data_t *data, uint32_t index,
           uint8_t direction, uint8_t update_mode)
{
    pds_flow_session_flow_data_t *h2s_info;
    pds_flow_session_flow_data_t *s2h_info;
    pds_flow_session_rewrite_info_t *rewrite_info;
    uint32_t cu_index = 0;
    uint32_t ru_index = 0;

    if (update_mode == COMMON_UPDATE) {
        data->conntrack_id = index + 100;
        cu_index = index + 100;
        //ru_index = index + 100;
    } else if (update_mode == REWRITE_UPDATE) {
        //data->conntrack_id = index + 200;
        //cu_index = index + 200;
        ru_index = index + 200;
    } else if (update_mode == ALL_UPDATE) {
        data->conntrack_id = index + 300;
        cu_index = index + 300;
        ru_index = index + 300;
    } else {
        data->conntrack_id = index;
        cu_index = index;
        ru_index = index;
    }

    if (direction & HOST_TO_SWITCH) {
        h2s_info = &data->host_to_switch_flow_info;
        h2s_info->epoch1 = epoch1 + ru_index;
        h2s_info->epoch1_id = ru_index;
        h2s_info->epoch2 = epoch2 + ru_index;
        h2s_info->epoch2_id = ru_index;
        h2s_info->allowed_flow_state_bitmask = 0x3FF;
        h2s_info->egress_action = EGRESS_ACTION_TX_TO_SWITCH;
        if (update_mode != REWRITE_UPDATE) {
            h2s_info->policer_pps_id = cu_index;
            h2s_info->policer_bw_id = cu_index;
            h2s_info->statistics_id = cu_index;
            h2s_info->statistics_mask = 0x3FF;
            h2s_info->histogram_id = cu_index;
        }
    }
    if (direction & SWITCH_TO_HOST) {
        s2h_info = &data->switch_to_host_flow_info;
        s2h_info->epoch1 = epoch1 + ru_index;
        s2h_info->epoch1_id = ru_index;
        s2h_info->epoch2 = epoch2 + ru_index;
        s2h_info->epoch2_id = ru_index;
        s2h_info->allowed_flow_state_bitmask = 0x3FF;
        s2h_info->egress_action = EGRESS_ACTION_TX_TO_HOST;
        if (update_mode != REWRITE_UPDATE) {
            s2h_info->policer_pps_id = cu_index;
            s2h_info->policer_bw_id = cu_index;
            s2h_info->statistics_id = cu_index;
            s2h_info->statistics_mask = 0x3FF;
            s2h_info->histogram_id = cu_index;
        }
    }

    if (update_mode != COMMON_UPDATE) {
        if (direction & HOST_TO_SWITCH) {
            rewrite_info = &data->host_to_switch_flow_info.rewrite_info;
            fill_rewrite_info(rewrite_info, index, ru_index);
        }
        if (direction & SWITCH_TO_HOST) {
            rewrite_info = &data->switch_to_host_flow_info.rewrite_info;
            fill_rewrite_info(rewrite_info, index, ru_index);
        }
    }
    return;
}
