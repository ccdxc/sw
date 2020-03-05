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
#include "nic/apollo/api/include/athena/pds_flow_session.h"
#include "gen/p4gen/athena/include/p4pd.h"

using namespace sdk;

extern "C" {

static sdk_ret_t
pds_flow_session_info_write (pds_flow_session_spec_t *spec, bool update)
{
#if 0
    p4pd_error_t p4pd_ret;
    uint32_t session_info_id;
    session_info_common_actiondata_t session_common_actiondata = { 0 };
    session_info_common_actiondata_t rd_session_common_actiondata = { 0 };
    session_info_common_session_info_common_t *session_cinfo;
    session_info_h2s_actiondata_t session_h2s_actiondata = { 0 };
    session_info_h2s_session_info_per_direction_t *session_h2sinfo;
    session_info_s2h_actiondata_t session_s2h_actiondata = { 0 };
    session_info_s2h_session_info_per_direction_t *session_s2hinfo;

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return SDK_RET_INVALID_ARG;
    }
    session_info_id = spec->key.session_info_id;
    if (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return SDK_RET_INVALID_ARG;
    }
    if (!((spec->key.direction & HOST_TO_SWITCH) ||
        (spec->key.direction & SWITCH_TO_HOST)))
        return SDK_RET_INVALID_ARG;

    // For update, check if there is an entry already at the index
    if (update) {
        if (p4pd_global_entry_read(P4TBL_ID_SESSION_INFO_COMMON, session_info_id,
                                   NULL, NULL, &rd_session_common_actiondata) !=
                                   P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to read session common table at index %u",
                          session_info_id);
            return SDK_RET_HW_READ_ERR;
        }
        session_cinfo =
            &rd_session_common_actiondata.action_u.session_info_common_session_info_common;
        if (!session_cinfo->valid_flag)
            return SDK_RET_ENTRY_NOT_FOUND;
    }

    // Session common table programming
    session_common_actiondata.action_id =
        SESSION_INFO_COMMON_SESSION_INFO_COMMON_ID;
    session_cinfo =
        &session_common_actiondata.action_u.session_info_common_session_info_common;
    if (spec->key.direction & HOST_TO_SWITCH) {
        session_cinfo->h2s_throttle_pps_id =
            spec->data.host_to_switch_flow_info.policer_pps_id;
        session_cinfo->h2s_throttle_bw_id =
            spec->data.host_to_switch_flow_info.policer_bw_id;
        session_cinfo->h2s_vnic_statistics_id =
            spec->data.host_to_switch_flow_info.statistics_id;
        session_cinfo->h2s_vnic_statistics_mask =
            spec->data.host_to_switch_flow_info.statistics_mask;
        session_cinfo->h2s_vnic_histogram_id =
            spec->data.host_to_switch_flow_info.histogram_id;
    }
    if (spec->key.direction & SWITCH_TO_HOST) {
        session_cinfo->s2h_throttle_pps_id =
            spec->data.switch_to_host_flow_info.policer_pps_id;
        session_cinfo->s2h_throttle_bw_id =
            spec->data.switch_to_host_flow_info.policer_bw_id;
        session_cinfo->s2h_vnic_statistics_id =
            spec->data.switch_to_host_flow_info.statistics_id;
        session_cinfo->s2h_vnic_statistics_mask =
            spec->data.switch_to_host_flow_info.statistics_mask;
        session_cinfo->s2h_vnic_histogram_id =
            spec->data.switch_to_host_flow_info.histogram_id;
    }
    session_cinfo->valid_flag = 1;
    session_cinfo->conntrack_id = spec->data.conntrack_id;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_COMMON,
                                       session_info_id, NULL, NULL,
                                       &session_common_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to write session common table at index %u",
                       session_info_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    // Program the other rewrite tables
    if (spec->key.direction & HOST_TO_SWITCH) {
        session_h2s_actiondata.action_id =
            SESSION_INFO_H2S_SESSION_INFO_PER_DIRECTION_ID;
        session_h2sinfo = 
            &session_h2s_actiondata.action_u.session_info_h2s_session_info_per_direction;
        session_h2sinfo->epoch1_value = 
            spec->data.host_to_switch_flow_info.epoch1;
        session_h2sinfo->epoch1_id = 
            spec->data.host_to_switch_flow_info.epoch1_id;
        session_h2sinfo->epoch2_value = 
            spec->data.host_to_switch_flow_info.epoch2;
        session_h2sinfo->epoch2_id = 
            spec->data.host_to_switch_flow_info.epoch2_id;
        session_h2sinfo->allowed_flow_state_bitmask = 
            spec->data.host_to_switch_flow_info.allowed_flow_state_bitmask;
        session_h2sinfo->egress_action = 
            spec->data.host_to_switch_flow_info.egress_action;
        session_h2sinfo->strip_outer_encap_flag = 
            spec->data.host_to_switch_flow_info.rewrite_info.strip_encap_header;
        session_h2sinfo->strip_l2_header_flag = 
            spec->data.host_to_switch_flow_info.rewrite_info.strip_l2_header;
        session_h2sinfo->strip_vlan_tag_flag = 
            spec->data.host_to_switch_flow_info.rewrite_info.strip_vlan_tag;
        session_h2sinfo->nat_type = 
            spec->data.host_to_switch_flow_info.rewrite_info.user_packet_rewrite_type;
        memcpy(session_h2sinfo->nat_address,
               spec->data.host_to_switch_flow_info.rewrite_info.user_packet_rewrite_ip,
               IP6_ADDR8_LEN);
        session_h2sinfo->encap_type = 
            spec->data.host_to_switch_flow_info.rewrite_info.encap_type;
        if (session_h2sinfo->encap_type == ENCAP_TYPE_L2) {
            session_h2sinfo->add_vlan_tag_flag =
                spec->data.host_to_switch_flow_info.rewrite_info.u.l2_encap.insert_vlan_tag;
            session_h2sinfo->vlan =
                spec->data.host_to_switch_flow_info.rewrite_info.u.l2_encap.vlan_id;
            memcpy(session_h2sinfo->dmac,
                   spec->data.host_to_switch_flow_info.rewrite_info.u.l2_encap.dmac,
                   ETH_ADDR_LEN);
            memcpy(session_h2sinfo->smac,
                   spec->data.host_to_switch_flow_info.rewrite_info.u.l2_encap.smac,
                   ETH_ADDR_LEN);
        } else if (session_h2sinfo->encap_type == ENCAP_TYPE_MPLSOUDP) {
            session_h2sinfo->add_vlan_tag_flag =
                spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.insert_vlan_tag;
            session_h2sinfo->vlan =
                spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.vlan_id;
            memcpy(session_h2sinfo->dmac,
                   spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.dmac,
                   ETH_ADDR_LEN);
            memcpy(session_h2sinfo->smac,
                   spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.smac,
                   ETH_ADDR_LEN);
            session_h2sinfo->ipv4_sa = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_saddr;
            session_h2sinfo->ipv4_da = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_daddr;
            session_h2sinfo->udp_sport = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_sport;
            session_h2sinfo->udp_dport = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_dport;
            session_h2sinfo->mpls_label1 = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.mpls1_label;
            session_h2sinfo->mpls_label2 = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.mpls2_label;
            session_h2sinfo->mpls_label3 = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.mpls3_label;
        } else if (session_h2sinfo->encap_type == ENCAP_TYPE_MPLSOGRE) {
            session_h2sinfo->add_vlan_tag_flag =
                spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.insert_vlan_tag;
            session_h2sinfo->vlan =
                spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.vlan_id;
            memcpy(session_h2sinfo->dmac,
                   spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.dmac,
                   ETH_ADDR_LEN);
            memcpy(session_h2sinfo->smac,
                   spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.smac,
                   ETH_ADDR_LEN);
            session_h2sinfo->ipv4_sa = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.ip_encap.ip_saddr;
            session_h2sinfo->ipv4_da = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.ip_encap.ip_daddr;
            session_h2sinfo->mpls_label1 = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.mpls1_label;
            session_h2sinfo->mpls_label2 = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.mpls2_label;
            session_h2sinfo->mpls_label3 = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.mpls3_label;
        } else if (session_h2sinfo->encap_type != ENCAP_TYPE_NONE) {
            PDS_TRACE_ERR("Unknown encap type");
            return SDK_RET_INVALID_ARG;
        }
        session_h2sinfo->valid_flag = 1;
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_H2S,
                                           session_info_id, NULL, NULL,
                                           &session_h2s_actiondata);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to write session h2s table at index %u",
                           session_info_id);
            return SDK_RET_HW_PROGRAM_ERR;
        }
    }
    if (spec->key.direction & SWITCH_TO_HOST) {
        session_s2h_actiondata.action_id =
            SESSION_INFO_S2H_SESSION_INFO_PER_DIRECTION_ID;
        session_s2hinfo = 
            &session_s2h_actiondata.action_u.session_info_s2h_session_info_per_direction;
        session_s2hinfo->epoch1_value = 
            spec->data.switch_to_host_flow_info.epoch1;
        session_s2hinfo->epoch1_id = 
            spec->data.switch_to_host_flow_info.epoch1_id;
        session_s2hinfo->epoch2_value = 
            spec->data.switch_to_host_flow_info.epoch2;
        session_s2hinfo->epoch2_id = 
            spec->data.switch_to_host_flow_info.epoch2_id;
        session_s2hinfo->allowed_flow_state_bitmask = 
            spec->data.switch_to_host_flow_info.allowed_flow_state_bitmask;
        session_s2hinfo->egress_action = 
            spec->data.switch_to_host_flow_info.egress_action;
        session_s2hinfo->strip_outer_encap_flag = 
            spec->data.switch_to_host_flow_info.rewrite_info.strip_encap_header;
        session_s2hinfo->strip_l2_header_flag = 
            spec->data.switch_to_host_flow_info.rewrite_info.strip_l2_header;
        session_s2hinfo->strip_vlan_tag_flag = 
            spec->data.switch_to_host_flow_info.rewrite_info.strip_vlan_tag;
        session_s2hinfo->nat_type = 
            spec->data.switch_to_host_flow_info.rewrite_info.user_packet_rewrite_type;
        memcpy(session_s2hinfo->nat_address,
               spec->data.switch_to_host_flow_info.rewrite_info.user_packet_rewrite_ip,
               IP6_ADDR8_LEN);
        session_s2hinfo->encap_type = 
            spec->data.switch_to_host_flow_info.rewrite_info.encap_type;
        if (session_s2hinfo->encap_type == ENCAP_TYPE_L2) {
            session_s2hinfo->add_vlan_tag_flag =
                spec->data.switch_to_host_flow_info.rewrite_info.u.l2_encap.insert_vlan_tag;
            session_s2hinfo->vlan =
                spec->data.switch_to_host_flow_info.rewrite_info.u.l2_encap.vlan_id;
            memcpy(session_s2hinfo->dmac,
                   spec->data.switch_to_host_flow_info.rewrite_info.u.l2_encap.dmac,
                   ETH_ADDR_LEN);
            memcpy(session_s2hinfo->smac,
                   spec->data.switch_to_host_flow_info.rewrite_info.u.l2_encap.smac,
                   ETH_ADDR_LEN);
        } else if (session_s2hinfo->encap_type == ENCAP_TYPE_MPLSOUDP) {
            session_s2hinfo->add_vlan_tag_flag =
                spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.insert_vlan_tag;
            session_s2hinfo->vlan =
                spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.vlan_id;
            memcpy(session_s2hinfo->dmac,
                   spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.dmac,
                   ETH_ADDR_LEN);
            memcpy(session_s2hinfo->smac,
                   spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.smac,
                   ETH_ADDR_LEN);
            session_s2hinfo->ipv4_sa = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_saddr;
            session_s2hinfo->ipv4_da = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_daddr;
            session_s2hinfo->udp_sport = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_sport;
            session_s2hinfo->udp_dport = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_dport;
            session_s2hinfo->mpls_label1 = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.mpls1_label;
            session_s2hinfo->mpls_label2 = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.mpls2_label;
            session_s2hinfo->mpls_label3 = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.mpls3_label;
        } else if (session_s2hinfo->encap_type == ENCAP_TYPE_MPLSOGRE) {
            session_s2hinfo->add_vlan_tag_flag =
                spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.insert_vlan_tag;
            session_s2hinfo->vlan =
                spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.vlan_id;
            memcpy(session_s2hinfo->dmac,
                   spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.dmac,
                   ETH_ADDR_LEN);
            memcpy(session_s2hinfo->smac,
                   spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.smac,
                   ETH_ADDR_LEN);
            session_s2hinfo->ipv4_sa = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.ip_encap.ip_saddr;
            session_s2hinfo->ipv4_da = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.ip_encap.ip_daddr;
            session_s2hinfo->mpls_label1 = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.mpls1_label;
            session_s2hinfo->mpls_label2 = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.mpls2_label;
            session_s2hinfo->mpls_label3 = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.mpls3_label;
        } else if (session_s2hinfo->encap_type != ENCAP_TYPE_NONE) {
            PDS_TRACE_ERR("Unknown encap type");
            return SDK_RET_INVALID_ARG;
        }
        session_s2hinfo->valid_flag = 1;
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_S2H,
                                           session_info_id, NULL, NULL,
                                           &session_s2h_actiondata);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to write session s2h table at index %u",
                           session_info_id);
            return SDK_RET_HW_PROGRAM_ERR;
        }
    }
