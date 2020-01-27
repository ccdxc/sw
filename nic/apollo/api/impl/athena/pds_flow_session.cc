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

sdk_ret_t
pds_flow_session_info_create (pds_flow_session_spec_t *spec)
{
    p4pd_error_t p4pd_ret;
    uint32_t session_info_id;
#if 0 /* Commenting out for now. To be refactored for new table definition */
    session_info_actiondata_t session_actiondata = { 0 };
    session_info_session_info_t *session_info;
    session_info_rewrite_actiondata_t rewrite_actiondata = { 0 };
    session_info_rewrite_session_info_rewrite_t *rewrite_info;

    if (!spec) {
        PDS_TRACE_ERR("spec is null");
        return SDK_RET_INVALID_ARG;
    }
    session_info_id = spec->key.session_info_id;
    if (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return SDK_RET_INVALID_ARG;
    }

    session_actiondata.action_id = SESSION_INFO_SESSION_INFO_ID;
    session_info = &session_actiondata.action_u.session_info_session_info;
    // TODO: Check this
    //memcpy(session_info->timestamp, spec->data.timestamp, 8);
    session_info->config1_epoch = spec->data.epoch1;
    session_info->config1_idx = spec->data.epoch1_id;
    session_info->config2_epoch = spec->data.epoch2;
    session_info->config2_idx = spec->data.epoch2_id;
    session_info->config_substrate_src_ip = 
                  spec->data.config_substrate_src_ip;
    session_info->throttle_pps = spec->data.policer_pps_id;
    session_info->throttle_bw = spec->data.policer_bw_id;
    session_info->counterset1 = spec->data.counter_set1_id;
    session_info->counterset2 = spec->data.counter_set2_id;
    session_info->histogram = spec->data.histogram_id;
    session_info->pop_hdr_flag = spec->data.rewrite_info.pop_outer_header;
    session_info->valid_flag = 1;
    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO,
                                       session_info_id, NULL, NULL,
                                       &session_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to write session table at index %u",
                       session_info_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    // TODO : Check if is this possible?
    if (spec->data.rewrite_info.encap_type == ENCAP_TYPE_NONE) {
        return SDK_RET_OK;
    }
    // Program the other rewrite tables
    rewrite_actiondata.action_id =
        SESSION_INFO_REWRITE_SESSION_INFO_REWRITE_ID;
    rewrite_info = 
        &rewrite_actiondata.action_u.session_info_rewrite_session_info_rewrite;
    rewrite_info->user_pkt_rewrite_type = 
        (uint8_t)spec->data.rewrite_info.user_packet_rewrite_type;
    memcpy(rewrite_info->user_pkt_rewrite_ip,
           spec->data.rewrite_info.user_packet_rewrite_ip, INET6_ADDRSTRLEN);
    rewrite_info->encap_type = (uint8_t)spec->data.rewrite_info.encap_type;
    if (spec->data.rewrite_info.encap_type == ENCAP_TYPE_L2) {
        memcpy(rewrite_info->smac, spec->data.rewrite_info.u.l2_encap.smac,
               ETH_ADDR_LEN);
        memcpy(rewrite_info->dmac, spec->data.rewrite_info.u.l2_encap.dmac,
               ETH_ADDR_LEN);
        rewrite_info->vlan = spec->data.rewrite_info.u.l2_encap.vlan_id;
        rewrite_info->valid_flag = 1;
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_REWRITE,
                                           session_info_id, NULL, NULL,
                                           &rewrite_actiondata);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to write session rewrite table at index %u",
                          session_info_id);
            return SDK_RET_HW_PROGRAM_ERR;
        }
    } else if (spec->data.rewrite_info.encap_type == ENCAP_TYPE_MPLSOUDP) {
        pds_flow_session_mplsoudp_encap_t *mplsoudp_encap =
            &spec->data.rewrite_info.u.mplsoudp_encap;
        memcpy(rewrite_info->smac, mplsoudp_encap->l2_encap.smac,
               ETH_ADDR_LEN);
        memcpy(rewrite_info->dmac, mplsoudp_encap->l2_encap.dmac,
               ETH_ADDR_LEN);
        rewrite_info->vlan = mplsoudp_encap->l2_encap.vlan_id;
        rewrite_info->ip_ttl = mplsoudp_encap->ip_encap.ip_ttl;
        rewrite_info->ip_saddr = mplsoudp_encap->ip_encap.ip_saddr;
        rewrite_info->ip_daddr = mplsoudp_encap->ip_encap.ip_daddr;
        rewrite_info->udp_sport = mplsoudp_encap->udp_encap.udp_sport;
        rewrite_info->udp_dport = mplsoudp_encap->udp_encap.udp_dport;
        rewrite_info->mpls1_label = mplsoudp_encap->mpls1_label;
        rewrite_info->mpls2_label = mplsoudp_encap->mpls2_label;
        rewrite_info->valid_flag = 1;
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_REWRITE,
                                           session_info_id, NULL, NULL,
                                           &rewrite_actiondata);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to write session rewrite table at index %u",
                          session_info_id);
            return SDK_RET_HW_PROGRAM_ERR;
        }
    } else if (spec->data.rewrite_info.encap_type == ENCAP_TYPE_MPLSOGRE) {
        pds_flow_session_mplsogre_encap_t *mplsogre_encap =
            &spec->data.rewrite_info.u.mplsogre_encap;
        memcpy(rewrite_info->smac, mplsogre_encap->l2_encap.smac,
               ETH_ADDR_LEN);
        memcpy(rewrite_info->dmac, mplsogre_encap->l2_encap.dmac,
               ETH_ADDR_LEN);
        rewrite_info->vlan = mplsogre_encap->l2_encap.vlan_id;
        rewrite_info->ip_ttl = mplsogre_encap->ip_encap.ip_ttl;
        rewrite_info->ip_saddr = mplsogre_encap->ip_encap.ip_saddr;
        rewrite_info->ip_daddr = mplsogre_encap->ip_encap.ip_daddr;
        rewrite_info->mpls1_label = mplsogre_encap->mpls1_label;
        rewrite_info->mpls2_label = mplsogre_encap->mpls2_label;
        rewrite_info->valid_flag = 1;
        p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_REWRITE,
                                           session_info_id, NULL, NULL,
                                           &rewrite_actiondata);
        if (p4pd_ret != P4PD_SUCCESS) {
            PDS_TRACE_ERR("Failed to write session rewrite table at index %u",
                          session_info_id);
            return SDK_RET_HW_PROGRAM_ERR;
        }
    } else {
        PDS_TRACE_ERR("Unknown encap type");
        return SDK_RET_INVALID_ARG;
    }
