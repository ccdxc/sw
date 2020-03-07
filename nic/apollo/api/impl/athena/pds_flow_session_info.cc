//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena flow session implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
#include "gen/p4gen/athena/include/p4pd.h"

using namespace sdk;

extern "C" {

// Helper function to fill spec with direction specific session info
static void
flow_session_info_spec_fill (pds_flow_session_spec_t *spec,
                             session_info_session_info_t *session_info,
                             uint8_t direction)
{
    if (direction & SWITCH_TO_HOST) {
        spec->data.switch_to_host_flow_info.epoch_vnic =
            session_info->s2h_epoch_vnic_value;
        spec->data.switch_to_host_flow_info.epoch_vnic_id =
            session_info->s2h_epoch_vnic_id;
        spec->data.switch_to_host_flow_info.epoch_mapping =
            session_info->s2h_epoch_mapping_value;
        spec->data.switch_to_host_flow_info.epoch_mapping_id =
            session_info->s2h_epoch_mapping_id;
        spec->data.switch_to_host_flow_info.policer_bw1_id =
            session_info->s2h_throttle_bw1_id;
        spec->data.switch_to_host_flow_info.policer_bw2_id =
            session_info->s2h_throttle_bw2_id;
        spec->data.switch_to_host_flow_info.vnic_stats_id =
            session_info->s2h_vnic_statistics_id;
        // FIXME: Truncated copy, will fix later after P4 changes
        memcpy(spec->data.switch_to_host_flow_info.vnic_stats_mask,
               (uint8_t *)&session_info->s2h_vnic_statistics_mask, 4);
        spec->data.switch_to_host_flow_info.vnic_histogram_packet_len_id =
            session_info->s2h_vnic_histogram_packet_len_id;
        spec->data.switch_to_host_flow_info.vnic_histogram_latency_id =
            session_info->s2h_vnic_histogram_latency_id;
        spec->data.switch_to_host_flow_info.tcp_flags_bitmap =
            session_info->s2h_slow_path_tcp_flags_match;
        spec->data.switch_to_host_flow_info.rewrite_id =
            session_info->s2h_session_rewrite_id;
        spec->data.switch_to_host_flow_info.allowed_flow_state_bitmask =
            session_info->s2h_allowed_flow_state_bitmap;
        spec->data.switch_to_host_flow_info.egress_action =
            (pds_egress_action_t)session_info->s2h_egress_action;
    }
    if (direction & HOST_TO_SWITCH) {
        spec->data.host_to_switch_flow_info.epoch_vnic =
            session_info->h2s_epoch_vnic_value;
        spec->data.host_to_switch_flow_info.epoch_vnic_id =
            session_info->h2s_epoch_vnic_id;
        spec->data.host_to_switch_flow_info.epoch_mapping =
            session_info->h2s_epoch_mapping_value;
        spec->data.host_to_switch_flow_info.epoch_mapping_id =
            session_info->h2s_epoch_mapping_id;
        spec->data.host_to_switch_flow_info.policer_bw1_id =
            session_info->h2s_throttle_bw1_id =
        spec->data.host_to_switch_flow_info.policer_bw2_id =
            session_info->h2s_throttle_bw2_id;
        spec->data.host_to_switch_flow_info.vnic_stats_id = 
            session_info->h2s_vnic_statistics_id;
        // FIXME: Truncated copy, will fix later after P4 changes
        memcpy(spec->data.host_to_switch_flow_info.vnic_stats_mask,
               (uint8_t *)&session_info->h2s_vnic_statistics_mask, 4);
        spec->data.host_to_switch_flow_info.vnic_histogram_packet_len_id =
            session_info->h2s_vnic_histogram_packet_len_id;
        spec->data.host_to_switch_flow_info.vnic_histogram_latency_id =
            session_info->h2s_vnic_histogram_latency_id;
        spec->data.host_to_switch_flow_info.tcp_flags_bitmap =
            session_info->h2s_slow_path_tcp_flags_match;
        spec->data.host_to_switch_flow_info.rewrite_id =
            session_info->h2s_session_rewrite_id;
        spec->data.host_to_switch_flow_info.allowed_flow_state_bitmask =
            session_info->h2s_allowed_flow_state_bitmap;
        spec->data.host_to_switch_flow_info.egress_action =
            (pds_egress_action_t)session_info->h2s_egress_action;
    }
    return;
}

static sdk_ret_t
pds_flow_session_info_write (pds_flow_session_spec_t *spec, bool update)
{
    p4pd_error_t p4pd_ret;
    uint32_t session_info_id;
    session_info_actiondata_t session_actiondata = { 0 };
    session_info_actiondata_t rd_session_actiondata = { 0 };
    session_info_session_info_t *session_info, *rd_session_info;

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return SDK_RET_INVALID_ARG;
    }
    session_info_id = spec->key.session_info_id;
    if ((session_info_id == 0) ||
        (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX)) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return SDK_RET_INVALID_ARG;
    }
    if (!((spec->key.direction & HOST_TO_SWITCH) ||
        (spec->key.direction & SWITCH_TO_HOST))) {
        PDS_TRACE_ERR("Direction %u is invalid", spec->key.direction);
        return SDK_RET_INVALID_ARG;
    }

    // For update, check if there is an entry already at the index
    if (update) {
        if (p4pd_global_entry_read(P4TBL_ID_SESSION_INFO, session_info_id,
                                   NULL, NULL, &rd_session_actiondata) !=
                                   P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to read session info table at index %u",
                          session_info_id);
            return SDK_RET_HW_READ_ERR;
        }
        rd_session_info =
            &rd_session_actiondata.action_u.session_info_session_info;
        if (!rd_session_info->valid_flag) {
            PDS_TRACE_ERR("No entry in session info table at index %u",
                          session_info_id);
            return SDK_RET_ENTRY_NOT_FOUND;
        }
        // Copy the other already written fields from table entry
        spec->data.conntrack_id = rd_session_info->conntrack_id;
        spec->data.skip_flow_log = rd_session_info->skip_flow_log;
        memcpy(spec->data.host_mac, rd_session_info->smac, ETH_ADDR_LEN);
        if (spec->key.direction & HOST_TO_SWITCH)
            flow_session_info_spec_fill(spec, rd_session_info, SWITCH_TO_HOST);
        else
            flow_session_info_spec_fill(spec, rd_session_info, HOST_TO_SWITCH);
    }

    session_actiondata.action_id = SESSION_INFO_SESSION_INFO_ID;
    session_info =
        &session_actiondata.action_u.session_info_session_info;
    if (spec->key.direction & HOST_TO_SWITCH) {
        session_info->h2s_epoch_vnic_value =
            spec->data.host_to_switch_flow_info.epoch_vnic;
        session_info->h2s_epoch_vnic_id =
            spec->data.host_to_switch_flow_info.epoch_vnic_id;
        session_info->h2s_epoch_mapping_value =
            spec->data.host_to_switch_flow_info.epoch_mapping;
        session_info->h2s_epoch_mapping_id =
            spec->data.host_to_switch_flow_info.epoch_mapping_id;
        session_info->h2s_throttle_bw1_id =
            spec->data.host_to_switch_flow_info.policer_bw1_id;
        session_info->h2s_throttle_bw2_id =
            spec->data.host_to_switch_flow_info.policer_bw2_id;
        session_info->h2s_vnic_statistics_id =
            spec->data.host_to_switch_flow_info.vnic_stats_id;
        // FIXME: Truncated copy, will fix later after P4 changes
        memcpy((uint8_t *)&session_info->h2s_vnic_statistics_mask,
               spec->data.host_to_switch_flow_info.vnic_stats_mask, 4);
        session_info->h2s_vnic_histogram_latency_id =
            spec->data.host_to_switch_flow_info.vnic_histogram_latency_id;
        session_info->h2s_vnic_histogram_packet_len_id =
            spec->data.host_to_switch_flow_info.vnic_histogram_packet_len_id;
        session_info->h2s_slow_path_tcp_flags_match =
            spec->data.host_to_switch_flow_info.tcp_flags_bitmap;
        session_info->h2s_session_rewrite_id =
            spec->data.host_to_switch_flow_info.rewrite_id;
        session_info->h2s_allowed_flow_state_bitmap =
            spec->data.host_to_switch_flow_info.allowed_flow_state_bitmask;
        session_info->h2s_egress_action =
            spec->data.host_to_switch_flow_info.egress_action;
    }
    if (spec->key.direction & SWITCH_TO_HOST) {
        session_info->s2h_epoch_vnic_value =
            spec->data.switch_to_host_flow_info.epoch_vnic;
        session_info->s2h_epoch_vnic_id =
            spec->data.switch_to_host_flow_info.epoch_vnic_id;
        session_info->s2h_epoch_mapping_value =
            spec->data.switch_to_host_flow_info.epoch_mapping;
        session_info->s2h_epoch_mapping_id =
            spec->data.switch_to_host_flow_info.epoch_mapping_id;
        session_info->s2h_throttle_bw1_id =
            spec->data.switch_to_host_flow_info.policer_bw1_id;
        session_info->s2h_throttle_bw2_id =
            spec->data.switch_to_host_flow_info.policer_bw2_id;
        session_info->s2h_vnic_statistics_id =
            spec->data.switch_to_host_flow_info.vnic_stats_id;
        // FIXME: Truncated copy, will fix later after P4 changes
        memcpy((uint8_t *)&session_info->s2h_vnic_statistics_mask,
               spec->data.switch_to_host_flow_info.vnic_stats_mask, 4);
        session_info->s2h_vnic_histogram_latency_id =
            spec->data.switch_to_host_flow_info.vnic_histogram_latency_id;
        session_info->s2h_vnic_histogram_packet_len_id =
            spec->data.switch_to_host_flow_info.vnic_histogram_packet_len_id;
        session_info->s2h_slow_path_tcp_flags_match =
            spec->data.switch_to_host_flow_info.tcp_flags_bitmap;
        session_info->s2h_session_rewrite_id =
            spec->data.switch_to_host_flow_info.rewrite_id;
        session_info->s2h_allowed_flow_state_bitmap =
            spec->data.switch_to_host_flow_info.allowed_flow_state_bitmask;
        session_info->s2h_egress_action =
            spec->data.switch_to_host_flow_info.egress_action;
    }
    session_info->valid_flag = 1;
    session_info->conntrack_id = spec->data.conntrack_id;
    session_info->skip_flow_log = spec->data.skip_flow_log;
    memcpy(session_info->smac, spec->data.host_mac, ETH_ADDR_LEN);
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO,
                                       session_info_id, NULL, NULL,
                                       &session_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to write session info table at index %u",
                       session_info_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    return SDK_RET_OK;
}