#endif
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
#if 0
    p4pd_error_t p4pd_ret;
    uint32_t session_info_id;
    session_info_common_actiondata_t session_common_actiondata = { 0 };
    session_info_common_session_info_common_t *session_cinfo;
    session_info_h2s_actiondata_t session_h2s_actiondata = { 0 };
    session_info_h2s_session_info_per_direction_t *session_h2sinfo;
    session_info_s2h_actiondata_t session_s2h_actiondata = { 0 };
    session_info_s2h_session_info_per_direction_t *session_s2hinfo;

    if (!key || !info) {
        PDS_TRACE_ERR("key or info is null");
        return SDK_RET_INVALID_ARG;
    }
    session_info_id = key->session_info_id;
    if (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return SDK_RET_INVALID_ARG;
    }
    if (!((key->direction & HOST_TO_SWITCH) ||
        (key->direction & SWITCH_TO_HOST)))
        return SDK_RET_INVALID_ARG;

    // Check if there is an entry already at the index
    if (p4pd_global_entry_read(P4TBL_ID_SESSION_INFO_COMMON, session_info_id,
                               NULL, NULL, &session_common_actiondata) !=
                               P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read session common table at index %u",
                      session_info_id);
        return SDK_RET_HW_READ_ERR;
    }
    session_cinfo =
        &session_common_actiondata.action_u.session_info_common_session_info_common;
    if (!session_cinfo->valid_flag)
        return SDK_RET_ENTRY_NOT_FOUND;

    info->spec.data.conntrack_id = session_cinfo->conntrack_id;
    info->status.timestamp = session_cinfo->timestamp;
    if (key->direction & HOST_TO_SWITCH) {
        info->spec.data.host_to_switch_flow_info.policer_pps_id =
            session_cinfo->h2s_throttle_pps_id =
        info->spec.data.host_to_switch_flow_info.policer_bw_id =
            session_cinfo->h2s_throttle_bw_id;
        info->spec.data.host_to_switch_flow_info.statistics_id = 
            session_cinfo->h2s_vnic_statistics_id;
        info->spec.data.host_to_switch_flow_info.statistics_mask = 
            session_cinfo->h2s_vnic_statistics_mask;
        info->spec.data.host_to_switch_flow_info.histogram_id =
            session_cinfo->h2s_vnic_histogram_id;
    }
    if (key->direction & SWITCH_TO_HOST) {
        info->spec.data.switch_to_host_flow_info.policer_pps_id =
            session_cinfo->s2h_throttle_pps_id;
        info->spec.data.switch_to_host_flow_info.policer_bw_id =
            session_cinfo->s2h_throttle_bw_id;
        info->spec.data.switch_to_host_flow_info.statistics_id =
            session_cinfo->s2h_vnic_statistics_id;
        info->spec.data.switch_to_host_flow_info.statistics_mask =
            session_cinfo->s2h_vnic_statistics_mask;
        info->spec.data.switch_to_host_flow_info.histogram_id =
            session_cinfo->s2h_vnic_histogram_id;
    }

    if (key->direction & HOST_TO_SWITCH) {
        if (p4pd_global_entry_read(P4TBL_ID_SESSION_INFO_H2S,
                                   session_info_id, NULL, NULL,
                                   &session_h2s_actiondata) !=
                                   P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to read session h2s table at index %u",
                          session_info_id);
            return SDK_RET_HW_READ_ERR;
        }
        session_h2sinfo = 
            &session_h2s_actiondata.action_u.session_info_h2s_session_info_per_direction;
        if (!session_h2sinfo->valid_flag)
             goto read_s2h;
        info->spec.data.host_to_switch_flow_info.epoch1 =
            session_h2sinfo->epoch1_value;
        info->spec.data.host_to_switch_flow_info.epoch1_id =
            session_h2sinfo->epoch1_id;
        info->spec.data.host_to_switch_flow_info.epoch2 =
            session_h2sinfo->epoch2_value;
        info->spec.data.host_to_switch_flow_info.epoch2_id =
            session_h2sinfo->epoch2_id;
        info->spec.data.host_to_switch_flow_info.allowed_flow_state_bitmask =
            session_h2sinfo->allowed_flow_state_bitmask;
        info->spec.data.host_to_switch_flow_info.egress_action =
            (pds_egress_action_t)session_h2sinfo->egress_action;
        info->spec.data.host_to_switch_flow_info.rewrite_info.strip_encap_header =
            session_h2sinfo->strip_outer_encap_flag;
        info->spec.data.host_to_switch_flow_info.rewrite_info.strip_l2_header =
            session_h2sinfo->strip_l2_header_flag;
        info->spec.data.host_to_switch_flow_info.rewrite_info.strip_vlan_tag =
            session_h2sinfo->strip_vlan_tag_flag;
        info->spec.data.host_to_switch_flow_info.rewrite_info.user_packet_rewrite_type =
            (pds_user_packet_rewrite_type_t)session_h2sinfo->nat_type;
        memcpy(info->spec.data.host_to_switch_flow_info.rewrite_info.user_packet_rewrite_ip,
               session_h2sinfo->nat_address,
               IP6_ADDR8_LEN);
        info->spec.data.host_to_switch_flow_info.rewrite_info.encap_type =
            (pds_flow_session_encap_t)session_h2sinfo->encap_type;
        if (session_h2sinfo->encap_type == ENCAP_TYPE_L2) {
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.l2_encap.insert_vlan_tag =
                session_h2sinfo->add_vlan_tag_flag;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.l2_encap.vlan_id =
                session_h2sinfo->vlan;
            memcpy(info->spec.data.host_to_switch_flow_info.rewrite_info.u.l2_encap.dmac,
                   session_h2sinfo->dmac,
                   ETH_ADDR_LEN);
            memcpy(info->spec.data.host_to_switch_flow_info.rewrite_info.u.l2_encap.smac,
                   session_h2sinfo->smac,
                   ETH_ADDR_LEN);
        } else if (session_h2sinfo->encap_type == ENCAP_TYPE_MPLSOUDP) {
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.insert_vlan_tag =
                session_h2sinfo->add_vlan_tag_flag;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.vlan_id =
                session_h2sinfo->vlan;
            memcpy(info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.dmac,
                   session_h2sinfo->dmac,
                   ETH_ADDR_LEN);
            memcpy(info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.smac,
                   session_h2sinfo->smac,
                   ETH_ADDR_LEN);
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_saddr = session_h2sinfo->ipv4_sa;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_daddr = session_h2sinfo->ipv4_da;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_sport = session_h2sinfo->udp_sport;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_dport = session_h2sinfo->udp_dport;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.mpls1_label = session_h2sinfo->mpls_label1;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.mpls2_label = session_h2sinfo->mpls_label2;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.mpls3_label = session_h2sinfo->mpls_label3;
        } else if (session_h2sinfo->encap_type == ENCAP_TYPE_MPLSOGRE) {
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.insert_vlan_tag =
                session_h2sinfo->add_vlan_tag_flag;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.vlan_id =
                session_h2sinfo->vlan;
            memcpy(info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.dmac,
                   session_h2sinfo->dmac,
                   ETH_ADDR_LEN);
            memcpy(info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.smac,
                   session_h2sinfo->smac,
                   ETH_ADDR_LEN);
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.ip_encap.ip_saddr = session_h2sinfo->ipv4_sa;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.ip_encap.ip_daddr = session_h2sinfo->ipv4_da;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.mpls1_label = session_h2sinfo->mpls_label1;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.mpls2_label = session_h2sinfo->mpls_label2;
            info->spec.data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.mpls3_label = session_h2sinfo->mpls_label3;
        } else if (session_h2sinfo->encap_type != ENCAP_TYPE_NONE) {
            PDS_TRACE_ERR("Unknown encap type");
            return SDK_RET_HW_READ_ERR;
        }
    }