#endif
    return SDK_RET_OK;
}

sdk_ret_t
pds_flow_session_info_read (pds_flow_session_key_t *key,
                            pds_flow_session_info_t *info)
{
    p4pd_error_t p4pd_ret;
    uint32_t session_info_id;
#if 0 /* Commenting out for now. To be refactored for new table definition */
    session_info_actiondata_t session_actiondata = { 0 };
    session_info_session_info_t *session_info;
    session_info_rewrite_actiondata_t rewrite_actiondata = { 0 };
    session_info_rewrite_session_info_rewrite_t *rewrite_info;

    if (!key || !info) {
        PDS_TRACE_ERR("key or info is null");
        return SDK_RET_INVALID_ARG;
    }
    session_info_id = key->session_info_id;
    if (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return SDK_RET_INVALID_ARG;
    }

    if (p4pd_global_entry_read(P4TBL_ID_SESSION_INFO, session_info_id,
                               NULL, NULL, &session_actiondata) !=
                               P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to read session table at index %u",
                      session_info_id);
        return SDK_RET_HW_READ_ERR;
    }
    session_info = &session_actiondata.action_u.session_info_session_info;
    if (!session_info->valid_flag)
        return SDK_RET_OK;
    // TODO: Check this
    //memcpy(info->spec.data.timestamp, session_info->timestamp, 8);
    info->spec.data.epoch1 = session_info->config1_epoch;
    info->spec.data.epoch1_id = session_info->config1_idx;
    info->spec.data.epoch2 = session_info->config2_epoch;
    info->spec.data.epoch2_id = session_info->config2_idx;
    info->spec.data.config_substrate_src_ip =
        session_info->config_substrate_src_ip;
    info->spec.data.policer_pps_id = session_info->throttle_pps;
    info->spec.data.policer_bw_id = session_info->throttle_bw;
    info->spec.data.counter_set1_id = session_info->counterset1;
    info->spec.data.counter_set2_id = session_info->counterset2;
    info->spec.data.histogram_id = session_info->histogram;
    info->spec.data.rewrite_info.pop_outer_header = session_info->pop_hdr_flag;

    if (p4pd_global_entry_read(P4TBL_ID_SESSION_INFO_REWRITE,
                               session_info_id, NULL, NULL,
                               &rewrite_actiondata) == P4PD_SUCCESS) {
        rewrite_info = 
            &rewrite_actiondata.action_u.session_info_rewrite_session_info_rewrite;
        // No encap case
        if (!rewrite_info->valid_flag)
            return SDK_RET_OK;
        info->spec.data.rewrite_info.user_packet_rewrite_type =
            (pds_user_packet_rewrite_type_t)rewrite_info->user_pkt_rewrite_type;
        memcpy(info->spec.data.rewrite_info.user_packet_rewrite_ip,
            rewrite_info->user_pkt_rewrite_ip, INET6_ADDRSTRLEN);
        info->spec.data.rewrite_info.encap_type =
            (pds_encap_type_t)rewrite_info->encap_type;
    } else {
        return SDK_RET_HW_READ_ERR;
    }

    if (info->spec.data.rewrite_info.encap_type == ENCAP_TYPE_L2) {
        memcpy(info->spec.data.rewrite_info.u.l2_encap.smac,
               rewrite_info->smac, ETH_ADDR_LEN);
        memcpy(info->spec.data.rewrite_info.u.l2_encap.dmac,
               rewrite_info->dmac, ETH_ADDR_LEN);
        info->spec.data.rewrite_info.u.l2_encap.vlan_id = rewrite_info->vlan;
    } else if (info->spec.data.rewrite_info.encap_type == ENCAP_TYPE_MPLSOUDP) {
        pds_flow_session_mplsoudp_encap_t *mplsoudp_encap =
            &info->spec.data.rewrite_info.u.mplsoudp_encap;
        memcpy(mplsoudp_encap->l2_encap.smac, rewrite_info->smac,
               ETH_ADDR_LEN);
        memcpy(mplsoudp_encap->l2_encap.dmac, rewrite_info->dmac,
               ETH_ADDR_LEN);
        mplsoudp_encap->l2_encap.vlan_id = rewrite_info->vlan;
        mplsoudp_encap->ip_encap.ip_ttl = rewrite_info->ip_ttl;
        mplsoudp_encap->ip_encap.ip_saddr = rewrite_info->ip_saddr;
        mplsoudp_encap->ip_encap.ip_daddr = rewrite_info->ip_daddr;
        mplsoudp_encap->udp_encap.udp_sport = rewrite_info->udp_sport;
        mplsoudp_encap->udp_encap.udp_dport = rewrite_info->udp_dport;
        mplsoudp_encap->mpls1_label = rewrite_info->mpls1_label;
        mplsoudp_encap->mpls2_label = rewrite_info->mpls2_label;
    } else if (info->spec.data.rewrite_info.encap_type == ENCAP_TYPE_MPLSOGRE) {
        pds_flow_session_mplsogre_encap_t *mplsogre_encap =
            &info->spec.data.rewrite_info.u.mplsogre_encap;
        memcpy(mplsogre_encap->l2_encap.smac, rewrite_info->smac,
               ETH_ADDR_LEN);
        memcpy(mplsogre_encap->l2_encap.dmac, rewrite_info->dmac,
               ETH_ADDR_LEN);
        mplsogre_encap->l2_encap.vlan_id = rewrite_info->vlan;
        mplsogre_encap->ip_encap.ip_ttl = rewrite_info->ip_ttl;
        mplsogre_encap->ip_encap.ip_saddr = rewrite_info->ip_saddr;
        mplsogre_encap->ip_encap.ip_daddr = rewrite_info->ip_daddr;
        mplsogre_encap->mpls1_label = rewrite_info->mpls1_label;
        mplsogre_encap->mpls2_label = rewrite_info->mpls2_label;
   }
