//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
#include "nic/apollo/test/athena/api/flow_session_info/utils.hpp"

#define UPDATE_DELTA 20

// Data templates
uint16_t epoch_vnic = 20;
uint16_t epoch_mapping = 30;
uint8_t host_mac[ETH_ADDR_LEN] = { 0x22, 0x33, 0x44, 0x11, 0x00, 0x0 };
uint8_t vnic_stats_mask[PDS_FLOW_STATS_MASK_LEN] = { 0, 0x0F, 0x2F, 0xFF };

void
fill_key (pds_flow_session_key_t *key, uint32_t index, uint8_t direction)
{
     memset(key, 0, sizeof(pds_flow_session_key_t));
     key->direction = direction;
     key->session_info_id = index;
     return;
}

void
fill_data (pds_flow_session_data_t *data, uint32_t index,
           uint8_t direction, bool update)
{
    pds_flow_session_flow_data_t *h2s_info;
    pds_flow_session_flow_data_t *s2h_info;
    uint32_t info_index = 0;

    if (!update) {
        info_index = index;
        data->conntrack_id = index;
        data->skip_flow_log = index % 2;
        memcpy(data->host_mac, host_mac, ETH_ADDR_LEN);
    } else {
        info_index = index + 300;
    }

    if (direction & HOST_TO_SWITCH) {
        h2s_info = &data->host_to_switch_flow_info;
        h2s_info->epoch_vnic = epoch_vnic + info_index;
        h2s_info->epoch_vnic_id = info_index;
        h2s_info->epoch_mapping = epoch_mapping + info_index;
        h2s_info->epoch_mapping_id = info_index;
        h2s_info->policer_bw1_id = info_index;
        h2s_info->policer_bw2_id = info_index;
        h2s_info->vnic_stats_id = info_index;
        memcpy(h2s_info->vnic_stats_mask, vnic_stats_mask,
               PDS_FLOW_STATS_MASK_LEN);
        h2s_info->vnic_histogram_latency_id = info_index;
        h2s_info->vnic_histogram_packet_len_id = info_index;
        h2s_info->tcp_flags_bitmap = 0xCF; 
        h2s_info->rewrite_id = info_index;
        h2s_info->allowed_flow_state_bitmask = 0x3FF;
        h2s_info->egress_action = EGRESS_ACTION_TX_TO_SWITCH;
    }
    if (direction & SWITCH_TO_HOST) {
        s2h_info = &data->switch_to_host_flow_info;
        s2h_info->epoch_vnic = epoch_vnic + info_index;
        s2h_info->epoch_vnic_id = info_index;
        s2h_info->epoch_mapping = epoch_mapping + info_index;
        s2h_info->epoch_mapping_id = info_index;
        s2h_info->policer_bw1_id = info_index;
        s2h_info->policer_bw2_id = info_index;
        s2h_info->vnic_stats_id = info_index;
        memcpy(s2h_info->vnic_stats_mask, vnic_stats_mask,
               PDS_FLOW_STATS_MASK_LEN);
        s2h_info->vnic_histogram_latency_id = info_index;
        s2h_info->vnic_histogram_packet_len_id = info_index;
        s2h_info->tcp_flags_bitmap = 0xFC; 
        s2h_info->rewrite_id = info_index;
        s2h_info->allowed_flow_state_bitmask = 0x11;
        s2h_info->egress_action = EGRESS_ACTION_TX_TO_HOST;
    }
    return;
}

void
fill_scale_data (pds_flow_session_data_t *data, uint32_t index)
{
    pds_flow_session_flow_data_t *h2s_info;
    pds_flow_session_flow_data_t *s2h_info;
    uint32_t info_index = 0;
    uint8_t direction = (index % 2) + 1;

    info_index = index;
    data->conntrack_id = index;
    data->skip_flow_log = index % 2;
    memcpy(data->host_mac, host_mac, ETH_ADDR_LEN);

    if (direction & HOST_TO_SWITCH) {
        h2s_info = &data->host_to_switch_flow_info;
        h2s_info->epoch_vnic = epoch_vnic + info_index;
        h2s_info->epoch_vnic_id = info_index;
        h2s_info->epoch_mapping = epoch_mapping + info_index;
        h2s_info->epoch_mapping_id = info_index;
        h2s_info->policer_bw1_id = info_index;
        h2s_info->policer_bw2_id = info_index;
        h2s_info->vnic_stats_id = info_index;
        memcpy(h2s_info->vnic_stats_mask, vnic_stats_mask,
               PDS_FLOW_STATS_MASK_LEN);
        h2s_info->vnic_histogram_latency_id = info_index;
        h2s_info->vnic_histogram_packet_len_id = info_index;
        h2s_info->tcp_flags_bitmap = 0xCF; 
        h2s_info->rewrite_id = info_index;
        h2s_info->allowed_flow_state_bitmask = 0x3FF;
        h2s_info->egress_action = EGRESS_ACTION_TX_TO_SWITCH;
    }
    if (direction & SWITCH_TO_HOST) {
        s2h_info = &data->switch_to_host_flow_info;
        s2h_info->epoch_vnic = epoch_vnic + info_index;
        s2h_info->epoch_vnic_id = info_index;
        s2h_info->epoch_mapping = epoch_mapping + info_index;
        s2h_info->epoch_mapping_id = info_index;
        s2h_info->policer_bw1_id = info_index;
        s2h_info->policer_bw2_id = info_index;
        s2h_info->vnic_stats_id = info_index;
        memcpy(s2h_info->vnic_stats_mask, vnic_stats_mask,
               PDS_FLOW_STATS_MASK_LEN);
        s2h_info->vnic_histogram_latency_id = info_index;
        s2h_info->vnic_histogram_packet_len_id = info_index;
        s2h_info->tcp_flags_bitmap = 0xFC; 
        s2h_info->rewrite_id = info_index;
        s2h_info->allowed_flow_state_bitmask = 0x11;
        s2h_info->egress_action = EGRESS_ACTION_TX_TO_HOST;
    }
    return;
}

void
update_scale_data (pds_flow_session_data_t *data, uint32_t index)
{
    uint32_t updated_index = index + UPDATE_DELTA;
    uint8_t direction = (index % 2) + 1;

    fill_scale_data(data, updated_index);
    return;
}