read_s2h:
    if (key->direction & SWITCH_TO_HOST) {
        if (p4pd_global_entry_read(P4TBL_ID_SESSION_INFO_S2H,
                                   session_info_id, NULL, NULL,
                                   &session_s2h_actiondata) !=
                                   P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to read session s2h table at index %u",
                          session_info_id);
            return SDK_RET_HW_READ_ERR;
        }
        session_s2hinfo = 
            &session_s2h_actiondata.action_u.session_info_s2h_session_info_per_direction;
        if (!session_s2hinfo->valid_flag)
            return SDK_RET_OK;
        info->spec.data.switch_to_host_flow_info.epoch1 =
            session_s2hinfo->epoch1_value;
        info->spec.data.switch_to_host_flow_info.epoch1_id =
            session_s2hinfo->epoch1_id;
        info->spec.data.switch_to_host_flow_info.epoch2 =
            session_s2hinfo->epoch2_value;
        info->spec.data.switch_to_host_flow_info.epoch2_id =
            session_s2hinfo->epoch2_id;
        info->spec.data.switch_to_host_flow_info.allowed_flow_state_bitmask =
            session_s2hinfo->allowed_flow_state_bitmask;
        info->spec.data.switch_to_host_flow_info.egress_action =
            (pds_egress_action_t)session_s2hinfo->egress_action;
        info->spec.data.switch_to_host_flow_info.rewrite_info.strip_encap_header =
            session_s2hinfo->strip_outer_encap_flag;
        info->spec.data.switch_to_host_flow_info.rewrite_info.strip_l2_header =
            session_s2hinfo->strip_l2_header_flag;
        info->spec.data.switch_to_host_flow_info.rewrite_info.strip_vlan_tag =
            session_s2hinfo->strip_vlan_tag_flag;
        info->spec.data.switch_to_host_flow_info.rewrite_info.user_packet_rewrite_type =
            (pds_user_packet_rewrite_type_t)session_s2hinfo->nat_type;
        memcpy(info->spec.data.switch_to_host_flow_info.rewrite_info.user_packet_rewrite_ip,
               session_s2hinfo->nat_address,
               IP6_ADDR8_LEN);
        info->spec.data.switch_to_host_flow_info.rewrite_info.encap_type =
            (pds_flow_session_encap_t)session_s2hinfo->encap_type;
        if (session_s2hinfo->encap_type == ENCAP_TYPE_L2) {
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.l2_encap.insert_vlan_tag =
                session_s2hinfo->add_vlan_tag_flag;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.l2_encap.vlan_id =
                session_s2hinfo->vlan;
            memcpy(info->spec.data.switch_to_host_flow_info.rewrite_info.u.l2_encap.dmac,
                   session_s2hinfo->dmac,
                   ETH_ADDR_LEN);
            memcpy(info->spec.data.switch_to_host_flow_info.rewrite_info.u.l2_encap.smac,
                   session_s2hinfo->smac,
                   ETH_ADDR_LEN);
        } else if (session_s2hinfo->encap_type == ENCAP_TYPE_MPLSOUDP) {
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.insert_vlan_tag =
                session_s2hinfo->add_vlan_tag_flag;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.vlan_id =
                session_s2hinfo->vlan;
            memcpy(info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.dmac,
                   session_s2hinfo->dmac,
                   ETH_ADDR_LEN);
            memcpy(info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.smac,
                   session_s2hinfo->smac,
                   ETH_ADDR_LEN);
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_saddr = session_s2hinfo->ipv4_sa;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_daddr = session_s2hinfo->ipv4_da;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_sport = session_s2hinfo->udp_sport;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_dport = session_s2hinfo->udp_dport;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.mpls1_label = session_s2hinfo->mpls_label1;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.mpls2_label = session_s2hinfo->mpls_label2;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.mpls3_label = session_s2hinfo->mpls_label3;
        } else if (session_s2hinfo->encap_type == ENCAP_TYPE_MPLSOGRE) {
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.insert_vlan_tag =
                session_s2hinfo->add_vlan_tag_flag;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.vlan_id =
                session_s2hinfo->vlan;
            memcpy(info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.dmac,
                   session_s2hinfo->dmac,
                   ETH_ADDR_LEN);
            memcpy(info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.smac,
                   session_s2hinfo->smac,
                   ETH_ADDR_LEN);
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.ip_encap.ip_saddr = session_s2hinfo->ipv4_sa;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.ip_encap.ip_daddr = session_s2hinfo->ipv4_da;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.mpls1_label = session_s2hinfo->mpls_label1;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.mpls2_label = session_s2hinfo->mpls_label2;
            info->spec.data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.mpls3_label = session_s2hinfo->mpls_label3;
        } else if (session_s2hinfo->encap_type != ENCAP_TYPE_NONE) {
            PDS_TRACE_ERR("Unknown encap type");
            return SDK_RET_HW_READ_ERR;
        }
    }
