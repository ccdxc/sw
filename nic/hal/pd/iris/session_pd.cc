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
#include <qos_api.hpp>
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

    if (session_pd->iflow_aug_valid) {
        ret = p4pd_add_flow_stats_table_entry(&session_pd->iflow_aug.flow_stats_hw_id);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    // program flow_stats table entry for rflow
    if (session_pd->rflow_valid) {
        ret = p4pd_add_flow_stats_table_entry(&session_pd->rflow.flow_stats_hw_id);
        if (ret != HAL_RET_OK) {
            p4pd_del_flow_stats_table_entry(session_pd->iflow.flow_stats_hw_id);
            return ret;
        }
    }

    if (session_pd->rflow_aug_valid) {
        ret = p4pd_add_flow_stats_table_entry(&session_pd->rflow_aug.flow_stats_hw_id);
        if (ret != HAL_RET_OK) {
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

    if (session_state) {
        // initiator flow specific information
        d.session_state_action_u.session_state_tcp_session_state_info.iflow_tcp_state =
            session_state->iflow_state.state;
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
        d.session_state_action_u.session_state_tcp_session_state_info.rflow_tcp_state =
            session_state->rflow_state.state;
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


        d.session_state_action_u.session_state_tcp_session_state_info.syn_cookie_delta =
            session_state->iflow_state.syn_ack_delta;
    }
    
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
//------------------------------------------------------------------------------
// program flow info table entry at either given index or if given index is 0
// allocate an index and return that
// TODO:
// 1. flow_ttl is set to DEF_TTL (64) currently ... if we have packet in hand we
//    should take it from there or else default is fine
// 2. twice nat not supported
//-----------------------------------------------------------------------------
hal_ret_t
p4pd_add_flow_info_table_entry (session_t *session, pd_flow_t *flow_pd, flow_role_t role, bool aug)
{
    hal_ret_t                ret;
    DirectMap                *dm;
    flow_info_actiondata     d = { 0};
    timespec_t               ts;
    flow_pgm_attrs_t         *flow_attrs = NULL;
    flow_cfg_t               *flow_cfg = NULL;
    bool                     is_tcp_proxy_flow = false;
    bool                     is_ipsec_proxy_flow = false;
    pd_session_t             *sess_pd = NULL;

    sess_pd = session->pd;

    dm = g_hal_state_pd->dm_table(P4TBL_ID_FLOW_INFO);
    HAL_ASSERT(dm != NULL);

    // get the flow attributes
    if (role == FLOW_ROLE_INITIATOR) {
        flow_attrs = aug ? &session->iflow->assoc_flow->pgm_attrs :
            &session->iflow->pgm_attrs;
        flow_cfg = &session->iflow->config;
    } else {
        flow_attrs = aug ? &session->rflow->assoc_flow->pgm_attrs :
            &session->rflow->pgm_attrs;
        flow_cfg = &session->rflow->config;
    }

    // populate the action information
    if (flow_attrs->drop) {
        d.actionid = FLOW_INFO_FLOW_HIT_DROP_ID;
        HAL_TRACE_DEBUG("PD-Session:{}: Action being set to drop", __FUNCTION__);
    } else {
        d.actionid = FLOW_INFO_FLOW_INFO_ID;
    }
    if (((flow_cfg->key.sport == 80) && (flow_cfg->key.dport == 47273)) ||
         ((flow_cfg->key.sport == 47273) && (flow_cfg->key.dport == 80))) {
        is_tcp_proxy_flow = true;
        HAL_TRACE_DEBUG("TCP Proxy flow_cfg {}", flow_cfg->key);
    }

    if (((flow_cfg->key.sport == 44445) && (flow_cfg->key.dport == 44444)) ||
         ((flow_cfg->key.sport == 44444) && (flow_cfg->key.dport == 44445))) {
        is_ipsec_proxy_flow = true;
        HAL_TRACE_DEBUG("IPSEC Proxy flow_cfg {}", flow_cfg->key);
    }

    d.flow_info_action_u.flow_info_flow_info.dst_lport = flow_attrs->lport;
    d.flow_info_action_u.flow_info_flow_info.multicast_en = flow_attrs->mcast_en;

    // TBD: where do these come from ?
    d.flow_info_action_u.flow_info_flow_info.flow_steering_only = 0;
    // TBD: the following come when QoS model is defined
    d.flow_info_action_u.flow_info_flow_info.ingress_policer_index = 0;
    d.flow_info_action_u.flow_info_flow_info.egress_policer_index = 0;
    d.flow_info_action_u.flow_info_flow_info.ingress_mirror_session_id = flow_cfg->ing_mirror_session;
    d.flow_info_action_u.flow_info_flow_info.egress_mirror_session_id = flow_cfg->eg_mirror_session;
    // Set the tunnel originate flag
    d.flow_info_action_u.flow_info_flow_info.tunnel_originate = 
                                                    flow_attrs->tunnel_orig;
    // L4 LB (NAT) Info
    // TODO: Replace these with nat_l4_port and nat_ip 
    d.flow_info_action_u.flow_info_flow_info.nat_l4_port = flow_attrs->nat_l4_port;
    switch (flow_attrs->nat_type) {
        case NAT_TYPE_SNAT:
            memcpy(d.flow_info_action_u.flow_info_flow_info.nat_ip, &flow_attrs->nat_sip.addr,
                    sizeof(ipvx_addr_t));
            if (flow_cfg->key.flow_type == FLOW_TYPE_V6) {
                memrev(d.flow_info_action_u.flow_info_flow_info.nat_ip, 
                        sizeof(d.flow_info_action_u.flow_info_flow_info.nat_ip));
            }
            // d.flow_info_action_u.flow_info_flow_info.nat_l4_port = flow_attrs->nat_sport;
            break;
        case NAT_TYPE_DNAT:
            memcpy(d.flow_info_action_u.flow_info_flow_info.nat_ip, &flow_attrs->nat_dip.addr,
                    sizeof(ipvx_addr_t));
            if (flow_cfg->key.flow_type == FLOW_TYPE_V6) {
                memrev(d.flow_info_action_u.flow_info_flow_info.nat_ip, 
                        sizeof(d.flow_info_action_u.flow_info_flow_info.nat_ip));
            }
            // d.flow_info_action_u.flow_info_flow_info.nat_l4_port = flow_attrs->nat_dport;
            break;
        case NAT_TYPE_TWICE_NAT:
            break;
        default:
            break;
    }

#if 0
    // TODO: Do this in PI
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
#endif

    d.flow_info_action_u.flow_info_flow_info.cos_en = 
        qos_marking_action_pcp_rewrite_en(&flow_cfg->eg_qos_action.marking_action);
    d.flow_info_action_u.flow_info_flow_info.cos = 
        qos_marking_action_pcp(&flow_cfg->eg_qos_action.marking_action);
    d.flow_info_action_u.flow_info_flow_info.dscp_en = 
        qos_marking_action_dscp_rewrite_en(&flow_cfg->eg_qos_action.marking_action);
    d.flow_info_action_u.flow_info_flow_info.dscp = 
        qos_marking_action_dscp(&flow_cfg->eg_qos_action.marking_action);

    // TBD: check class NIC mode and set this
    d.flow_info_action_u.flow_info_flow_info.qid_en = flow_attrs->qid_en;
#if 0
    if (is_tcp_proxy_flow) {
        // HACK DO NOT COMMIT
        d.flow_info_action_u.flow_info_flow_info.qid_en = 1;
    }
#endif
    if (flow_attrs->qid_en) {
        d.flow_info_action_u.flow_info_flow_info.qtype = flow_attrs->qtype;
        d.flow_info_action_u.flow_info_flow_info.tunnel_vnid = flow_attrs->qid;
    }

    // TBD: check analytics policy and set this
    d.flow_info_action_u.flow_info_flow_info.log_en = FALSE;

    d.flow_info_action_u.flow_info_flow_info.rewrite_flags = 
        (flow_attrs->mac_sa_rewrite ? REWRITE_FLAGS_MAC_SA : 0) |
        (flow_attrs->mac_da_rewrite ? REWRITE_FLAGS_MAC_DA : 0) |
        (flow_attrs->ttl_dec ? REWRITE_FLAGS_TTL_DEC : 0);

    if (flow_attrs->rw_act != REWRITE_NOP_ID) {
        d.flow_info_action_u.flow_info_flow_info.rewrite_index = flow_attrs->rw_idx;
#if 0
        d.flow_info_action_u.flow_info_flow_info.rewrite_index =
            ep_pd_get_rw_tbl_idx(flow_attrs->dep->pd,
                flow_attrs->rw_act);
#endif
    }
    d.flow_info_action_u.flow_info_flow_info.tunnel_vnid = flow_attrs->tnnl_vnid;
    d.flow_info_action_u.flow_info_flow_info.tunnel_rewrite_index = flow_attrs->tnnl_rw_idx;

    // TODO: if we are doing routing, then set ttl_dec to TRUE
    d.flow_info_action_u.flow_info_flow_info.flow_conn_track = session->config.conn_track_en;
    d.flow_info_action_u.flow_info_flow_info.flow_ttl = 64;
    d.flow_info_action_u.flow_info_flow_info.flow_role = flow_attrs->role;
    d.flow_info_action_u.flow_info_flow_info.session_state_index =
        session->config.conn_track_en ? sess_pd->session_state_idx : 0;
    clock_gettime(CLOCK_REALTIME_COARSE, &ts);
    d.flow_info_action_u.flow_info_flow_info.start_timestamp = ts.tv_sec;

     if (is_tcp_proxy_flow) {
         d.flow_info_action_u.flow_info_flow_info.dst_lport = 1001;
         d.flow_info_action_u.flow_info_flow_info.tunnel_vnid = 0;
         d.flow_info_action_u.flow_info_flow_info.qid_en = 1;
     }

     if (is_ipsec_proxy_flow) {
         d.flow_info_action_u.flow_info_flow_info.dst_lport = 1004;
         d.flow_info_action_u.flow_info_flow_info.tunnel_vnid = 0;
         d.flow_info_action_u.flow_info_flow_info.qid_en = 1;
     }
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

    ret = p4pd_add_flow_info_table_entry(args->session, &session_pd->iflow, FLOW_ROLE_INITIATOR, false);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    if (session_pd->iflow_aug_valid) {
        ret = p4pd_add_flow_info_table_entry(args->session, &session_pd->iflow_aug, FLOW_ROLE_INITIATOR, true);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    // program flow_info table entry for rflow
    if (session_pd->rflow_valid) {
        ret = p4pd_add_flow_info_table_entry(args->session, &session_pd->rflow, FLOW_ROLE_RESPONDER, false);
        if (ret != HAL_RET_OK) {
            p4pd_del_flow_info_table_entry(session_pd->iflow.flow_stats_hw_id);
            return ret;
        }
        if (session_pd->rflow_aug_valid) {
            ret = p4pd_add_flow_info_table_entry(args->session, &session_pd->rflow_aug, FLOW_ROLE_RESPONDER, true);
            if (ret != HAL_RET_OK) {
                return ret;
            }
        }
    }

    return ret;
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
    p4pd_flow_hash_data_t    flow_data = { 0 };
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
    } else if (flow->key.flow_type == FLOW_TYPE_L2) {
        memrev(key.flow_lkp_metadata_lkp_src, 6);
        memrev(key.flow_lkp_metadata_lkp_dst, 6);
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
    key.flow_lkp_metadata_lkp_vrf = l2seg_pd->l2seg_ten_hw_id;
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
    // TODO: Cleanup. Dont return flow coll from lib.
    if (ret != HAL_RET_OK && ret != HAL_RET_FLOW_COLL) {
        HAL_TRACE_ERR("flow table insert failed, err : {}", ret);
        return ret;
    }

    return ret;
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
                                         (pd_l2seg_t *)(session->iflow->sl2seg->pd),
                                         &session_pd->iflow);
    if (ret == HAL_RET_FLOW_COLL) {
        if (args->rsp) {
            args->rsp->mutable_status()->mutable_iflow_status()->set_flow_coll(true);
        }
        HAL_TRACE_DEBUG("IFlow Collision!");
        ret = HAL_RET_OK;
    }
    if (ret != HAL_RET_OK) {
        return ret;
    }

    if (session_pd->iflow_aug_valid) {
        // TODO: key has to involve service done? populate in flow_attrs
        ret = p4pd_add_flow_hash_table_entry(&session->iflow->config,
                (pd_l2seg_t *)(session->iflow->dl2seg->pd),
                &session_pd->iflow);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }
    if (session_pd->rflow_valid) {
        ret = p4pd_add_flow_hash_table_entry(&session->rflow->config,
                                             (pd_l2seg_t *)session->rflow->sl2seg->pd,
                                             // (pd_l2seg_t *)args->l2seg_d->pd,
                                             &session_pd->rflow);
        if (ret == HAL_RET_FLOW_COLL) {
            if (args->rsp) {
                args->rsp->mutable_status()->mutable_rflow_status()->set_flow_coll(true);
            }
            HAL_TRACE_DEBUG("RFlow Collision!");
            ret = HAL_RET_OK;
        }
        if (ret != HAL_RET_OK) {
            p4pd_del_flow_hash_table_entry(session_pd->iflow.flow_hash_hw_id);
        }
        if (session_pd->rflow_aug_valid) {
            // TODO: key has to involve service done? populate in flow_attrs
            ret = p4pd_add_flow_hash_table_entry(&session->rflow->config,
                    (pd_l2seg_t *)session->rflow->dl2seg->pd,
                    // (pd_l2seg_t *)args->l2seg_d->pd,
                    &session_pd->rflow);
            if (ret != HAL_RET_OK) {
                p4pd_del_flow_hash_table_entry(session_pd->iflow.flow_hash_hw_id);
            }
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
    session_t          *session = args->session;

    HAL_TRACE_DEBUG("Creating pd state for session");

    session_pd = session_pd_alloc_init();
    if (session_pd == NULL) {
        return HAL_RET_OOM;
    }
    session_pd->session = args->session;
    args->session->pd = session_pd;

    if (session->rflow) {
        session_pd->rflow_valid = TRUE;
        if (session->rflow->assoc_flow) {
            session_pd->rflow_aug_valid = true;
        }
    } else {
        session_pd->rflow_valid = FALSE;
    }

    if (session->iflow->assoc_flow) {
        session_pd->iflow_aug_valid = true;
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
