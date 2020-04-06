//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// athena flow session info implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/p4/p4_api.hpp"
#include "nic/sdk/lib/p4/p4_utils.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"
#include "gen/p4gen/athena/include/p4pd.h"
#include "nic/sdk/asic/pd/pd.hpp"
#include "gen/p4gen/p4/include/ftl.h"


#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE   8
#endif

using namespace sdk;

extern "C" {

static pds_ret_t
pds_flow_session_entry_setup (session_info_entry_t *entry,
                              pds_flow_session_data_t *data,
                              uint8_t direction)
{
    uint64_t smac = 0;

    if (!entry) {
        PDS_TRACE_ERR("entry is null");
        return PDS_RET_INVALID_ARG;
    }
    entry->set_skip_flow_log(data->skip_flow_log);
    entry->set_conntrack_id(data->conntrack_id);
    memcpy(&smac, data->host_mac,ETH_ADDR_LEN);
    entry->set_smac(smac);

    if (direction & HOST_TO_SWITCH) { 
        entry->set_h2s_epoch_vnic_value(
                data->host_to_switch_flow_info.epoch_vnic);
        entry->set_h2s_epoch_vnic_id(
                data->host_to_switch_flow_info.epoch_vnic_id);
        entry->set_h2s_epoch_mapping_value(
                data->host_to_switch_flow_info.epoch_mapping);
        entry->set_h2s_epoch_mapping_id(
                data->host_to_switch_flow_info.epoch_mapping_id);
        entry->set_h2s_throttle_bw1_id(
                data->host_to_switch_flow_info.policer_bw1_id);
        entry->set_h2s_throttle_bw2_id(
                data->host_to_switch_flow_info.policer_bw2_id);
        entry->set_h2s_vnic_statistics_id(
                data->host_to_switch_flow_info.vnic_stats_id);
        entry->set_h2s_vnic_statistics_mask(
                *(uint32_t *)data->host_to_switch_flow_info.vnic_stats_mask);
        entry->set_h2s_vnic_histogram_packet_len_id(
                data->host_to_switch_flow_info.vnic_histogram_packet_len_id);
        entry->set_h2s_vnic_histogram_latency_id(
                data->host_to_switch_flow_info.vnic_histogram_latency_id);
        entry->set_h2s_slow_path_tcp_flags_match(
                data->host_to_switch_flow_info.tcp_flags_bitmap);
        entry->set_h2s_session_rewrite_id(
                data->host_to_switch_flow_info.rewrite_id);
        entry->set_h2s_egress_action(
                data->host_to_switch_flow_info.egress_action);
        entry->set_h2s_allowed_flow_state_bitmap(
                data->host_to_switch_flow_info.allowed_flow_state_bitmask);
    } 

    if (direction & SWITCH_TO_HOST) {
        entry->set_s2h_epoch_vnic_value(
                data->switch_to_host_flow_info.epoch_vnic);
        entry->set_s2h_epoch_vnic_id(
                data->switch_to_host_flow_info.epoch_vnic_id);
        entry->set_s2h_epoch_mapping_value(
                data->switch_to_host_flow_info.epoch_mapping);
        entry->set_s2h_epoch_mapping_id(
                data->switch_to_host_flow_info.epoch_mapping_id);
        entry->set_s2h_throttle_bw1_id(
                data->switch_to_host_flow_info.policer_bw1_id);
        entry->set_s2h_throttle_bw2_id(
                data->switch_to_host_flow_info.policer_bw2_id);
        entry->set_s2h_vnic_statistics_id(
                data->switch_to_host_flow_info.vnic_stats_id);
        entry->set_s2h_vnic_statistics_mask(
                *(uint32_t *)data->switch_to_host_flow_info.vnic_stats_mask);
        entry->set_s2h_vnic_histogram_packet_len_id(
                data->switch_to_host_flow_info.vnic_histogram_packet_len_id);
        entry->set_s2h_vnic_histogram_latency_id(
                data->switch_to_host_flow_info.vnic_histogram_latency_id);
        entry->set_s2h_slow_path_tcp_flags_match(
                data->switch_to_host_flow_info.tcp_flags_bitmap);
        entry->set_s2h_session_rewrite_id(
                data->switch_to_host_flow_info.rewrite_id);
        entry->set_s2h_egress_action(
                data->switch_to_host_flow_info.egress_action);
        entry->set_s2h_allowed_flow_state_bitmap(
                data->switch_to_host_flow_info.allowed_flow_state_bitmask);
    }
    entry->set_valid_flag(TRUE);

    return PDS_RET_OK;
}
static pds_ret_t
pds_flow_session_info_spec_fill (pds_flow_session_spec_t *spec,
                                 session_info_entry_t    *entry,
                                 uint8_t direction)
{
    uint32_t   vnic_stats_mask = 0;

    if (!entry || !spec ) {
        PDS_TRACE_ERR("entry (%p) or spec(%p)is null", entry, spec);
        return PDS_RET_INVALID_ARG;
    }

    if (direction & SWITCH_TO_HOST) {
        spec->data.switch_to_host_flow_info.epoch_vnic = 
                entry->get_s2h_epoch_vnic_value();
        spec->data.switch_to_host_flow_info.epoch_vnic_id = 
                entry->get_s2h_epoch_vnic_id();
        spec->data.switch_to_host_flow_info.epoch_mapping = 
                entry->get_s2h_epoch_mapping_value();
        spec->data.switch_to_host_flow_info.epoch_mapping_id  = 
                entry->get_s2h_epoch_mapping_id();
        spec->data.switch_to_host_flow_info.policer_bw1_id = 
                entry->get_s2h_throttle_bw1_id();
        spec->data.switch_to_host_flow_info.policer_bw2_id = 
                entry->get_s2h_throttle_bw2_id();
        spec->data.switch_to_host_flow_info.vnic_stats_id = 
                entry->get_s2h_vnic_statistics_id();
        vnic_stats_mask = entry->get_s2h_vnic_statistics_mask();
        memcpy((char *)spec->data.switch_to_host_flow_info.vnic_stats_mask,  
                (char *)&vnic_stats_mask, PDS_FLOW_STATS_MASK_LEN);
        spec->data.switch_to_host_flow_info.vnic_histogram_packet_len_id = 
                entry->get_s2h_vnic_histogram_packet_len_id();
        spec->data.switch_to_host_flow_info.vnic_histogram_latency_id = 
                entry->get_s2h_vnic_histogram_latency_id();
        spec->data.switch_to_host_flow_info.tcp_flags_bitmap = 
                entry->get_s2h_slow_path_tcp_flags_match();
        spec->data.switch_to_host_flow_info.rewrite_id = 
                entry->get_s2h_session_rewrite_id();
        spec->data.switch_to_host_flow_info.allowed_flow_state_bitmask  = 
                entry->get_s2h_allowed_flow_state_bitmap();
        spec->data.switch_to_host_flow_info.egress_action  = 
                (pds_egress_action_t) entry->get_s2h_egress_action();
    }
    if (direction & HOST_TO_SWITCH) {
        spec->data.host_to_switch_flow_info.epoch_vnic = 
                entry->get_h2s_epoch_vnic_value();
        spec->data.host_to_switch_flow_info.epoch_vnic_id = 
                entry->get_h2s_epoch_vnic_id();
        spec->data.host_to_switch_flow_info.epoch_mapping = 
                entry->get_h2s_epoch_mapping_value();
        spec->data.host_to_switch_flow_info.epoch_mapping_id  = 
                entry->get_h2s_epoch_mapping_id();
        spec->data.host_to_switch_flow_info.policer_bw1_id = 
                entry->get_h2s_throttle_bw1_id();
        spec->data.host_to_switch_flow_info.policer_bw2_id = 
                entry->get_h2s_throttle_bw2_id();
        spec->data.host_to_switch_flow_info.vnic_stats_id = 
                entry->get_h2s_vnic_statistics_id();
        vnic_stats_mask = entry->get_h2s_vnic_statistics_mask();
        memcpy((char *)spec->data.host_to_switch_flow_info.vnic_stats_mask,  
                (char *)&vnic_stats_mask, PDS_FLOW_STATS_MASK_LEN);
        spec->data.host_to_switch_flow_info.vnic_histogram_packet_len_id = 
                entry->get_h2s_vnic_histogram_packet_len_id();
        spec->data.host_to_switch_flow_info.vnic_histogram_latency_id = 
                entry->get_h2s_vnic_histogram_latency_id();
        spec->data.host_to_switch_flow_info.tcp_flags_bitmap = 
                entry->get_h2s_slow_path_tcp_flags_match();
        spec->data.host_to_switch_flow_info.rewrite_id = 
                entry->get_h2s_session_rewrite_id();
        spec->data.host_to_switch_flow_info.allowed_flow_state_bitmask  = 
                entry->get_h2s_allowed_flow_state_bitmap();
        spec->data.host_to_switch_flow_info.egress_action  = 
                (pds_egress_action_t) entry->get_h2s_egress_action();
    }
    return PDS_RET_OK;
}

pds_ret_t
pds_flow_session_info_create (pds_flow_session_spec_t *spec)
{
    p4pd_error_t         p4pd_ret = P4PD_SUCCESS;
    pds_ret_t            ret = PDS_RET_OK;
    session_info_entry_t entry;

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return PDS_RET_INVALID_ARG;
    }

    if ((spec->key.session_info_id == 0) ||
        (spec->key.session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX)) {
        PDS_TRACE_ERR("session id %u is invalid", spec->key.session_info_id);
        return PDS_RET_INVALID_ARG;
    }
    if (!((spec->key.direction & HOST_TO_SWITCH) ||
        (spec->key.direction & SWITCH_TO_HOST))) {
        PDS_TRACE_ERR("Direction %u is invalid", spec->key.direction);
        return PDS_RET_INVALID_ARG;
    }
    entry.clear();
    if ((ret = pds_flow_session_entry_setup(&entry, 
                                            &spec->data, 
                                            spec->key.direction)) != 
            PDS_RET_OK ) {
        return ret;
    }

    p4pd_ret =  entry.write(spec->key.session_info_id);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to write session info table at index %u",
                       spec->key.session_info_id);
        return PDS_RET_HW_PROGRAM_ERR;
    }
    return PDS_RET_OK;
}