#endif
    return SDK_RET_OK;
}

sdk_ret_t
pds_flow_session_common_info_update (pds_flow_session_spec_t *spec)
{
#if 0
    p4pd_error_t p4pd_ret;
    uint32_t session_info_id;
    session_info_common_actiondata_t session_common_actiondata = { 0 };
    session_info_common_actiondata_t rd_session_common_actiondata = { 0 };
    session_info_common_session_info_common_t *session_cinfo;

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return SDK_RET_INVALID_ARG;
    }
    session_info_id = spec->key.session_info_id;
    if (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return SDK_RET_INVALID_ARG;
    }
    if (!((spec->key.direction & HOST_TO_SWITCH) ||
        (spec->key.direction & SWITCH_TO_HOST)))
        return SDK_RET_INVALID_ARG;

    // Check if there is an entry already at the index
    if (p4pd_global_entry_read(P4TBL_ID_SESSION_INFO_COMMON, session_info_id,
                               NULL, NULL, &rd_session_common_actiondata) !=
                               P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read session common table at index %u",
                      session_info_id);
        return SDK_RET_HW_READ_ERR;
    }
    session_cinfo =
        &rd_session_common_actiondata.action_u.session_info_common_session_info_common;
    if (!session_cinfo->valid_flag)
        return SDK_RET_ENTRY_NOT_FOUND;

    session_common_actiondata.action_id =
        SESSION_INFO_COMMON_SESSION_INFO_COMMON_ID;
    session_cinfo =
        &session_common_actiondata.action_u.session_info_common_session_info_common;
    if (spec->key.direction & HOST_TO_SWITCH) {
        session_cinfo->h2s_throttle_pps_id =
            spec->data.host_to_switch_flow_info.policer_pps_id;
        session_cinfo->h2s_throttle_bw_id =
            spec->data.host_to_switch_flow_info.policer_bw_id;
        session_cinfo->h2s_vnic_statistics_id =
            spec->data.host_to_switch_flow_info.statistics_id;
        session_cinfo->h2s_vnic_statistics_mask =
            spec->data.host_to_switch_flow_info.statistics_mask;
        session_cinfo->h2s_vnic_histogram_id =
            spec->data.host_to_switch_flow_info.histogram_id;
    }
    if (spec->key.direction & SWITCH_TO_HOST) {
        session_cinfo->s2h_throttle_pps_id =
            spec->data.switch_to_host_flow_info.policer_pps_id;
        session_cinfo->s2h_throttle_bw_id =
            spec->data.switch_to_host_flow_info.policer_bw_id;
        session_cinfo->s2h_vnic_statistics_id =
            spec->data.switch_to_host_flow_info.statistics_id;
        session_cinfo->s2h_vnic_statistics_mask =
            spec->data.switch_to_host_flow_info.statistics_mask;
        session_cinfo->s2h_vnic_histogram_id =
            spec->data.switch_to_host_flow_info.histogram_id;
    }
    session_cinfo->valid_flag = 1;
    session_cinfo->conntrack_id = spec->data.conntrack_id;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_COMMON,
                                       session_info_id, NULL, NULL,
                                       &session_common_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to write session common table at index %u",
                       session_info_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }
#endif
    return SDK_RET_OK;
}

sdk_ret_t
pds_flow_session_rewrite_info_update (pds_flow_session_spec_t *spec)
{
#if 0
    p4pd_error_t p4pd_ret;
    uint32_t session_info_id;
    session_info_h2s_actiondata_t session_h2s_actiondata = { 0 };
    session_info_h2s_session_info_per_direction_t *session_h2sinfo;
    session_info_s2h_actiondata_t session_s2h_actiondata = { 0 };
    session_info_s2h_session_info_per_direction_t *session_s2hinfo;
    session_info_common_actiondata_t rd_session_common_actiondata = { 0 };
    session_info_common_session_info_common_t *session_cinfo;

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return SDK_RET_INVALID_ARG;
    }
    session_info_id = spec->key.session_info_id;
    if (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return SDK_RET_INVALID_ARG;
    }
    if (!((spec->key.direction & HOST_TO_SWITCH) ||
        (spec->key.direction & SWITCH_TO_HOST)))
        return SDK_RET_INVALID_ARG;

    // Check if there is an entry already at the index
    if (p4pd_global_entry_read(P4TBL_ID_SESSION_INFO_COMMON, session_info_id,
                               NULL, NULL, &rd_session_common_actiondata) !=
                               P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read session common table at index %u",
                      session_info_id);
        return SDK_RET_HW_READ_ERR;
    }
    session_cinfo =
        &rd_session_common_actiondata.action_u.session_info_common_session_info_common;
    if (!session_cinfo->valid_flag)
        return SDK_RET_ENTRY_NOT_FOUND;

    if (spec->key.direction & HOST_TO_SWITCH) {
        session_h2s_actiondata.action_id =
            SESSION_INFO_H2S_SESSION_INFO_PER_DIRECTION_ID;
        session_h2sinfo = 
            &session_h2s_actiondata.action_u.session_info_h2s_session_info_per_direction;
        session_h2sinfo->epoch1_value = 
            spec->data.host_to_switch_flow_info.epoch1;
        session_h2sinfo->epoch1_id = 
            spec->data.host_to_switch_flow_info.epoch1_id;
        session_h2sinfo->epoch2_value = 
            spec->data.host_to_switch_flow_info.epoch2;
        session_h2sinfo->epoch2_id = 
            spec->data.host_to_switch_flow_info.epoch2_id;
        session_h2sinfo->allowed_flow_state_bitmask = 
            spec->data.host_to_switch_flow_info.allowed_flow_state_bitmask;
        session_h2sinfo->egress_action = 
            spec->data.host_to_switch_flow_info.egress_action;
        session_h2sinfo->strip_outer_encap_flag = 
            spec->data.host_to_switch_flow_info.rewrite_info.strip_encap_header;
        session_h2sinfo->strip_l2_header_flag = 
            spec->data.host_to_switch_flow_info.rewrite_info.strip_l2_header;
        session_h2sinfo->strip_vlan_tag_flag = 
            spec->data.host_to_switch_flow_info.rewrite_info.strip_vlan_tag;
        session_h2sinfo->nat_type = 
            spec->data.host_to_switch_flow_info.rewrite_info.user_packet_rewrite_type;
        memcpy(session_h2sinfo->nat_address,
               spec->data.host_to_switch_flow_info.rewrite_info.user_packet_rewrite_ip,
               IP6_ADDR8_LEN);
        session_h2sinfo->encap_type = 
            spec->data.host_to_switch_flow_info.rewrite_info.encap_type;
        if (session_h2sinfo->encap_type == ENCAP_TYPE_L2) {
            session_h2sinfo->add_vlan_tag_flag =
                spec->data.host_to_switch_flow_info.rewrite_info.u.l2_encap.insert_vlan_tag;
            session_h2sinfo->vlan =
                spec->data.host_to_switch_flow_info.rewrite_info.u.l2_encap.vlan_id;
            memcpy(session_h2sinfo->dmac,
                   spec->data.host_to_switch_flow_info.rewrite_info.u.l2_encap.dmac,
                   ETH_ADDR_LEN);
            memcpy(session_h2sinfo->smac,
                   spec->data.host_to_switch_flow_info.rewrite_info.u.l2_encap.smac,
                   ETH_ADDR_LEN);
        } else if (session_h2sinfo->encap_type == ENCAP_TYPE_MPLSOUDP) {
            session_h2sinfo->add_vlan_tag_flag =
                spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.insert_vlan_tag;
            session_h2sinfo->vlan =
                spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.vlan_id;
            memcpy(session_h2sinfo->dmac,
                   spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.dmac,
                   ETH_ADDR_LEN);
            memcpy(session_h2sinfo->smac,
                   spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.smac,
                   ETH_ADDR_LEN);
            session_h2sinfo->ipv4_sa = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_saddr;
            session_h2sinfo->ipv4_da = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_daddr;
            session_h2sinfo->udp_sport = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_sport;
            session_h2sinfo->udp_dport = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_dport;
            session_h2sinfo->mpls_label1 = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.mpls1_label;
            session_h2sinfo->mpls_label2 = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.mpls2_label;
            session_h2sinfo->mpls_label3 = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsoudp_encap.mpls3_label;
        } else if (session_h2sinfo->encap_type == ENCAP_TYPE_MPLSOGRE) {
            session_h2sinfo->add_vlan_tag_flag =
                spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.insert_vlan_tag;
            session_h2sinfo->vlan =
                spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.vlan_id;
            memcpy(session_h2sinfo->dmac,
                   spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.dmac,
                   ETH_ADDR_LEN);
            memcpy(session_h2sinfo->smac,
                   spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.smac,
                   ETH_ADDR_LEN);
            session_h2sinfo->ipv4_sa = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.ip_encap.ip_saddr;
            session_h2sinfo->ipv4_da = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.ip_encap.ip_daddr;
            session_h2sinfo->mpls_label1 = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.mpls1_label;
            session_h2sinfo->mpls_label2 = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.mpls2_label;
            session_h2sinfo->mpls_label3 = spec->data.host_to_switch_flow_info.rewrite_info.u.mplsogre_encap.mpls3_label;
        } else if (session_h2sinfo->encap_type != ENCAP_TYPE_NONE) {
            PDS_TRACE_ERR("Unknown encap type");
            return SDK_RET_INVALID_ARG;
        }
        session_h2sinfo->valid_flag = 1;
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_H2S,
                                           session_info_id, NULL, NULL,
                                           &session_h2s_actiondata);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to write session h2s table at index %u",
                           session_info_id);
            return SDK_RET_HW_PROGRAM_ERR;
        }
    }
    if (spec->key.direction & SWITCH_TO_HOST) {
        session_s2h_actiondata.action_id =
            SESSION_INFO_S2H_SESSION_INFO_PER_DIRECTION_ID;
        session_s2hinfo = 
            &session_s2h_actiondata.action_u.session_info_s2h_session_info_per_direction;
        session_s2hinfo->epoch1_value = 
            spec->data.switch_to_host_flow_info.epoch1;
        session_s2hinfo->epoch1_id = 
            spec->data.switch_to_host_flow_info.epoch1_id;
        session_s2hinfo->epoch2_value = 
            spec->data.switch_to_host_flow_info.epoch2;
        session_s2hinfo->epoch2_id = 
            spec->data.switch_to_host_flow_info.epoch2_id;
        session_s2hinfo->allowed_flow_state_bitmask = 
            spec->data.switch_to_host_flow_info.allowed_flow_state_bitmask;
        session_s2hinfo->egress_action = 
            spec->data.switch_to_host_flow_info.egress_action;
        session_s2hinfo->strip_outer_encap_flag = 
            spec->data.switch_to_host_flow_info.rewrite_info.strip_encap_header;
        session_s2hinfo->strip_l2_header_flag = 
            spec->data.switch_to_host_flow_info.rewrite_info.strip_l2_header;
        session_s2hinfo->strip_vlan_tag_flag = 
            spec->data.switch_to_host_flow_info.rewrite_info.strip_vlan_tag;
        session_s2hinfo->nat_type = 
            spec->data.switch_to_host_flow_info.rewrite_info.user_packet_rewrite_type;
        memcpy(session_s2hinfo->nat_address,
               spec->data.switch_to_host_flow_info.rewrite_info.user_packet_rewrite_ip,
               IP6_ADDR8_LEN);
        session_s2hinfo->encap_type = 
            spec->data.switch_to_host_flow_info.rewrite_info.encap_type;
        if (session_s2hinfo->encap_type == ENCAP_TYPE_L2) {
            session_s2hinfo->add_vlan_tag_flag =
                spec->data.switch_to_host_flow_info.rewrite_info.u.l2_encap.insert_vlan_tag;
            session_s2hinfo->vlan =
                spec->data.switch_to_host_flow_info.rewrite_info.u.l2_encap.vlan_id;
            memcpy(session_s2hinfo->dmac,
                   spec->data.switch_to_host_flow_info.rewrite_info.u.l2_encap.dmac,
                   ETH_ADDR_LEN);
            memcpy(session_s2hinfo->smac,
                   spec->data.switch_to_host_flow_info.rewrite_info.u.l2_encap.smac,
                   ETH_ADDR_LEN);
        } else if (session_s2hinfo->encap_type == ENCAP_TYPE_MPLSOUDP) {
            session_s2hinfo->add_vlan_tag_flag =
                spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.insert_vlan_tag;
            session_s2hinfo->vlan =
                spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.vlan_id;
            memcpy(session_s2hinfo->dmac,
                   spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.dmac,
                   ETH_ADDR_LEN);
            memcpy(session_s2hinfo->smac,
                   spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.l2_encap.smac,
                   ETH_ADDR_LEN);
            session_s2hinfo->ipv4_sa = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_saddr;
            session_s2hinfo->ipv4_da = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.ip_encap.ip_daddr;
            session_s2hinfo->udp_sport = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_sport;
            session_s2hinfo->udp_dport = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.udp_encap.udp_dport;
            session_s2hinfo->mpls_label1 = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.mpls1_label;
            session_s2hinfo->mpls_label2 = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.mpls2_label;
            session_s2hinfo->mpls_label3 = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsoudp_encap.mpls3_label;
        } else if (session_s2hinfo->encap_type == ENCAP_TYPE_MPLSOGRE) {
            session_s2hinfo->add_vlan_tag_flag =
                spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.insert_vlan_tag;
            session_s2hinfo->vlan =
                spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.vlan_id;
            memcpy(session_s2hinfo->dmac,
                   spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.dmac,
                   ETH_ADDR_LEN);
            memcpy(session_s2hinfo->smac,
                   spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.l2_encap.smac,
                   ETH_ADDR_LEN);
            session_s2hinfo->ipv4_sa = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.ip_encap.ip_saddr;
            session_s2hinfo->ipv4_da = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.ip_encap.ip_daddr;
            session_s2hinfo->mpls_label1 = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.mpls1_label;
            session_s2hinfo->mpls_label2 = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.mpls2_label;
            session_s2hinfo->mpls_label3 = spec->data.switch_to_host_flow_info.rewrite_info.u.mplsogre_encap.mpls3_label;
        } else if (session_s2hinfo->encap_type != ENCAP_TYPE_NONE) {
            PDS_TRACE_ERR("Unknown encap type");
            return SDK_RET_INVALID_ARG;
        }
        session_s2hinfo->valid_flag = 1;
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_S2H,
                                           session_info_id, NULL, NULL,
                                           &session_s2h_actiondata);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to write session s2h table at index %u",
                           session_info_id);
            return SDK_RET_HW_PROGRAM_ERR;
        }
    }