sdk_ret_t
pds_flow_session_info_create (pds_flow_session_spec_t *spec)
{
    return pds_flow_session_info_write(spec, false);
}

sdk_ret_t
pds_flow_session_info_read (pds_flow_session_key_t *key,
                            pds_flow_session_info_t *info)
{
    p4pd_error_t p4pd_ret;
    uint32_t session_info_id;
    session_info_actiondata_t session_actiondata = { 0 };
    session_info_session_info_t *session_info;

    if (!key || !info) {
        PDS_TRACE_ERR("key or info is null");
        return SDK_RET_INVALID_ARG;
    }
    session_info_id = key->session_info_id;
    if ((session_info_id == 0) ||
        (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX)) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return SDK_RET_INVALID_ARG;
    }
    if (!((key->direction & HOST_TO_SWITCH) ||
        (key->direction & SWITCH_TO_HOST))) {
        PDS_TRACE_ERR("Direction %u is invalid", key->direction);
        return SDK_RET_INVALID_ARG;
    }

    // Check if there is an entry already at the index
    if (p4pd_global_entry_read(P4TBL_ID_SESSION_INFO, session_info_id,
                               NULL, NULL, &session_actiondata) !=
                               P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read session info table at index %u",
                      session_info_id);
        return SDK_RET_HW_READ_ERR;
    }
    session_info =
        &session_actiondata.action_u.session_info_session_info;
    if (!session_info->valid_flag) {
        PDS_TRACE_ERR("No entry in session info table at index %u",
                      session_info_id);
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    info->spec.data.conntrack_id = session_info->conntrack_id;
    info->spec.data.skip_flow_log = session_info->skip_flow_log;
    memcpy(info->spec.data.host_mac, session_info->smac, ETH_ADDR_LEN);
    flow_session_info_spec_fill(&info->spec, session_info, key->direction);
    return SDK_RET_OK;
}

sdk_ret_t
pds_flow_session_info_update (pds_flow_session_spec_t *spec)
{
    return pds_flow_session_info_write(spec, true);
}

sdk_ret_t
pds_flow_session_info_delete (pds_flow_session_key_t *key)
{
    p4pd_error_t p4pd_ret;
    uint32_t session_info_id;
    session_info_actiondata_t session_actiondata = { 0 };

    if (!key) {
        PDS_TRACE_ERR("key is null");
        return SDK_RET_INVALID_ARG;
    }
    session_info_id = key->session_info_id;
    if ((session_info_id == 0) ||
        (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX)) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return SDK_RET_INVALID_ARG;
    }
    if (!((key->direction & HOST_TO_SWITCH) ||
        (key->direction & SWITCH_TO_HOST))) {
        PDS_TRACE_ERR("Direction %u is invalid", key->direction);
        return SDK_RET_INVALID_ARG;
    }

    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO,
                                       session_info_id, NULL, NULL,
                                       &session_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to delete session info table at index %u",
                      session_info_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }
    return SDK_RET_OK;
}

}
