/*
 * {C} Copyright 2020 Pensando Systems Inc. All rights reserved
 *
 * athena vnic test implementation
 *
 */

#include <pds_flow_session_info.h> 

uint8_t vnic_stats_mask[PDS_FLOW_STATS_MASK_LEN] = { 0, 0x0F, 0x2F, 0xFF };
uint8_t host_mac[ETH_ADDR_LEN] = { 0x22, 0x33, 0x44, 0x11, 0x00, 0x0 };

#define FILL_KEY(key, idx)          \
    {                               \
        key.direction = (idx%2)+1;  \
        key.session_info_id = idx;       \
    }

#define FILL_DATA(data, idx, direction) \
    {\
        data.conntrack_id =  idx;\
        data.skip_flow_log =  idx%2;\
        memcpy(data.host_mac, host_mac, ETH_ADDR_LEN);\
        if (HOST_TO_SWITCH == direction) {\
            data.host_to_switch_flow_info.epoch_vnic = idx%256;\
            data.host_to_switch_flow_info.epoch_vnic_id = idx%256;\
            data.host_to_switch_flow_info.epoch_mapping = idx%256;\
            data.host_to_switch_flow_info.epoch_mapping_id = idx%256;\
            data.host_to_switch_flow_info.policer_bw1_id = idx%256;\
            data.host_to_switch_flow_info.policer_bw2_id = idx%256;\
            data.host_to_switch_flow_info.vnic_stats_id = idx%256;\
            memcpy(&data.host_to_switch_flow_info.vnic_stats_mask,\
                   vnic_stats_mask, \
                   PDS_FLOW_STATS_MASK_LEN);\
            data.host_to_switch_flow_info.vnic_histogram_latency_id = idx%256;\
            data.host_to_switch_flow_info.vnic_histogram_packet_len_id = idx%256;\
            data.host_to_switch_flow_info.tcp_flags_bitmap = 0xcf ;\
            data.host_to_switch_flow_info.rewrite_id = idx; \
            data.host_to_switch_flow_info.allowed_flow_state_bitmask = 0x3ff;\
            data.host_to_switch_flow_info.egress_action = EGRESS_ACTION_TX_TO_SWITCH;\
        } else {\
            data.switch_to_host_flow_info.epoch_vnic = idx%256;\
            data.switch_to_host_flow_info.epoch_vnic_id = idx%256;\
            data.switch_to_host_flow_info.epoch_mapping = idx%256;\
            data.switch_to_host_flow_info.epoch_mapping_id = idx%256;\
            data.switch_to_host_flow_info.policer_bw1_id = idx%256;\
            data.switch_to_host_flow_info.policer_bw2_id = idx%256;\
            data.switch_to_host_flow_info.vnic_stats_id = idx%256;\
            memcpy(&data.switch_to_host_flow_info.vnic_stats_mask,\
                   vnic_stats_mask, \
                   PDS_FLOW_STATS_MASK_LEN);\
            data.switch_to_host_flow_info.vnic_histogram_latency_id = idx%256;\
            data.switch_to_host_flow_info.vnic_histogram_packet_len_id = idx%256;\
            data.switch_to_host_flow_info.tcp_flags_bitmap = 0xcf; \
            data.switch_to_host_flow_info.rewrite_id = idx; \
            data.switch_to_host_flow_info.allowed_flow_state_bitmask = 0x3ff;\
            data.switch_to_host_flow_info.egress_action = EGRESS_ACTION_TX_TO_SWITCH;\
        } \
    }

static inline void
test_pds_flow_session_info_create(uint32_t idx)
{
    pds_flow_session_spec_t   spec = {0};
    pds_ret_t                 ret = 0;

    FILL_KEY(spec.key, idx);

    FILL_DATA(spec.data, idx, (idx%2)+1);
    ret = pds_flow_session_info_create(&spec);
    assert (ret == PDS_RET_OK);
}

static inline void
test_pds_flow_session_info_update(uint32_t idx)
{
    pds_flow_session_spec_t   spec = {0};
    pds_ret_t                 ret = 0;


    FILL_KEY(spec.key, idx);
    FILL_DATA(spec.data, idx, (((idx+1)%2)+1));
    ret = pds_flow_session_info_update(&spec);
    assert (ret == PDS_RET_OK);
}

static inline void
test_pds_flow_session_info_read(uint32_t idx)
{
    pds_flow_session_key_t    key = {0};
    pds_flow_session_info_t   info = {0};
    pds_ret_t                 ret = 0;

    FILL_KEY(key, idx);
    ret = pds_flow_session_info_read(&key, &info);
    assert (ret == PDS_RET_OK);

}

static inline void
test_pds_flow_session_info_delete(uint32_t idx)
{
    pds_flow_session_key_t    key = {0};
    pds_ret_t                 ret = 0;

    FILL_KEY(key, idx);
    ret = pds_flow_session_info_delete(&key);
    assert (ret == PDS_RET_OK);
}