#endif
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
#if 0
    p4pd_error_t p4pd_ret;
    uint32_t session_info_id;
    session_info_common_actiondata_t session_common_actiondata = { 0 };
    session_info_common_actiondata_t rd_session_common_actiondata = { 0 };
    session_info_common_session_info_common_t *session_cinfo;
    session_info_h2s_actiondata_t session_h2s_actiondata = { 0 };
    session_info_s2h_actiondata_t session_s2h_actiondata = { 0 };

    if (!key) {
        PDS_TRACE_ERR("key is null");
        return SDK_RET_INVALID_ARG;
    }
    session_info_id = key->session_info_id;
    if (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return SDK_RET_INVALID_ARG;
    }
    if (!((key->direction & HOST_TO_SWITCH) ||
        (key->direction & SWITCH_TO_HOST)))
        return SDK_RET_INVALID_ARG;

    // Check if there is an entry already at the index
    if (p4pd_global_entry_read(P4TBL_ID_SESSION_INFO_COMMON, session_info_id,
                               NULL, NULL, &rd_session_common_actiondata) !=
                               P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read session common table at index %u",
                      session_info_id);
        return SDK_RET_HW_READ_ERR;
    }
    session_cinfo =
        &rd_session_common_actiondata.action_u.session_info_common_session_info_common;
    if (!session_cinfo->valid_flag)
        return SDK_RET_ENTRY_NOT_FOUND;

    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_COMMON,
                                       session_info_id, NULL, NULL,
                                       &session_common_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to delete session common table at index %u",
                      session_info_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_H2S,
                                       session_info_id, NULL, NULL,
                                       &session_h2s_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to delete session h2s table at index %u",
                      session_info_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_S2H,
                                       session_info_id, NULL, NULL,
                                       &session_s2h_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to delete session s2h table at index %u",
                      session_info_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }
#endif
    return SDK_RET_OK;
}

}