pds_ret_t
pds_flow_session_info_read (pds_flow_session_key_t *key,
                            pds_flow_session_info_t *info)
{
    p4pd_error_t              p4pd_ret;
    uint32_t                  session_info_id;
    session_info_entry_t      entry;
    uint64_t                  smac = 0;

    if (!key || !info) {
        PDS_TRACE_ERR("key or info is null");
        return PDS_RET_INVALID_ARG;
    }
    session_info_id = key->session_info_id;
    if ((session_info_id == 0) ||
        (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX)) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return PDS_RET_INVALID_ARG;
    }
    if (!((key->direction & HOST_TO_SWITCH) ||
        (key->direction & SWITCH_TO_HOST))) {
        PDS_TRACE_ERR("Direction %u is invalid", key->direction);
        return PDS_RET_INVALID_ARG;
    }
    entry.clear();
    entry.read(session_info_id);
    if (!entry.get_valid_flag()) {

        // Reading an entry to see if it's valid is a normal action
        // so no need to log.
        // PDS_TRACE_ERR("No entry in session info table at index %u",
        //               session_info_id);
        return PDS_RET_ENTRY_NOT_FOUND;
    }

    info->spec.data.conntrack_id = entry.get_conntrack_id();
    info->spec.data.skip_flow_log = entry.get_skip_flow_log();
    info->status.timestamp = entry.get_timestamp();;
    smac = entry.get_smac();
    memcpy(info->spec.data.host_mac, &smac, ETH_ADDR_LEN);
    return pds_flow_session_info_spec_fill(&info->spec, 
                                           &entry, 
                                           key->direction);
}

