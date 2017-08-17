#include <time.h>
#include <base.h>
#include <eth.h>
#include <timestamp.h>
#include <hal_lock.hpp>
#include <p4pd.h>
#include <pd_api.hpp>
#include <if_pd_utils.hpp>
#include <l2seg_pd.hpp>
#include <session_pd.hpp>
#include <interface.hpp>
#include <endpoint.hpp>
#include <endpoint_api.hpp>
#include <endpoint_pd.hpp>
#include <hal_state_pd.hpp>
#include <defines.h>
#include <string.h>


namespace hal {
namespace pd {

//------------------------------------------------------------------------------
// program flow stats table entry and return the index at which
// the entry is programmed
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_flow_stats_table_entry (uint32_t *flow_stats_idx)
{
    hal_ret_t                ret;
    DirectMap                *dm;
    flow_stats_actiondata    d = { 0 };

    HAL_ASSERT(flow_stats_idx != NULL);
    dm = g_hal_state_pd->dm_table(P4TBL_ID_FLOW_STATS);
    HAL_ASSERT(dm != NULL);


    d.actionid = FLOW_STATS_FLOW_STATS_ID;
    // insert the entry
    ret = dm->insert(&d, flow_stats_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow stats table write failure, err : {}", ret);
        return ret;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// delete flow stats table entry at given index
//------------------------------------------------------------------------------
hal_ret_t
p4pd_del_flow_stats_table_entry (uint32_t flow_stats_idx)
{
    DirectMap                *dm;

    // 0th entry is reserved
    if (!flow_stats_idx) {
        return HAL_RET_INVALID_ARG;
    }

    dm = g_hal_state_pd->dm_table(P4TBL_ID_FLOW_STATS);
    HAL_ASSERT(dm != NULL);

    return dm->remove(flow_stats_idx);
}

//------------------------------------------------------------------------------
// program flow stats table entries for a given session
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_flow_stats_table_entries (pd_session_t *session_pd)
{
    hal_ret_t    ret;

    // program flow_stats table entry for iflow
    ret = p4pd_add_flow_stats_table_entry(&session_pd->iflow.flow_stats_hw_id);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // program flow_stats table entry for rflow
    if (session_pd->rflow_valid) {
        ret = p4pd_add_flow_stats_table_entry(&session_pd->rflow.flow_stats_hw_id);
        if (ret != HAL_RET_OK) {
            p4pd_del_flow_stats_table_entry(session_pd->iflow.flow_stats_hw_id);
            return ret;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// delete flow stats table entries for a given session
//------------------------------------------------------------------------------
hal_ret_t
p4pd_del_flow_stats_table_entries (pd_session_t *session_pd)
{
    hal_ret_t    ret;

    ret = p4pd_del_flow_stats_table_entry(session_pd->iflow.flow_stats_hw_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("iflow flow stats table entry delete failed, err : {}",
                      ret);
    }
    if (session_pd->rflow_valid) {
        ret =
            p4pd_del_flow_stats_table_entry(session_pd->rflow.flow_stats_hw_id);
            HAL_TRACE_ERR("iflow flow stats table entry delete failed, "
                          "err : {}", ret);
    }

    return ret;
}

//------------------------------------------------------------------------------
// program flow state table entry and return the index at which
// the entry is programmed
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_session_state_table_entry (pd_session_t *session_pd,
                                    session_state_t *session_state,
                                    nwsec_profile_t *nwsec_profile)
{
    hal_ret_t                ret;
    DirectMap                *dm;
    flow_t                   *iflow, *rflow;
    session_state_actiondata    d = { 0 };
    session_t                *session = (session_t *)session_pd->session;

    HAL_ASSERT(session_pd != NULL);
    iflow = session->iflow;
    rflow = session->rflow;
    HAL_ASSERT((iflow != NULL) && (rflow != NULL));

    dm = g_hal_state_pd->dm_table(P4TBL_ID_SESSION_STATE);
    HAL_ASSERT(dm != NULL);

    // populate the action information
    d.actionid = SESSION_STATE_TCP_SESSION_STATE_INFO_ID;

    d.session_state_action_u.session_state_tcp_session_state_info.iflow_tcp_state =
        iflow->config.state;
    d.session_state_action_u.session_state_tcp_session_state_info.rflow_tcp_state =
        rflow->config.state;

    if (session_state) {
        // initiator flow specific information
        d.session_state_action_u.session_state_tcp_session_state_info.iflow_tcp_seq_num =
            session_state->iflow_state.tcp_seq_num;
        d.session_state_action_u.session_state_tcp_session_state_info.iflow_tcp_ack_num =
            session_state->iflow_state.tcp_ack_num;
        d.session_state_action_u.session_state_tcp_session_state_info.iflow_tcp_win_sz =
            session_state->iflow_state.tcp_win_sz;
        d.session_state_action_u.session_state_tcp_session_state_info.iflow_tcp_win_scale =
            session_state->iflow_state.tcp_win_scale;
        d.session_state_action_u.session_state_tcp_session_state_info.iflow_tcp_mss =
            session_state->iflow_state.tcp_mss;
        d.session_state_action_u.session_state_tcp_session_state_info.iflow_exceptions_seen =
            session_state->iflow_state.exception_bmap;

        // responder flow specific information
        d.session_state_action_u.session_state_tcp_session_state_info.rflow_tcp_seq_num =
            session_state->rflow_state.tcp_seq_num;
        d.session_state_action_u.session_state_tcp_session_state_info.rflow_tcp_ack_num =
            session_state->rflow_state.tcp_ack_num;
        d.session_state_action_u.session_state_tcp_session_state_info.rflow_tcp_win_sz =
            session_state->rflow_state.tcp_win_sz;
        d.session_state_action_u.session_state_tcp_session_state_info.rflow_tcp_win_scale =
            session_state->rflow_state.tcp_win_scale;
        d.session_state_action_u.session_state_tcp_session_state_info.rflow_tcp_mss =
            session_state->rflow_state.tcp_mss;
        d.session_state_action_u.session_state_tcp_session_state_info.rflow_exceptions_seen =
            session_state->rflow_state.exception_bmap;
    }
    

    // session level information
    d.session_state_action_u.session_state_tcp_session_state_info.syn_cookie_delta =
        session->config.syn_ack_delta;
    d.session_state_action_u.session_state_tcp_session_state_info.flow_rtt_seq_check_enabled =
        nwsec_profile ?  nwsec_profile->tcp_rtt_estimate_en : FALSE;

    // insert the entry
    ret = dm->insert(&d, &session_pd->session_state_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow state table write failure, err : {}", ret);
        return ret;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// delete flow state table entry at given index
//------------------------------------------------------------------------------
hal_ret_t
p4pd_del_session_state_table_entry (uint32_t session_state_idx)
{
    DirectMap                *dm;

    // 0th entry is reserved
    if (!session_state_idx) {
        return HAL_RET_INVALID_ARG;
    }

    dm = g_hal_state_pd->dm_table(P4TBL_ID_SESSION_STATE);
    HAL_ASSERT(dm != NULL);

    return dm->remove(session_state_idx);
}
hal_ret_t
session_pd_fill_queue_info(ep_t *dst_ep,
                           intf::LifQType qtype,
                           uint8_t *qid_en,
                           uint8_t *q_off,
                           uint32_t *qid)
{
    hal_ret_t       ret = HAL_RET_OK;
    intf::IfType    if_type = intf::IF_TYPE_NONE;
    if_t            *pi_if;

    if_type = ep_pd_get_if_type(dst_ep);

    *qid_en = *q_off = *qid = 0;
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            pi_if = ep_find_if_by_handle(dst_ep);
            HAL_ASSERT(pi_if != NULL);

            ret = if_get_qid_qoff(pi_if, qtype, q_off, qid);
            if (ret == HAL_RET_OK) {
               *qid_en = 1; 
            }
            break;
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            break;
        case intf::IF_TYPE_TUNNEL:
            break;
        default:
            HAL_ASSERT(0);
    }

    return ret;
}
//------------------------------------------------------------------------------
// program flow info table entry at either given index or if given index is 0
// allocate an index and return that
// TODO:
// 1. flow_ttl is set to DEF_TTL (64) currently ... if we have packet in hand we
//    should take it from there or else default is fine
// 2. twice nat not supported
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_flow_info_table_entry (tenant_t *tenant, session_t *session,
                                l2seg_t *l2seg_s, l2seg_t *l2seg_d,
                                nwsec_profile_t *nwsec_profile,
                                flow_cfg_t *flow, pd_flow_t *flow_pd,
                                if_t *sif, if_t *dif, ep_t *sep, ep_t *dep,
                                bool mcast)
{
    hal_ret_t                ret;
    DirectMap                *dm;
    flow_info_actiondata     d = { 0};
    timespec_t               ts;

    HAL_ASSERT(dep != NULL);
    dm = g_hal_state_pd->dm_table(P4TBL_ID_FLOW_INFO);
    HAL_ASSERT(dm != NULL);

    // populate the action information
    d.actionid = FLOW_INFO_FLOW_INFO_ID;

    if (!mcast) {
        d.flow_info_action_u.flow_info_flow_info.lif =
            ep_pd_get_hw_lif_id(dep);
    } else {
    }

    d.flow_info_action_u.flow_info_flow_info.multicast_en = mcast;
    // TBD: where do these come from ?
    d.flow_info_action_u.flow_info_flow_info.p4plus_app_id = 0;
    d.flow_info_action_u.flow_info_flow_info.flow_steering_only = 0;
    // TBD: the following come when QoS model is defined
    d.flow_info_action_u.flow_info_flow_info.ingress_policer_index = 0;
    d.flow_info_action_u.flow_info_flow_info.egress_policer_index = 0;
    // TBD:  populate these once SPAN policy is defined
    d.flow_info_action_u.flow_info_flow_info.ingress_mirror_session_id = 0;
    d.flow_info_action_u.flow_info_flow_info.egress_mirror_session_id = 0;

    // TODO: Sarat: We may have to pass the right action id.
    //d.flow_info_action_u.flow_info_flow_info.rewrite_index =
        //ep_get_rewrite_index(dst_ep);
        //ep_pd_get_rw_tbl_idx_from_pi_ep(dst_ep, REWRITE_L3_REWRITE_ID);

    // there is no transit case for us, so this is always FALSE
    if (is_if_type_tunnel(dif) && (sif->if_type != dif->if_type)) {
        d.flow_info_action_u.flow_info_flow_info.tunnel_originate = TRUE;
    } else {
        d.flow_info_action_u.flow_info_flow_info.tunnel_originate = FALSE;
    }

    switch (flow->nat_type) {
    case NAT_TYPE_SNAT:
        memcpy(d.flow_info_action_u.flow_info_flow_info.nat_ip, &flow->nat_sip,
               sizeof(ipvx_addr_t));
        d.flow_info_action_u.flow_info_flow_info.nat_l4_port = flow->nat_sport;
        break;

    case NAT_TYPE_DNAT:
        memcpy(d.flow_info_action_u.flow_info_flow_info.nat_ip, &flow->nat_dip,
               sizeof(ipvx_addr_t));
        d.flow_info_action_u.flow_info_flow_info.nat_l4_port = flow->nat_dport;
        break;

    case NAT_TYPE_TWICE_NAT:
        if (flow->role == FLOW_ROLE_INITIATOR) {
            memcpy(d.flow_info_action_u.flow_info_flow_info.nat_ip, &flow->nat_sip,
                   sizeof(ipvx_addr_t));
            d.flow_info_action_u.flow_info_flow_info.nat_l4_port = flow->nat_sport;
            d.flow_info_action_u.flow_info_flow_info.twice_nat_idx = 0;
        } else {
            memcpy(d.flow_info_action_u.flow_info_flow_info.nat_ip, &flow->nat_dip,
                   sizeof(ipvx_addr_t));
            d.flow_info_action_u.flow_info_flow_info.nat_l4_port = flow->nat_dport;
            d.flow_info_action_u.flow_info_flow_info.twice_nat_idx = 0;
        }
        break;

    default:
        break;
    }

    // TBD: the following come when QoS model is defined
    d.flow_info_action_u.flow_info_flow_info.cos_en = 0;
    d.flow_info_action_u.flow_info_flow_info.cos = 0;
    d.flow_info_action_u.flow_info_flow_info.dscp_en = 0;
    d.flow_info_action_u.flow_info_flow_info.dscp = 0;

    // TBD: check class NIC mode and set this
    d.flow_info_action_u.flow_info_flow_info.qid_en = FALSE;
    session_pd_fill_queue_info(dep,
                               flow->lif_qtype,
                               &d.flow_info_action_u.flow_info_flow_info.qid_en,
                               &d.flow_info_action_u.flow_info_flow_info.qtype,
                               &d.flow_info_action_u.flow_info_flow_info.tunnel_vnid);


    // TBD: check analytics policy and set this
    d.flow_info_action_u.flow_info_flow_info.log_en = FALSE;

    d.flow_info_action_u.flow_info_flow_info.mac_sa_rewrite = flow->mac_sa_rewrite;
    d.flow_info_action_u.flow_info_flow_info.mac_da_rewrite = flow->mac_da_rewrite;
    d.flow_info_action_u.flow_info_flow_info.ttl_dec = flow->ttl_dec;
    d.flow_info_action_u.flow_info_flow_info.rewrite_index = ep_pd_get_rw_tbl_idx(dep->pd, 
            flow->rw_act);
    d.flow_info_action_u.flow_info_flow_info.tunnel_vnid = flow->tnnl_vnid;
    d.flow_info_action_u.flow_info_flow_info.tunnel_rewrite_index = ep_pd_get_tnnl_rw_tbl_idx(dep->pd,
            flow->tnnl_rw_act);

    // TODO: if we are doing routing, then set ttl_dec to TRUE
    d.flow_info_action_u.flow_info_flow_info.flow_conn_track = session->config.conn_track_en;
    d.flow_info_action_u.flow_info_flow_info.flow_ttl = 64;
    d.flow_info_action_u.flow_info_flow_info.flow_role = flow->role;
    d.flow_info_action_u.flow_info_flow_info.session_state_index =
        session->config.conn_track_en ? flow_pd->session_state_hw_id : 0;
    clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    d.flow_info_action_u.flow_info_flow_info.start_timestamp = ts.tv_sec;

    // insert the entry
    ret = dm->insert_withid(&d, flow_pd->flow_stats_hw_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow state table write failure, idx : {}, err : {}",
                      flow_pd->flow_stats_hw_id, ret);
        return ret;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// delete flow info table entry at given index
//------------------------------------------------------------------------------
hal_ret_t
p4pd_del_flow_info_table_entry (uint32_t index)
{
    DirectMap                *dm;

    // 0th entry is reserved
    if (!index) {
        return HAL_RET_INVALID_ARG;
    }

    dm = g_hal_state_pd->dm_table(P4TBL_ID_FLOW_INFO);
    HAL_ASSERT(dm != NULL);

    return dm->remove(index);
}

//------------------------------------------------------------------------------
// delete flow info table entries for a given session
//------------------------------------------------------------------------------
hal_ret_t
p4pd_del_flow_info_table_entries (pd_session_t *session_pd)
{
    hal_ret_t    ret;

    ret = p4pd_del_flow_info_table_entry(session_pd->iflow.flow_stats_hw_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("iflow flow info table entry delete failed, "
                      "idx : {}, err : {}",
                      session_pd->iflow.flow_stats_hw_id, ret);
    }
    if (session_pd->rflow_valid) {
        ret =
            p4pd_del_flow_info_table_entry(session_pd->rflow.flow_stats_hw_id);
            HAL_TRACE_ERR("iflow flow info table entry delete failed, "
                          "idx : {}, err : {}",
                          session_pd->rflow.flow_stats_hw_id, ret);
    }

    return ret;
}

//------------------------------------------------------------------------------
// program flow info table entries for a given session
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_flow_info_table_entries (pd_session_args_t *args)
{
    hal_ret_t       ret;
    pd_session_t    *session_pd = (pd_session_t *)args->session->pd;

    // program flow_info table entry for iflow
    ret = p4pd_add_flow_info_table_entry(args->tenant, args->session,
                                         args->l2seg_s, args->l2seg_d,
                                         args->nwsec_prof,
                                         &args->session->iflow->config,
                                         &session_pd->iflow,
                                         args->sif, args->dif,
                                         args->sep, args->dep,
                                         false);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // program flow_info table entry for rflow
    if (session_pd->rflow_valid) {
        ret = p4pd_add_flow_info_table_entry(args->tenant, args->session,
                                             args->l2seg_d, args->l2seg_s,
                                             args->nwsec_prof,
                                             &args->session->rflow->config,
                                             &session_pd->rflow,
                                             args->dif, args->sif,
                                             args->dep, args->sep,
                                             false);
        if (ret != HAL_RET_OK) {
            p4pd_del_flow_info_table_entry(session_pd->iflow.flow_stats_hw_id);
            return ret;
        }
    }

    return ret;
}

uint8_t *memrev(uint8_t *block, size_t elnum)
{
     uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
     return block;
}

//------------------------------------------------------------------------------
// program flow hash table entry for a given flow
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_flow_hash_table_entry (flow_cfg_t *flow, pd_l2seg_t *l2seg_pd,
                                pd_flow_t *flow_pd)
{
    hal_ret_t                ret;
    flow_hash_swkey_t        key = { 0 };
    p4pd_flow_hash_data_t    flow_data;
    fmt::MemoryWriter src_buf, dst_buf;

    // initialize all the key fields of flow
    if (flow->key.flow_type == FLOW_TYPE_V4 || flow->key.flow_type == FLOW_TYPE_V6) {
        memcpy(key.flow_lkp_metadata_lkp_src, flow->key.sip.v6_addr.addr8,
                IP6_ADDR8_LEN);
        memcpy(key.flow_lkp_metadata_lkp_dst, flow->key.dip.v6_addr.addr8,
                IP6_ADDR8_LEN);
    } else {
        memcpy(key.flow_lkp_metadata_lkp_src, flow->key.smac, sizeof(flow->key.smac)); 
        memcpy(key.flow_lkp_metadata_lkp_dst, flow->key.dmac, sizeof(flow->key.dmac)); 
    }

    if (flow->key.flow_type == FLOW_TYPE_V6) {
        memrev(key.flow_lkp_metadata_lkp_src, sizeof(key.flow_lkp_metadata_lkp_src));
        memrev(key.flow_lkp_metadata_lkp_dst, sizeof(key.flow_lkp_metadata_lkp_dst));
    }

    if (flow->key.flow_type == FLOW_TYPE_V4 || flow->key.flow_type == FLOW_TYPE_V6) {
        if ((flow->key.proto == IP_PROTO_TCP) || (flow->key.proto == IP_PROTO_UDP)) {
            key.flow_lkp_metadata_lkp_sport = flow->key.sport;
            key.flow_lkp_metadata_lkp_dport = flow->key.dport;
        } else if ((flow->key.proto == IP_PROTO_ICMP) ||
                (flow->key.proto == IP_PROTO_ICMPV6)) {
            // Revisit: Swapped sport and dport. This is what matches what 
            //          is coded in P4. 
            key.flow_lkp_metadata_lkp_dport = flow->key.icmp_id;
            key.flow_lkp_metadata_lkp_sport =
                ((flow->key.icmp_type << 8) | flow->key.icmp_code);
        }
    } else {
        // For FLOW_TYPE_L2
        key.flow_lkp_metadata_lkp_sport = flow->key.ether_type;
    }
    if (flow->key.flow_type == FLOW_TYPE_L2) {
        key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_MAC;
    } else if (flow->key.flow_type == FLOW_TYPE_V4) {
        key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV4;
    } else if (flow->key.flow_type == FLOW_TYPE_V6) {
        key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV6;
    } else {
        // TODO: for now !!
        key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_NONE;
    }
    key.flow_lkp_metadata_lkp_vrf = l2seg_pd->hw_id;
    key.flow_lkp_metadata_lkp_proto = flow->key.proto;
    key.flow_lkp_metadata_lkp_dir = flow->key.dir;

    flow_data.flow_index = flow_pd->flow_stats_hw_id;
    flow_data.export_en = FALSE;    // TODO: when analytics APIs are ready,
                                    // set this appropriately
    for (uint32_t i = 0; i < 16; i++) {
        src_buf.write("{:#x} ", key.flow_lkp_metadata_lkp_src[i]);
    }
    HAL_TRACE_DEBUG("Src:");
    HAL_TRACE_DEBUG(src_buf.c_str());
    for (uint32_t i = 0; i < 16; i++) {
        dst_buf.write("{:#x} ", key.flow_lkp_metadata_lkp_dst[i]);
    }
    HAL_TRACE_DEBUG("Dst:");
    HAL_TRACE_DEBUG(dst_buf.c_str());
    ret = g_hal_state_pd->flow_table()->insert(&key, &flow_data,
                                               &flow_pd->flow_hash_hw_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow table insert failed, err : {}", ret);
        return ret;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// delete flow hash table entry at given index
//------------------------------------------------------------------------------
hal_ret_t
p4pd_del_flow_hash_table_entry (uint32_t flow_index)
{
    return g_hal_state_pd->flow_table()->remove(flow_index);
}

//------------------------------------------------------------------------------
// program flow hash table entry with given data (flow index)
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_flow_hash_table_entries (pd_session_t *session_pd,
                                  pd_session_args_t *args)
{
    hal_ret_t               ret;
    session_t               *session = (session_t *)session_pd->session;

    ret = p4pd_add_flow_hash_table_entry(&session->iflow->config,
                                         (pd_l2seg_t *)args->l2seg_s->pd,
                                         &session_pd->iflow);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    if (session_pd->rflow_valid) {
        ret = p4pd_add_flow_hash_table_entry(&session->rflow->config,
                                             (pd_l2seg_t *)args->l2seg_d->pd,
                                             &session_pd->rflow);
        if (ret != HAL_RET_OK) {
            p4pd_del_flow_hash_table_entry(session_pd->iflow.flow_hash_hw_id);
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// program all PD tables for given session and maintain meta data in PD state
//------------------------------------------------------------------------------
hal_ret_t
pd_session_create (pd_session_args_t *args)
{
    hal_ret_t          ret;
    pd_session_t       *session_pd;

    HAL_TRACE_DEBUG("Creating pd state for session");

    session_pd = session_pd_alloc_init();
    if (session_pd == NULL) {
        return HAL_RET_OOM;
    }
    session_pd->session = args->session;
    args->session->pd = session_pd;
    if (args->session->rflow) {
        session_pd->rflow_valid = TRUE;
    } else {
        session_pd->rflow_valid = FALSE;
    }

    // add flow stats entries first
    ret = p4pd_add_flow_stats_table_entries(session_pd);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // if connection tracking is on, add flow state entry for this session
    if (args->session->config.conn_track_en) {
        session_pd->conn_track_en = TRUE;
        ret = p4pd_add_session_state_table_entry(session_pd, args->session_state, args->nwsec_prof);
        if (ret != HAL_RET_OK) {
            goto cleanup;
        }
    } else {
        session_pd->conn_track_en = FALSE;
    }

    // add flow info table entries
    ret = p4pd_add_flow_info_table_entries(args);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_flow_hash_table_entries(session_pd, args);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }

    return HAL_RET_OK;

cleanup:

    if (session_pd) {
        p4pd_del_flow_stats_table_entries(session_pd);
        p4pd_del_session_state_table_entry(session_pd->session_state_idx);
        p4pd_del_flow_info_table_entries(session_pd);
        session_pd_free(session_pd);
        args->session->pd = NULL;
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