#endif
   return SDK_RET_OK;
}

sdk_ret_t
pds_flow_session_info_update (pds_flow_session_spec_t *spec)
{
    return pds_flow_session_info_create(spec);
}

sdk_ret_t
pds_flow_session_info_delete (pds_flow_session_key_t *key)
{
    p4pd_error_t p4pd_ret;
    uint32_t session_info_id;
#if 0 /* Commenting out for now. To be refactored for new table definition */
    session_info_actiondata_t session_actiondata = { 0 };
    session_info_rewrite_actiondata_t rewrite_actiondata = { 0 };

    if (!key) {
        PDS_TRACE_ERR("key is null");
        return SDK_RET_INVALID_ARG;
    }
    session_info_id = key->session_info_id;
    if (session_info_id >= PDS_FLOW_SESSION_INFO_ID_MAX) {
        PDS_TRACE_ERR("session id %u is invalid", session_info_id);
        return SDK_RET_INVALID_ARG;
    }

    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO,
                                       session_info_id, NULL, NULL,
                                       &session_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to delete session table at index %u",
                      session_info_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }

    p4pd_ret = p4pd_global_entry_write(P4TBL_ID_SESSION_INFO_REWRITE,
                                       session_info_id, NULL, NULL,
                                       &rewrite_actiondata);
    if (p4pd_ret != P4PD_SUCCESS) {
        PDS_TRACE_ERR("Failed to delete session rewrite table at index %u",
                      session_info_id);
        return SDK_RET_HW_PROGRAM_ERR;
    }
#endif
    return SDK_RET_OK;
}

}