pds_ret_t
pds_flow_session_info_update (pds_flow_session_spec_t *spec)
{
    pds_ret_t                 ret = PDS_RET_OK;
    p4pd_error_t              p4pd_ret = P4PD_SUCCESS;;
    session_info_entry_t      entry;
    uint32_t                  session_info_id = 0;

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return PDS_RET_INVALID_ARG;
    }

    session_info_id = spec->key.session_info_id;
    if ((session_info_id == 0) ||
        (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX)) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return PDS_RET_INVALID_ARG;
    }
    if (!((spec->key.direction & HOST_TO_SWITCH) ||
        (spec->key.direction & SWITCH_TO_HOST))) {
        PDS_TRACE_ERR("Direction %u is invalid", spec->key.direction);
        return PDS_RET_INVALID_ARG;
    }
    entry.clear();
    entry.read(session_info_id);
    if ((ret = pds_flow_session_entry_setup(&entry, 
                                            &spec->data, 
                                            spec->key.direction)) != 
            PDS_RET_OK ) {
        return ret;
    } 
    p4pd_ret =  entry.write(spec->key.session_info_id);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to update session info table at index %u",
                      session_info_id);
        return PDS_RET_HW_PROGRAM_ERR;
    }
    return PDS_RET_OK;
}


pds_ret_t
pds_flow_session_info_delete (pds_flow_session_key_t *key)
{
    p4pd_error_t              p4pd_ret = PDS_RET_OK;
    uint32_t                  session_info_id = 0;
    session_info_entry_t      entry;

    if (!key) {
        PDS_TRACE_ERR("key is null");
        return PDS_RET_INVALID_ARG;
    }
    session_info_id = key->session_info_id;
    if ((session_info_id == 0) ||
        (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX)) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return PDS_RET_INVALID_ARG;
    }
    if (!((key->direction & HOST_TO_SWITCH) ||
        (key->direction & SWITCH_TO_HOST))) {
        PDS_TRACE_ERR("Direction %u is invalid", key->direction);
        return PDS_RET_INVALID_ARG;
    }
    entry.clear();
    p4pd_ret =  entry.write(session_info_id);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to delete session info table at index %u",
                      session_info_id);
        return PDS_RET_HW_PROGRAM_ERR;
    }
    return PDS_RET_OK;
}
}

