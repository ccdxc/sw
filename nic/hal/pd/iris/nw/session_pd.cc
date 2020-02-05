// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <time.h>
#include <string.h>
#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/timestamp.hpp"
#include "nic/sdk/include/sdk/lock.hpp"
#include "gen/p4gen/p4/include/p4pd.h"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "nic/hal/pd/iris/nw/session_pd.hpp"
#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint_api.hpp"
#include "nic/hal/pd/iris/nw/endpoint_pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/plugins/cfg/aclqos/qos_api.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/pd/iris/internal/system_pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "platform/capri/capri_tm_rw.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "nic/sdk/lib/table/sldirectmap/sldirectmap.hpp"
#include "nic/sdk/include/sdk/table.hpp"
#include "nic/sdk/lib/utils/utils.hpp"

using sdk::table::sdk_table_api_params_t;

#define	FLOW_RATE_COMPUTE_TIME_1SEC	1
#define	FLOW_RATE_COMPUTE_TIME_2SEC	2
#define	FLOW_RATE_COMPUTE_TIME_4SEC	4
#define	FLOW_RATE_COMPUTE_TIME_8SEC	8

namespace hal {
namespace pd {

std::string hex_str(const uint8_t *buf, size_t sz)
{
    std::ostringstream result;

    for(size_t i = 0; i < sz; i+=8) {
        result << " 0x";
        for (size_t j = i ; j < sz && j < i+8; j++) {
            result << std::setw(2) << std::setfill('0') << std::hex << (int)buf[j];
        }
    }

    return result.str();
}

#define BYTES_TO_UINT64(addr) \
    (((uint64_t)(addr)[0] & 0xFF)               | \
     ((uint64_t)((addr)[1] & 0xFF) << 8)        | \
     ((uint64_t)((addr)[2] & 0xFF) << 16)       | \
     ((uint64_t)((addr)[3] & 0xFF) << 24)       | \
     ((uint64_t)((addr)[4] & 0xFF) << 32)       | \
     ((uint64_t)((addr)[5] & 0xFF) << 40)       | \
     ((uint64_t)((addr)[6] & 0xFF) << 48)       | \
     ((uint64_t)((addr)[7] & 0xFF) << 56))

//------------------------------------------------------------------------------
// program flow stats table entry and return the index at which
// the entry is programmed
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_upd_flow_stats_table_entry (uint32_t *assoc_hw_idx, uint64_t permit_packets,
                                     uint64_t permit_bytes, uint64_t drop_packets,
                                     uint64_t drop_bytes, uint64_t clock, bool update)
{
    hal_ret_t ret;
    sdk_ret_t sdk_ret;
    sldirectmap *stats_table = NULL;
    flow_stats_actiondata_t d = { 0 };
    sdk_table_api_params_t params;

    if (!update && (*assoc_hw_idx)) {
        HAL_TRACE_VERBOSE("Hw entry already created Hw index:{}", *assoc_hw_idx);
        return HAL_RET_OK;
    }

    SDK_ASSERT(assoc_hw_idx != NULL);
    stats_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_FLOW_STATS);
    SDK_ASSERT(stats_table != NULL);

    d.action_id = FLOW_STATS_FLOW_STATS_ID;
    // P4 has 32 bits so we have to use top 32 bits. We lose the precision by 2^16 ns
    d.action_u.flow_stats_flow_stats.last_seen_timestamp = clock >> 16;

    memcpy(d.action_u.flow_stats_flow_stats.permit_packets, &permit_packets, sizeof(uint64_t));
    memcpy(d.action_u.flow_stats_flow_stats.permit_bytes, &permit_bytes, sizeof(uint64_t));
    memcpy(d.action_u.flow_stats_flow_stats.drop_packets, &drop_packets, sizeof(uint64_t));
    memcpy(d.action_u.flow_stats_flow_stats.drop_bytes, &drop_bytes, sizeof(uint64_t));

    bzero(&params, sizeof(sdk_table_api_params_t));
    params.actiondata = &d;

    if (update) {
        params.handle.pindex(*assoc_hw_idx);
        sdk_ret = stats_table->update(&params);
    } else {
        // insert the entry
        sdk_ret = stats_table->insert(&params);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow stats table write failure, err : {}", ret);
        return ret;
    }
 
    SDK_ASSERT(params.handle.pvalid());
    *assoc_hw_idx = params.handle.pindex();

    HAL_TRACE_VERBOSE("Setting the last seen timestamp: {} clock {} hwIndx: {}",
                      d.action_u.flow_stats_flow_stats.last_seen_timestamp,
                      clock, *assoc_hw_idx);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
hal_ret_t
p4pd_flow_stats_table_entry_last_seen_ts (uint32_t assoc_hw_idx, uint64_t clock)
{
    sldirectmap            *stats_table = NULL;
    hal_ret_t               ret;
    sdk_ret_t               sdk_ret;
    flow_stats_actiondata_t d = { 0 };
    sdk_table_api_params_t  params;

    stats_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_FLOW_STATS);
    SDK_ASSERT(stats_table != NULL);

    // Get the entry
    bzero(&params, sizeof(sdk_table_api_params_t));
    params.handle.pindex(assoc_hw_idx);
    params.actiondata = &d;

    sdk_ret = stats_table->get(&params);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error reading stats entry {} ret {}", assoc_hw_idx, ret);
        return ret;
    }

    // Update the last seen timestamp field
    d.action_u.flow_stats_flow_stats.last_seen_timestamp = clock >> 16;

    sdk_ret = stats_table->update(&params);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow stats table write failure, err : {}", ret);
        return ret;
    }
 
    HAL_TRACE_VERBOSE("Setting the last seen timestamp: {} clock {} hwIndx: {}",
                      d.action_u.flow_stats_flow_stats.last_seen_timestamp,
                      clock, assoc_hw_idx);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// delete flow stats table entry at given index
//------------------------------------------------------------------------------
hal_ret_t
p4pd_del_flow_stats_table_entry (uint32_t assoc_hw_idx)
{
    sldirectmap *stats_table = NULL;
    sdk_table_api_params_t params;

    // 0th entry is reserved
    if (!assoc_hw_idx) {
        return HAL_RET_INVALID_ARG;
    }

    stats_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_FLOW_STATS);
    SDK_ASSERT(stats_table != NULL);

    bzero(&params, sizeof(sdk_table_api_params_t));
    params.handle.pindex(assoc_hw_idx);
    return hal_sdk_ret_to_hal_ret(stats_table->remove(&params));
}

//------------------------------------------------------------------------------
// program flow stats table entries for a given session
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_flow_stats_table_entries (pd_session_t *session_pd, pd_session_create_args_t *args,
                                   bool update)
{
    hal_ret_t ret = HAL_RET_OK;

    HAL_TRACE_VERBOSE("iAugVld:{} rVld:{} rAugVld:{} iID:{} iAugID:{} rID:{} rAugID:{} Updt:{}",
                      session_pd->iflow_aug.valid, session_pd->rflow.valid, session_pd->rflow_aug.valid,
                      session_pd->iflow.assoc_hw_id, session_pd->iflow_aug.assoc_hw_id,
                      session_pd->rflow.assoc_hw_id, session_pd->rflow_aug.assoc_hw_id, update);

    // program flow_stats table entry for iflow
    if (args->session_state) {
        ret = p4pd_add_upd_flow_stats_table_entry(&session_pd->iflow.assoc_hw_id,
                                                  args->session_state->iflow_state.packets,
                                                  args->session_state->iflow_state.bytes,
                                                  args->session_state->iflow_state.drop_packets,
                                                  args->session_state->iflow_state.drop_bytes,
                                                  args->clock, update);
    } else {
        ret = p4pd_add_upd_flow_stats_table_entry(&session_pd->iflow.assoc_hw_id,
                                                  0, 0, 0, 0, args->clock, update);
    }
    if (ret != HAL_RET_OK) {
        return ret;
    }

    if (session_pd->iflow_aug.valid) {
        if (args->session_state) {
            ret = p4pd_add_upd_flow_stats_table_entry(&session_pd->iflow_aug.assoc_hw_id,
                                                      args->session_state->iflow_aug_state.packets,
                                                      args->session_state->iflow_aug_state.bytes,
                                                      args->session_state->iflow_aug_state.drop_packets,
                                                      args->session_state->iflow_aug_state.drop_bytes,
                                                      args->clock, update);
        } else {
            ret = p4pd_add_upd_flow_stats_table_entry(&session_pd->iflow_aug.assoc_hw_id,
                                                      0, 0, 0, 0, args->clock, update);

        }
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    // program flow_stats table entry for rflow
    if (session_pd->rflow.valid) {
        if (args->session_state) {
            ret = p4pd_add_upd_flow_stats_table_entry(&session_pd->rflow.assoc_hw_id,
                                                      args->session_state->rflow_state.packets,
                                                      args->session_state->rflow_state.bytes,
                                                      args->session_state->rflow_state.drop_packets,
                                                      args->session_state->rflow_state.drop_bytes,
                                                      args->clock, update);
        } else {
            ret = p4pd_add_upd_flow_stats_table_entry(&session_pd->rflow.assoc_hw_id,
                                                      0, 0, 0, 0, args->clock, update);
        }
        if (ret != HAL_RET_OK) {
            p4pd_del_flow_stats_table_entry(session_pd->iflow.assoc_hw_id);
            return ret;
        }
    }

    if (session_pd->rflow_aug.valid) {
        if (args->session_state) {
            ret = p4pd_add_upd_flow_stats_table_entry(&session_pd->rflow_aug.assoc_hw_id,
                                                      args->session_state->rflow_aug_state.packets,
                                                      args->session_state->rflow_aug_state.bytes,
                                                      args->session_state->rflow_aug_state.drop_packets,
                                                      args->session_state->rflow_aug_state.drop_bytes,
                                                      args->clock, update);
        } else {
            ret = p4pd_add_upd_flow_stats_table_entry(&session_pd->rflow_aug.assoc_hw_id,
                                                      0, 0, 0, 0, args->clock, update);
        }
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
    hal_ret_t ret = HAL_RET_OK;

    ret = p4pd_del_flow_stats_table_entry(session_pd->iflow.assoc_hw_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("iflow flow stats table entry delete failed, "
                      "id {}, err {}", session_pd->iflow.assoc_hw_id, ret);
    }
    if (session_pd->rflow.valid) {
        SDK_ASSERT(session_pd->rflow.assoc_hw_id);
        ret = p4pd_del_flow_stats_table_entry(session_pd->rflow.assoc_hw_id);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("rflow flow stats table entry delete failed, "
                          "id {}, err {}", session_pd->rflow.assoc_hw_id, ret);
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// program flow state table entry and return the index at which
// the entry is programmed
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_upd_session_state_table_entry (pd_session_t *session_pd,
                                        session_state_t *session_state,
                                        nwsec_profile_t *nwsec_profile,
                                        bool update)
{
    hal_ret_t                ret;
    sdk_ret_t                sdk_ret;
    sldirectmap             *session_state_table;
    flow_t                   *iflow, *rflow;
    session_state_actiondata_t d = { 0 };
    session_t                *session = (session_t *)session_pd->session;
    sdk_table_api_params_t   params;

    SDK_ASSERT(session_pd != NULL);
    iflow = session->iflow;
    rflow = session->rflow;
    SDK_ASSERT((iflow != NULL) && (rflow != NULL));

    session_state_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_SESSION_STATE);
    SDK_ASSERT(session_state_table != NULL);

    bzero(&params, sizeof(sdk_table_api_params_t));
    params.actiondata = &d;

    if (update) {
        params.handle.pindex(session_pd->session_state_idx);

        sdk_ret = session_state_table->get(&params);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("sess table update entry: {} not found", session_pd->session_state_idx);
            return ret;
        }
    }
    HAL_TRACE_DEBUG("sess table updt:{} Index:{}", update, session_pd->session_state_idx);

    // populate the action information
    d.action_id = SESSION_STATE_TCP_SESSION_STATE_INFO_ID;

    if (session_state) {
        // session specific information
        d.action_u.session_state_tcp_session_state_info.tcp_ts_option_negotiated =
            session_state->tcp_ts_option;
        d.action_u.session_state_tcp_session_state_info.tcp_sack_perm_option_negotiated =
            session_state->tcp_sack_perm_option;

        // initiator flow specific information
        d.action_u.session_state_tcp_session_state_info.iflow_tcp_state =
            session_state->iflow_state.state;
        d.action_u.session_state_tcp_session_state_info.iflow_tcp_seq_num =
            session_state->iflow_state.tcp_seq_num;
        d.action_u.session_state_tcp_session_state_info.iflow_tcp_ack_num =
            session_state->iflow_state.tcp_ack_num;
        d.action_u.session_state_tcp_session_state_info.iflow_tcp_win_sz =
            session_state->iflow_state.tcp_win_sz;
        d.action_u.session_state_tcp_session_state_info.iflow_tcp_win_scale =
            session_state->iflow_state.tcp_win_scale;
        d.action_u.session_state_tcp_session_state_info.iflow_tcp_mss =
            session_state->iflow_state.tcp_mss;
        d.action_u.session_state_tcp_session_state_info.iflow_tcp_ws_option_sent =
            session_state->iflow_state.tcp_ws_option_sent;
        d.action_u.session_state_tcp_session_state_info.iflow_tcp_ts_option_sent =
            session_state->iflow_state.tcp_ts_option_sent;
        d.action_u.session_state_tcp_session_state_info.iflow_tcp_sack_perm_option_sent =
            session_state->iflow_state.tcp_sack_perm_option_sent;
        d.action_u.session_state_tcp_session_state_info.iflow_exceptions_seen =
            session_state->iflow_state.exception_bmap;

        HAL_TRACE_VERBOSE("iflow:S:{} Seq:{} Ack:{} Sz:{} Sc:{} Mss:{} Ws:{}, ts:{}, Sa:{}, Ex:{}",
                session_state->iflow_state.state,session_state->iflow_state.tcp_seq_num,
                session_state->iflow_state.tcp_ack_num, session_state->iflow_state.tcp_win_sz,
                session_state->iflow_state.tcp_win_scale, session_state->iflow_state.tcp_mss,
                session_state->iflow_state.tcp_ws_option_sent,
                session_state->iflow_state.tcp_ts_option_sent,
                session_state->iflow_state.tcp_sack_perm_option_sent,
                session_state->iflow_state.exception_bmap);

        // responder flow specific information
        d.action_u.session_state_tcp_session_state_info.rflow_tcp_state =
            session_state->rflow_state.state;
        d.action_u.session_state_tcp_session_state_info.rflow_tcp_seq_num =
            session_state->rflow_state.tcp_seq_num;
        d.action_u.session_state_tcp_session_state_info.rflow_tcp_ack_num =
            session_state->rflow_state.tcp_ack_num;
        d.action_u.session_state_tcp_session_state_info.rflow_tcp_win_sz =
            session_state->rflow_state.tcp_win_sz;
        d.action_u.session_state_tcp_session_state_info.rflow_tcp_win_scale =
            session_state->rflow_state.tcp_win_scale;
        d.action_u.session_state_tcp_session_state_info.rflow_tcp_mss =
            session_state->rflow_state.tcp_mss;
        d.action_u.session_state_tcp_session_state_info.rflow_exceptions_seen =
            session_state->rflow_state.exception_bmap;

        HAL_TRACE_VERBOSE("rflow: S:{} Seq:{} Ack:{}, Sz:{} Sc:{}, Mss:{}, Ex:{}",
                session_state->rflow_state.state,session_state->rflow_state.tcp_seq_num,
                session_state->rflow_state.tcp_ack_num, session_state->rflow_state.tcp_win_sz,
                session_state->rflow_state.tcp_win_scale, session_state->rflow_state.tcp_mss,
                session_state->rflow_state.exception_bmap);

        d.action_u.session_state_tcp_session_state_info.syn_cookie_delta =
            session_state->iflow_state.syn_ack_delta;
    }

    d.action_u.session_state_tcp_session_state_info.flow_rtt_seq_check_enabled =
        nwsec_profile ?  nwsec_profile->tcp_rtt_estimate_en : FALSE;

    if (update) {
        // update the entry
        sdk_ret = session_state_table->update(&params);
    } else {
        // insert the entry
        sdk_ret = session_state_table->insert(&params);
    }
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("session state table write failure, err : {}", ret);
        return ret;
    }

    SDK_ASSERT(params.handle.pvalid());
    session_pd->session_state_idx = params.handle.pindex();
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// delete flow state table entry at given index
//------------------------------------------------------------------------------
hal_ret_t
p4pd_del_session_state_table_entry (uint32_t session_state_idx)
{
    hal_ret_t                ret;
    sldirectmap *session_state_table = NULL;
    sdk_table_api_params_t params;

    // 0th entry is reserved
    if (!session_state_idx) {
        return HAL_RET_INVALID_ARG;
    }

    session_state_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_SESSION_STATE);
    SDK_ASSERT(session_state_table != NULL);

    bzero(&params, sizeof(sdk_table_api_params_t));
    params.handle.pindex(session_state_idx);

    ret = hal_sdk_ret_to_hal_ret(session_state_table->remove(&params));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Session State Table entry delete failed, "
                      "id {}, err : {}", session_state_idx, ret);
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
//-----------------------------------------------------------------------------
hal_ret_t
p4pd_add_upd_flow_info_table_entry (session_t *session, pd_flow_t *flow_pd,
                                    flow_role_t role, bool aug, uint64_t clock)
{
    hal_ret_t ret;
    sdk_ret_t sdk_ret;
    sldirectmap *info_table = NULL;
    flow_info_actiondata_t d = { 0};
    flow_pgm_attrs_t *flow_attrs = NULL;
    flow_cfg_t *flow_cfg = NULL;
    pd_session_t *sess_pd = NULL;
    bool entry_exists = false;
    sdk_table_api_params_t params;

    sess_pd = session->pd;

    info_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_FLOW_INFO);
    SDK_ASSERT(info_table != NULL);

    bzero(&params, sizeof(sdk_table_api_params_t));
    params.handle.pindex(flow_pd->assoc_hw_id);
    params.actiondata = &d;
    sdk_ret = info_table->get(&params);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret == HAL_RET_OK) {
        entry_exists = true;
    }
    d.action_u.flow_info_flow_info.flow_only_policy = 1;

    if (role == FLOW_ROLE_INITIATOR) {
        flow_attrs = aug ? &session->iflow->assoc_flow->pgm_attrs :
                &session->iflow->pgm_attrs;
    } else {
        flow_attrs = aug ? &session->rflow->assoc_flow->pgm_attrs :
                &session->rflow->pgm_attrs;
    }
    if (flow_attrs->drop) {
        d.action_id = FLOW_INFO_FLOW_HIT_DROP_ID;
        d.action_u.flow_info_flow_hit_drop.start_timestamp = (clock>>16);
        HAL_TRACE_DEBUG("Action being set to drop");

    } else {
        d.action_id = FLOW_INFO_FLOW_INFO_ID;
        // get the flow attributes
        if (role == FLOW_ROLE_INITIATOR) {
            flow_cfg = &session->iflow->config;
            if (aug) {
                d.action_u.flow_info_flow_info.ingress_mirror_session_id =
                                     session->iflow->assoc_flow->config.ing_mirror_session;
                d.action_u.flow_info_flow_info.egress_mirror_session_id =
                                     session->iflow->assoc_flow->config.eg_mirror_session;
            } else {
                d.action_u.flow_info_flow_info.ingress_mirror_session_id =
                                     flow_cfg->ing_mirror_session;
                d.action_u.flow_info_flow_info.egress_mirror_session_id =
                                     flow_cfg->eg_mirror_session;
            }
        } else {
            flow_cfg = &session->rflow->config;
            if (aug) {
                d.action_u.flow_info_flow_info.ingress_mirror_session_id =
                                     session->rflow->assoc_flow->config.ing_mirror_session;
                d.action_u.flow_info_flow_info.egress_mirror_session_id =
                                     session->rflow->assoc_flow->config.eg_mirror_session;
            } else {
                d.action_u.flow_info_flow_info.ingress_mirror_session_id =
                                     flow_cfg->ing_mirror_session;
                d.action_u.flow_info_flow_info.egress_mirror_session_id =
                                     flow_cfg->eg_mirror_session;
            }
        }
        d.action_u.flow_info_flow_info.ingress_mirror_overwrite = 
            d.action_u.flow_info_flow_info.ingress_mirror_session_id ? 1 : 0;

        if (flow_attrs->export_en) {
            d.action_u.flow_info_flow_info.export_id1 =
                                                 flow_attrs->export_id1;
            d.action_u.flow_info_flow_info.export_id2 =
                                                 flow_attrs->export_id2;
            d.action_u.flow_info_flow_info.export_id3 =
                                                 flow_attrs->export_id3;
            d.action_u.flow_info_flow_info.export_id4 =
                                                 flow_attrs->export_id4;
        }

#if 0
        d.action_u.flow_info_flow_info.expected_src_lif_check_en =
            flow_attrs->expected_src_lif_en;
        d.action_u.flow_info_flow_info.expected_src_lif =
            flow_attrs->expected_src_lif;
        d.action_u.flow_info_flow_info.dst_lport = flow_attrs->lport;
#endif
        d.action_u.flow_info_flow_info.multicast_en = flow_attrs->mcast_en;
        d.action_u.flow_info_flow_info.multicast_ptr = flow_attrs->mcast_ptr;

        // Set the tunnel originate flag
        d.action_u.flow_info_flow_info.tunnel_originate =
                                                        flow_attrs->tunnel_orig;
        // L4 LB (NAT) Info
        d.action_u.flow_info_flow_info.nat_l4_port = flow_attrs->nat_l4_port;
        memcpy(d.action_u.flow_info_flow_info.nat_ip, &flow_attrs->nat_ip.addr,
               sizeof(ipvx_addr_t));
        if (flow_cfg->key.flow_type == FLOW_TYPE_V6) {
            memrev(d.action_u.flow_info_flow_info.nat_ip,
                   sizeof(d.action_u.flow_info_flow_info.nat_ip));
        }
        d.action_u.flow_info_flow_info.twice_nat_idx = flow_attrs->twice_nat_idx;

        // QOS Info
        d.action_u.flow_info_flow_info.qos_class_en = flow_attrs->qos_class_en;
        d.action_u.flow_info_flow_info.qos_class_id = flow_attrs->qos_class_id;

        // TBD: check class NIC mode and set this
        d.action_u.flow_info_flow_info.qid_en = flow_attrs->qid_en;
        if (flow_attrs->qid_en) {
            // d.action_u.flow_info_flow_info.qtype = flow_attrs->qtype;
            d.action_u.flow_info_flow_info.tunnel_vnid = flow_attrs->qid;
        } else {
            d.action_u.flow_info_flow_info.tunnel_vnid = flow_attrs->tnnl_vnid;
        }

        /*
         * For packets to host, the vlan id will be derived from output_mapping, but
         * the decision to do vlan encap or not is coming from here.
         */
        d.action_u.flow_info_flow_info.tunnel_rewrite_index = flow_attrs->tnnl_rw_idx;

        // TBD: check analytics policy and set this
        d.action_u.flow_info_flow_info.log_en = FALSE;

        d.action_u.flow_info_flow_info.rewrite_flags =
            (flow_attrs->mac_sa_rewrite ? REWRITE_FLAGS_MAC_SA : 0) |
            (flow_attrs->mac_da_rewrite ? REWRITE_FLAGS_MAC_DA : 0) |
            (flow_attrs->ttl_dec ? REWRITE_FLAGS_TTL_DEC : 0);

        if (flow_attrs->rw_act != REWRITE_NOP_ID) {
            d.action_u.flow_info_flow_info.rewrite_index = flow_attrs->rw_idx;
        }
    #if 0
        // TODO: if we are doing routing, then set ttl_dec to TRUE
        if ((role == FLOW_ROLE_INITIATOR && !aug) ||
            (role == FLOW_ROLE_RESPONDER && aug)) {
            // Assuming aug flows are either send to uplink or receive from uplink
            d.action_u.flow_info_flow_info.flow_conn_track = true;
        } else {
            d.action_u.flow_info_flow_info.flow_conn_track = session->config.conn_track_en;
        }
    #endif
        d.action_u.flow_info_flow_info.flow_conn_track = session->conn_track_en;
        d.action_u.flow_info_flow_info.flow_ttl = 64;
        d.action_u.flow_info_flow_info.flow_role = flow_attrs->role;
        d.action_u.flow_info_flow_info.session_state_index =
        d.action_u.flow_info_flow_info.flow_conn_track ?
                 sess_pd->session_state_idx : 0;
   
        // Dont update the start timestamp during update
        // we dont want to mess up aging logic
        if (!entry_exists)
            d.action_u.flow_info_flow_info.start_timestamp = (clock>>16);
    } // End updateion flow allow action

    if (entry_exists) {
       params.handle.pindex(flow_pd->assoc_hw_id);
       params.actiondata = &d;
       // Update the entry
       sdk_ret = info_table->update(&params);
       ret = hal_sdk_ret_to_hal_ret(sdk_ret);
       if (ret != HAL_RET_OK) {
           HAL_TRACE_ERR("flow info table update failure, idx : {}, err : {}",
                         flow_pd->assoc_hw_id, ret);
           return ret;
       }
    } else {
       params.handle.pindex(flow_pd->assoc_hw_id);
       params.actiondata = &d;
       // insert the entry
       sdk_ret = info_table->insert_withid(&params);
       ret = hal_sdk_ret_to_hal_ret(sdk_ret);
       if (ret != HAL_RET_OK) {
           HAL_TRACE_ERR("flow info table write failure, idx : {}, err : {}",
                             flow_pd->assoc_hw_id, ret);
           return ret;
       }
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// delete flow info table entry at given index
//------------------------------------------------------------------------------
hal_ret_t
p4pd_del_flow_info_table_entry (uint32_t index)
{
    sldirectmap *info_table = NULL;
    sdk_table_api_params_t params;

    // 0th entry is reserved
    if (!index) {
        return HAL_RET_INVALID_ARG;
    }

    info_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_FLOW_INFO);
    SDK_ASSERT(info_table != NULL);

    bzero(&params, sizeof(sdk_table_api_params_t));
    params.handle.pindex(index);
    return hal_sdk_ret_to_hal_ret(info_table->remove(&params));
}

//------------------------------------------------------------------------------
// delete flow info table entries for a given session
//------------------------------------------------------------------------------
hal_ret_t
p4pd_del_flow_info_table_entries (pd_session_t *session_pd)
{
    hal_ret_t    ret;

    ret = p4pd_del_flow_info_table_entry(session_pd->iflow.assoc_hw_id);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("iflow flow info table entry delete failed, "
                      "idx {}, err {}", session_pd->iflow.assoc_hw_id, ret);
    }
    if (session_pd->rflow.valid) {
        SDK_ASSERT(session_pd->rflow.assoc_hw_id);
        ret = p4pd_del_flow_info_table_entry(session_pd->rflow.assoc_hw_id);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("rflow flow info table entry delete failed, "
                          "idx {}, err {}", session_pd->rflow.assoc_hw_id, ret);
         }
    }

    return ret;
}

//------------------------------------------------------------------------------
// program flow info table entries for a given session
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_flow_info_table_entries (pd_session_create_args_t *args)
{
    hal_ret_t       ret;
    pd_session_t    *session_pd = (pd_session_t *)args->session->pd;

    HAL_TRACE_VERBOSE("Adding flow info table entries");
    // program flow_info table entry for rflow
    if (session_pd->rflow.valid) {
        ret = p4pd_add_upd_flow_info_table_entry(args->session, &session_pd->rflow,
                                                 FLOW_ROLE_RESPONDER, false, args->clock);
        if (ret != HAL_RET_OK) {
            p4pd_del_flow_info_table_entry(session_pd->iflow.assoc_hw_id);
            return ret;
        }
        if (session_pd->rflow_aug.valid) {
            ret = p4pd_add_upd_flow_info_table_entry(args->session, &session_pd->rflow_aug,
                                                     FLOW_ROLE_RESPONDER, true, args->clock);
            if (ret != HAL_RET_OK) {
                return ret;
            }
        }
    }

    ret = p4pd_add_upd_flow_info_table_entry(args->session, &session_pd->iflow,
                                             FLOW_ROLE_INITIATOR, false, args->clock);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    if (session_pd->iflow_aug.valid) {
        ret = p4pd_add_upd_flow_info_table_entry(args->session, &session_pd->iflow_aug,
                                                 FLOW_ROLE_INITIATOR, true, args->clock);
        if (ret != HAL_RET_OK) {
            return ret;
        }
    }

    return ret;
}

hal_ret_t
p4pd_fill_flow_hash_key (flow_key_t *flow_key,
                         uint8_t lkp_inst, flow_hash_info_entry_t &key)
{
    // initialize all the key fields of flow
    if (flow_key->flow_type == FLOW_TYPE_V4) {
        memcpy(key.flow_lkp_metadata_lkp_src, flow_key->sip.v6_addr.addr8, IP6_ADDR8_LEN);
        memcpy(key.flow_lkp_metadata_lkp_dst, flow_key->dip.v6_addr.addr8, IP6_ADDR8_LEN);
    }
    else if (flow_key->flow_type == FLOW_TYPE_V6) {
        sdk::lib::memrev(key.flow_lkp_metadata_lkp_src, flow_key->sip.v6_addr.addr8, IP6_ADDR8_LEN);
        sdk::lib::memrev(key.flow_lkp_metadata_lkp_dst, flow_key->dip.v6_addr.addr8, IP6_ADDR8_LEN);
    } else if (flow_key->flow_type == FLOW_TYPE_L2) {
        sdk::lib::memrev(key.flow_lkp_metadata_lkp_src, flow_key->smac, sizeof(mac_addr_t));
        sdk::lib::memrev(key.flow_lkp_metadata_lkp_dst, flow_key->dmac, sizeof(mac_addr_t));
    }

    if (flow_key->flow_type == FLOW_TYPE_V4 || flow_key->flow_type == FLOW_TYPE_V6) {
        if ((flow_key->proto == IP_PROTO_TCP) || (flow_key->proto == IP_PROTO_UDP)) {
            key.set_flow_lkp_metadata_lkp_sport(flow_key->sport);
            key.set_flow_lkp_metadata_lkp_dport(flow_key->dport);
        } else if ((flow_key->proto == IP_PROTO_ICMP) ||
                (flow_key->proto == IP_PROTO_ICMPV6)) {
            // Set Sport for ECHO request/response
            if (((flow_key->icmp_type == ICMP_TYPE_ECHO_REQUEST || 
                  flow_key->icmp_type == ICMPV6_TYPE_ECHO_REQUEST) && 
                 flow_key->icmp_code == ICMP_CODE_ECHO_REQUEST) ||
                ((flow_key->icmp_type == ICMP_TYPE_ECHO_RESPONSE ||
                  flow_key->icmp_type == ICMPV6_TYPE_ECHO_RESPONSE) &&
                 flow_key->icmp_code == ICMP_CODE_ECHO_RESPONSE)) {
                key.set_flow_lkp_metadata_lkp_sport(flow_key->icmp_id);
            }
            key.set_flow_lkp_metadata_lkp_dport(
                ((flow_key->icmp_type << 8) | flow_key->icmp_code));
        } else if (flow_key->proto == IPPROTO_ESP) {
            key.set_flow_lkp_metadata_lkp_sport(flow_key->spi >> 16 & 0xFFFF);
            key.set_flow_lkp_metadata_lkp_dport(flow_key->spi & 0xFFFF);
        }
    } else {
        // For FLOW_TYPE_L2
        key.set_flow_lkp_metadata_lkp_dport(flow_key->ether_type);
    }
    if (flow_key->flow_type == FLOW_TYPE_L2) {
        key.set_flow_lkp_metadata_lkp_type(FLOW_KEY_LOOKUP_TYPE_MAC);
    } else if (flow_key->flow_type == FLOW_TYPE_V4) {
        key.set_flow_lkp_metadata_lkp_type(FLOW_KEY_LOOKUP_TYPE_IPV4);
    } else if (flow_key->flow_type == FLOW_TYPE_V6) {
        key.set_flow_lkp_metadata_lkp_type(FLOW_KEY_LOOKUP_TYPE_IPV6);
    } else {
        // TODO: for now !!
        key.set_flow_lkp_metadata_lkp_type(FLOW_KEY_LOOKUP_TYPE_NONE);
    }
    key.set_flow_lkp_metadata_lkp_vrf(flow_key->lkpvrf);
    key.set_flow_lkp_metadata_lkp_proto(flow_key->proto);
    key.set_flow_lkp_metadata_lkp_inst(lkp_inst);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// program flow hash table entry for a given flow
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_upd_flow_hash_table_entry (flow_key_t *flow_key,
                                uint8_t lkp_inst, pd_flow_t *flow_pd,
                                uint32_t hash_val, uint8_t export_en,
                                uint32_t *flow_hash_p, bool update=false)
{
    hal_ret_t ret = HAL_RET_OK;
    flow_hash_info_entry_t key;

    key.clear();
    HAL_TRACE_VERBOSE("update {} flow_pd->installed {}", update, flow_pd->installed);
    if (!update && flow_pd->installed) {
        return HAL_RET_OK;
    }

    // initialize all the key fields of flow
    if (flow_key->flow_type == FLOW_TYPE_V4) {
        memcpy(key.flow_lkp_metadata_lkp_src, flow_key->sip.v6_addr.addr8, IP6_ADDR8_LEN);
        memcpy(key.flow_lkp_metadata_lkp_dst, flow_key->dip.v6_addr.addr8, IP6_ADDR8_LEN);
    } 
    else if (flow_key->flow_type == FLOW_TYPE_V6) {
        sdk::lib::memrev(key.flow_lkp_metadata_lkp_src, flow_key->sip.v6_addr.addr8, IP6_ADDR8_LEN);
        sdk::lib::memrev(key.flow_lkp_metadata_lkp_dst, flow_key->dip.v6_addr.addr8, IP6_ADDR8_LEN);
    } else if (flow_key->flow_type == FLOW_TYPE_L2) {
        sdk::lib::memrev(key.flow_lkp_metadata_lkp_src, flow_key->smac, sizeof(mac_addr_t));
        sdk::lib::memrev(key.flow_lkp_metadata_lkp_dst, flow_key->dmac, sizeof(mac_addr_t));
    }

    if (flow_key->flow_type == FLOW_TYPE_V4 || flow_key->flow_type == FLOW_TYPE_V6) {
        if ((flow_key->proto == IP_PROTO_TCP) ||
            (flow_key->proto == IP_PROTO_UDP)) {
            key.flow_lkp_metadata_lkp_sport = (flow_key->sport);
            key.flow_lkp_metadata_lkp_dport = (flow_key->dport);
        } else if ((flow_key->proto == IP_PROTO_ICMP) ||
                (flow_key->proto == IP_PROTO_ICMPV6)) {
            // Set Sport for ECHO request/response
            if (((flow_key->icmp_type == ICMP_TYPE_ECHO_REQUEST ||
                  flow_key->icmp_type == ICMPV6_TYPE_ECHO_REQUEST) &&
                 flow_key->icmp_code == ICMP_CODE_ECHO_REQUEST) ||
                ((flow_key->icmp_type == ICMP_TYPE_ECHO_RESPONSE ||
                  flow_key->icmp_type == ICMPV6_TYPE_ECHO_RESPONSE) &&
                 flow_key->icmp_code == ICMP_CODE_ECHO_RESPONSE)) {
                key.flow_lkp_metadata_lkp_sport = (flow_key->icmp_id);
            }
            key.flow_lkp_metadata_lkp_dport =
                ((flow_key->icmp_type << 8) | flow_key->icmp_code);
        } else if (flow_key->proto == IPPROTO_ESP) {
            key.flow_lkp_metadata_lkp_sport = (flow_key->spi >> 16 & 0xFFFF);
            key.flow_lkp_metadata_lkp_dport = (flow_key->spi & 0xFFFF);
        }
    } else {
        // For FLOW_TYPE_L2
        key.flow_lkp_metadata_lkp_dport = flow_key->ether_type;
    }
    if (flow_key->flow_type == FLOW_TYPE_L2) {
        key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_MAC;
    } else if (flow_key->flow_type == FLOW_TYPE_V4) {
        key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV4;
    } else if (flow_key->flow_type == FLOW_TYPE_V6) {
        key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV6;
    } else {
        // TODO: for now !!
        key.flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_NONE;
    }
    key.flow_lkp_metadata_lkp_vrf = flow_key->lkpvrf;
    key.flow_lkp_metadata_lkp_proto = flow_key->proto;
    key.flow_lkp_metadata_lkp_inst = lkp_inst;

    key.flow_index = flow_pd->assoc_hw_id;
    key.export_en = export_en;
    key.entry_valid = true;

    if (hal::utils::hal_trace_level() >= ::utils::trace_verbose) {
        fmt::MemoryWriter src_buf, dst_buf;
        for (uint32_t i = 0; i < 16; i++) {
            src_buf.write("{:#x} ", key.flow_lkp_metadata_lkp_src[i]);
        }
        for (uint32_t i = 0; i < 16; i++) {
            dst_buf.write("{:#x} ", key.flow_lkp_metadata_lkp_dst[i]);
        }
        HAL_TRACE_VERBOSE("update {}, src {}, dst {}", update, src_buf.c_str(), dst_buf.c_str());
    }
   
    bool hash_valid = (hash_val) ? true : false;

    if (update) {
        ret = g_hal_state_pd->flow_table_pd_get()->update(&key,
                                                          &hash_val, hash_valid);
    } else {
        ret = g_hal_state_pd->flow_table_pd_get()->insert(&key,
                                                          &hash_val, hash_valid);
    }

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow table insert failed, err {}", ret);
        return ret;
    }

    *flow_hash_p = hash_val;
    flow_pd->installed = true;

    return ret;
}

//------------------------------------------------------------------------------
// delete flow hash table entry at given index
//------------------------------------------------------------------------------
static hal_ret_t
p4pd_del_flow_hash_table_entry (flow_key_t *flow_key,
                                uint8_t lkp_inst, uint8_t export_en, 
                                pd_flow_t *flow_pd)
{
    hal_ret_t ret = HAL_RET_OK;
    flow_hash_info_entry_t key;

    key.clear();
    if (flow_pd->installed == false) {
        return HAL_RET_OK;
    }

    p4pd_fill_flow_hash_key(flow_key, lkp_inst, key);

    if (hal::utils::hal_trace_level() >= ::utils::trace_verbose) {
        fmt::MemoryWriter src_buf, dst_buf;
        for (uint32_t i = 0; i < 16; i++) {
            src_buf.write("{:#x} ", key.flow_lkp_metadata_lkp_src[i]);
        }
        for (uint32_t i = 0; i < 16; i++) {
            dst_buf.write("{:#x} ", key.flow_lkp_metadata_lkp_dst[i]);
        }
        HAL_TRACE_VERBOSE("src {}, dst {}", src_buf.c_str(), dst_buf.c_str());
    }

    ret = g_hal_state_pd->flow_table_pd_get()->remove(&key);
    if (ret == HAL_RET_OK) {
        flow_pd->installed = false;
    }
    return ret;
}

//------------------------------------------------------------------------------
// Remove all valid flow hash table entries with given data (flow index)
//------------------------------------------------------------------------------
hal_ret_t
p4pd_del_flow_hash_table_entries (pd_session_t *session_pd)
{
    hal_ret_t ret = HAL_RET_OK;
    hal_ret_t final_ret = HAL_RET_OK;
    session_t *session = (session_t *)session_pd->session;

    if (session_pd->iflow.valid) {
        ret = p4pd_del_flow_hash_table_entry(&session->iflow->config.key,
                                             session->iflow->pgm_attrs.lkp_inst,
                                             session->iflow->pgm_attrs.export_en,
                                             &session_pd->iflow);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("iflow flow info table entry delete failed, "
                          "session handle :{}, session idx :{}, err :{}",
                          session->hal_handle, session_pd->session_state_idx, ret);
            final_ret = ret;
        }
    }

    if (session_pd->iflow_aug.valid) {
        ret = p4pd_del_flow_hash_table_entry(&session->iflow->assoc_flow->config.key,
                                             session->iflow->assoc_flow->pgm_attrs.lkp_inst,
                                             session->iflow->assoc_flow->pgm_attrs.export_en,
                                             &session_pd->iflow_aug);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("iflow aug flow info table entry delete failed, "
                          "session handle :{}, session idx :{}, err :{}",
                          session->hal_handle, session_pd->session_state_idx, ret);
            final_ret = ret;
        }
    }

    if (session_pd->rflow.valid) {
        ret = p4pd_del_flow_hash_table_entry(&session->rflow->config.key,
                                             session->rflow->pgm_attrs.lkp_inst,
                                             session->rflow->pgm_attrs.export_en,
                                             &session_pd->rflow);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("rflow flow info table entry delete failed, "
                          "session handle :{}, session idx :{}, err :{}",
                          session->hal_handle, session_pd->session_state_idx, ret);
            final_ret = ret;
        }
    }

    if (session_pd->rflow_aug.valid) {
        ret = p4pd_del_flow_hash_table_entry(&session->rflow->assoc_flow->config.key,
                                             session->rflow->assoc_flow->pgm_attrs.lkp_inst,
                                             session->rflow->assoc_flow->pgm_attrs.export_en,
                                             &session_pd->rflow_aug);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("rflow aug flow info table entry delete failed, "
                          "session handle :{}, session idx :{}, err :{}",
                          session->hal_handle, session_pd->session_state_idx, ret);
            final_ret = ret;
        }

    }

    return final_ret;
}

//------------------------------------------------------------------------------
// Program flow hash table entry with given data (flow index)
// This will be called from session update and create
//------------------------------------------------------------------------------
hal_ret_t
p4pd_add_upd_flow_hash_table_entries (pd_session_t *session_pd,
                                      pd_session_create_args_t *args,
                                      bool update)
{
    hal_ret_t               ret = HAL_RET_OK;
    session_t               *session = (session_t *)session_pd->session;
    uint32_t                flow_hash = 0;

    HAL_TRACE_VERBOSE("Add flow hash table entries update {} rflow.valid {} "
                    "rflow.installed {} iflow.installed {} "
                    "update_iflow {} rflow_aug.valid {} "
                    "rflow_aug.installed {}", update, session_pd->rflow.valid,
                    session_pd->rflow.installed, session_pd->iflow.installed,
                    args->update_iflow, session_pd->rflow_aug.valid,
                    session_pd->rflow_aug.installed);
    if (session_pd->rflow.valid &&
            (args->update_rflow || !session_pd->rflow.installed)) {
        ret = p4pd_add_upd_flow_hash_table_entry(&session->rflow->config.key,
                                             session->rflow->pgm_attrs.lkp_inst,
                                             &session_pd->rflow,
                                             0,
                                             session->rflow->pgm_attrs.export_en,
                                             &flow_hash, update);

        if (args->rsp) {
            args->rsp->mutable_status()->mutable_rflow_status()->set_flow_hash(
                                    flow_hash);
        }
        if (ret != HAL_RET_OK) {
            goto p4pd_add_flow_hash_table_entries_return;
        }

        if (session_pd->rflow_aug.valid &&
                (args->update_rflow || !session_pd->rflow_aug.installed)) {
            // TODO: key has to involve service done? populate in flow_attrs
            ret = p4pd_add_upd_flow_hash_table_entry(&session->rflow->assoc_flow->config.key,
                                                 session->rflow->assoc_flow->pgm_attrs.lkp_inst,
                                                 &session_pd->rflow_aug,
                                                 0,
                                                 session->rflow->assoc_flow->pgm_attrs.export_en,
                                                 &flow_hash, update);
            if (args->rsp) {
                args->rsp->mutable_status()->mutable_rflow_status()->set_flow_hash(
                                            flow_hash);
            }
            if (ret != HAL_RET_OK) {
                goto p4pd_add_flow_hash_table_entries_return;
            }
        }
    }

    if (session_pd->iflow.valid &&
            (args->update_iflow || !session_pd->iflow.installed)) {
        ret = p4pd_add_upd_flow_hash_table_entry(&session->iflow->config.key,
                                             session->iflow->pgm_attrs.lkp_inst,
                                             &session_pd->iflow,
                                             args->iflow_hash,
                                             session->iflow->pgm_attrs.export_en,
                                             &flow_hash, update);
        if (args->rsp) {
            args->rsp->mutable_status()->mutable_iflow_status()->set_flow_hash(
                                          flow_hash);
        }

        if (ret != HAL_RET_OK) {
            goto p4pd_add_flow_hash_table_entries_return;
        }
    }

    if (session_pd->iflow_aug.valid &&
            (args->update_iflow || !session_pd->iflow_aug.installed)) {
        ret = p4pd_add_upd_flow_hash_table_entry(&session->iflow->assoc_flow->config.key,
                                             session->iflow->assoc_flow->pgm_attrs.lkp_inst,
                                             &session_pd->iflow_aug,
                                             0,
                                             session->iflow->assoc_flow->pgm_attrs.export_en,
                                             &flow_hash, update);
        if (args->rsp) {
            args->rsp->mutable_status()->mutable_iflow_status()->set_flow_hash(
                              flow_hash);
        }

        if (ret != HAL_RET_OK) {
            goto p4pd_add_flow_hash_table_entries_return;
        }
    }

p4pd_add_flow_hash_table_entries_return:
    if (ret != HAL_RET_OK) {
        SDK_ASSERT(p4pd_del_flow_hash_table_entries(session_pd) == HAL_RET_OK);
    }

    return ret;
}

//------------------------------------------------------------------------------
// program all PD tables for given session and maintain meta data in PD state
//------------------------------------------------------------------------------
hal_ret_t
pd_session_create (pd_func_args_t *pd_func_args)
{
    pd_conv_sw_clock_to_hw_clock_args_t     clock_args;
    hal_ret_t                               ret;
    pd_session_create_args_t               *args = pd_func_args->pd_session_create;
    pd_session_t                           *session_pd;
    session_t                              *session = args->session;
    uint64_t                                sw_ns = 0, clock=0;
    timespec_t                              ts;
    pd_func_args_t                          pd_clock_fn_args = {0};

    HAL_TRACE_VERBOSE("Creating pd state for session");

    session_pd = session_pd_alloc_init();
    if (session_pd == NULL) {
        return HAL_RET_OOM;
    }
    session_pd->session = args->session;
    args->session->pd = session_pd;

    if (session->rflow) {
        session_pd->rflow.valid = TRUE;
        if (session->rflow->assoc_flow) {
            session_pd->rflow_aug.valid = TRUE;
        }
    } else {
        session_pd->rflow.valid = FALSE;
    }

    session_pd->iflow.valid = true;
    if (session->iflow->assoc_flow) {
        session_pd->iflow_aug.valid = TRUE;
    }

    // Get the hw clock
    clock_gettime(CLOCK_MONOTONIC, &ts);
    sdk::timestamp_to_nsecs(&ts, &sw_ns);
    clock_args.hw_tick = &clock;
    clock_args.sw_ns = sw_ns;
    pd_clock_fn_args.pd_conv_sw_clock_to_hw_clock = &clock_args;
    pd_conv_sw_clock_to_hw_clock(&pd_clock_fn_args);

    args->clock = clock;

    // add flow stats entries first
    ret = p4pd_add_flow_stats_table_entries(session_pd, args, false);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // HACK: Only for TCP proxy we are disabling connection tracking.
    //       Have to disable once we make TCP proxy work with connection tracking
    if (session_pd->iflow_aug.valid || session_pd->rflow_aug.valid) {
        args->session->conn_track_en = 0;
    }

    // if connection tracking is on, add flow state entry for this session
    if (args->session->conn_track_en) {
        session_pd->conn_track_en = 1;
        ret = p4pd_add_upd_session_state_table_entry(session_pd, args->session_state,
                                                     args->nwsec_prof, false);
        if (ret != HAL_RET_OK) {
            goto cleanup;
        }
    } else {
        session_pd->conn_track_en = 0;
    }

    // add flow info table entries
    ret = p4pd_add_flow_info_table_entries(args);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }

    ret = p4pd_add_upd_flow_hash_table_entries(session_pd, args, false);
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

//-----------------------------------------
// update PD tables for given session
//------------------------------------------
hal_ret_t
pd_session_update (pd_func_args_t *pd_func_args)
{
    timespec_t                           ts;
    pd_conv_sw_clock_to_hw_clock_args_t  clock_args;
    pd_func_args_t                       pd_clock_fn_args = {0};
    hal_ret_t                            ret;
    pd_session_update_args_t            *args = pd_func_args->pd_session_update;
    pd_session_t                        *session_pd = NULL;
    session_t                           *session = args->session;
    uint64_t                             sw_ns = 0, clock=0;

    HAL_TRACE_VERBOSE("Updating pd state for session");

    session_pd = session->pd;

    SDK_ASSERT(session_pd != NULL);

    if (session->rflow) {
        //HAL_TRACE_DEBUG("Programming Rflow");
        session_pd->rflow.valid = TRUE;
        if (session->rflow->assoc_flow) {
            session_pd->rflow_aug.valid = TRUE;
        }
    } else {
        session_pd->rflow.valid = FALSE;
    }

    if (session->iflow->assoc_flow) {
        session_pd->iflow_aug.valid = TRUE;
    }

    // Get the hw clock
    clock_gettime(CLOCK_MONOTONIC, &ts);
    sdk::timestamp_to_nsecs(&ts, &sw_ns);
    clock_args.hw_tick = &clock;
    clock_args.sw_ns = sw_ns;
    pd_clock_fn_args.pd_conv_sw_clock_to_hw_clock = &clock_args;
    pd_conv_sw_clock_to_hw_clock(&pd_clock_fn_args);

    args->clock = clock;

    // update/add flow stats entries first
    ret = p4pd_add_flow_stats_table_entries(session_pd, (pd_session_create_args_t *)args, true);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Flow stats table entry upd failure");
        return ret;
    }

    // if connection tracking is on, add flow state entry for this session
    if (args->session->conn_track_en) {
        session_pd->conn_track_en = 1;
        ret = p4pd_add_upd_session_state_table_entry(session_pd, args->session_state,
                                                     args->nwsec_prof, true);
        if (ret != HAL_RET_OK) {
            goto cleanup;
        }
    } else {
        session_pd->conn_track_en = 0;
    }

    // update/add flow info table entries
    ret = p4pd_add_flow_info_table_entries((pd_session_create_args_t *)args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Flow info table entry upd failure");
        goto cleanup;
    }

    // Typecasting to pd_session_create_args_t even though its an update case
    // since the structures are the same. Can be unified in the future
    ret = p4pd_add_upd_flow_hash_table_entries(session_pd,
                                              (pd_session_create_args_t *)args,
                                               true);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Flow hash table entry upd failure");
        goto cleanup;
    }

    return HAL_RET_OK;

cleanup:

    if (session_pd) {
        p4pd_del_flow_stats_table_entries(session_pd);
        if (args->session->conn_track_en)
            p4pd_del_session_state_table_entry(session_pd->session_state_idx);
        p4pd_del_flow_info_table_entries(session_pd);
        session_pd_free(session_pd);
        args->session->pd = NULL;
    }

    return ret;
}

//-----------------------------------------
// Delete PD tables for given session
//------------------------------------------
hal_ret_t
pd_session_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t             ret = HAL_RET_OK;
    pd_session_delete_args_t *args = pd_func_args->pd_session_delete;
    pd_session_t         *session_pd = NULL;
    pd_func_args_t        get_func_args = {0};
    pd_session_get_args_t session_get_args = {0};

    HAL_TRACE_VERBOSE("Deleting pd state for session");

    session_get_args.session = args->session;
    session_get_args.session_state = args->session_state;
    get_func_args.pd_session_get = &session_get_args;
    pd_session_get(&get_func_args);

    session_pd = args->session->pd;

    SDK_ASSERT(session_pd != NULL);

    // del flow stats entries first
    ret = p4pd_del_flow_stats_table_entries(session_pd);
    SDK_ASSERT(ret == HAL_RET_OK);

    // Del session state
    if (args->session->conn_track_en) {
        ret = p4pd_del_session_state_table_entry(session_pd->session_state_idx);
        if (session_pd->session_state_idx)
            SDK_ASSERT(ret == HAL_RET_OK);
    }

    // del flow info table entries
    ret = p4pd_del_flow_info_table_entries(session_pd);
    SDK_ASSERT(ret == HAL_RET_OK);

    // del flow hash table entries
    ret = p4pd_del_flow_hash_table_entries(session_pd);
    //SDK_ASSERT(ret == HAL_RET_OK);

    session_pd_free(session_pd);
    args->session->pd = NULL;

    return ret;
}

//-----------------------------------------
// get all session related information
//------------------------------------------
hal_ret_t
pd_session_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_session_get_args_t *args = pd_func_args->pd_session_get;
    pd_func_args_t pd_func_args1 = {0};
    sdk_ret_t sdk_ret;
    session_state_actiondata_t d = {0};
    sldirectmap *session_state_table = NULL;
    session_state_t *ss = args->session_state;
    pd_flow_get_args_t flow_get_args;
    pd_session_t *pd_session;
    session_state_tcp_session_state_info_t info;
    sdk_table_api_params_t   params;

    if (args->session == NULL || args->session->pd == NULL) {
        return HAL_RET_INVALID_ARG;
    }
    pd_session = args->session->pd;

    if (args->session->conn_track_en) {
        session_state_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_SESSION_STATE);
        SDK_ASSERT(session_state_table != NULL);

        bzero(&params, sizeof(sdk_table_api_params_t));
        params.handle.pindex(pd_session->session_state_idx);
        params.actiondata = &d;
        sdk_ret = session_state_table->get(&params);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret == HAL_RET_OK) {
            info = d.action_u.session_state_tcp_session_state_info;
            ss->tcp_ts_option = info.tcp_ts_option_negotiated;
            ss->tcp_sack_perm_option = info.tcp_sack_perm_option_negotiated;

            // Initiator flow specific information
            ss->iflow_state.state = (session::FlowTCPState)info.iflow_tcp_state;
            ss->iflow_state.tcp_seq_num = info.iflow_tcp_seq_num;
            ss->iflow_state.tcp_ack_num = info.iflow_tcp_ack_num;
            ss->iflow_state.tcp_win_sz = info.iflow_tcp_win_sz;
            ss->iflow_state.syn_ack_delta = info.syn_cookie_delta;
            ss->iflow_state.tcp_mss = info.iflow_tcp_mss;
            ss->iflow_state.tcp_win_scale = info.iflow_tcp_win_scale;
            ss->iflow_state.tcp_ws_option_sent = info.iflow_tcp_ws_option_sent;
            ss->iflow_state.tcp_ts_option_sent = info.iflow_tcp_ts_option_sent;
            ss->iflow_state.tcp_sack_perm_option_sent =
                                          info.iflow_tcp_sack_perm_option_sent;
            ss->iflow_state.exception_bmap = info.iflow_exceptions_seen;

            // Responder flow specific information
            ss->rflow_state.state =
              (session::FlowTCPState)info.rflow_tcp_state;
            ss->rflow_state.tcp_seq_num = info.rflow_tcp_seq_num;
            ss->rflow_state.tcp_ack_num = info.rflow_tcp_ack_num;
            ss->rflow_state.tcp_win_sz = info.rflow_tcp_win_sz;
            ss->rflow_state.tcp_mss = info.rflow_tcp_mss;
            ss->rflow_state.tcp_win_scale = info.rflow_tcp_win_scale;
            ss->rflow_state.exception_bmap = info.rflow_exceptions_seen;
        }
    }

    flow_get_args.pd_session = pd_session;
    flow_get_args.role = FLOW_ROLE_INITIATOR;
    flow_get_args.flow_state = &ss->iflow_state;
    flow_get_args.aug = false;
    pd_func_args1.pd_flow_get = &flow_get_args;
    ret = pd_flow_get(&pd_func_args1);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Initiator Flow get failed session {}",
                      args->session->hal_handle);
        return ret;
    }

    flow_get_args.pd_session = pd_session;
    flow_get_args.role = FLOW_ROLE_RESPONDER;
    flow_get_args.flow_state = &ss->rflow_state;
    flow_get_args.aug = false;
    pd_func_args1.pd_flow_get = &flow_get_args;
    ret = pd_flow_get(&pd_func_args1);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Responder Flow get failed session {}",
                      args->session->hal_handle);
        return ret;
    }

    if (pd_session->iflow_aug.valid) {
        flow_get_args.pd_session = pd_session;
        flow_get_args.role = FLOW_ROLE_INITIATOR;
        flow_get_args.flow_state = &ss->iflow_aug_state;
        flow_get_args.aug = true;
        pd_func_args1.pd_flow_get = &flow_get_args;
        ret = pd_flow_get(&pd_func_args1);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Initiator Aug Flow get failed session {}",
                          args->session->hal_handle);
            return ret;
        }
    }

    if (pd_session->rflow_aug.valid) {
        flow_get_args.pd_session = pd_session;
        flow_get_args.role = FLOW_ROLE_RESPONDER;
        flow_get_args.flow_state = &ss->rflow_aug_state;
        flow_get_args.aug = true;
        pd_func_args1.pd_flow_get = &flow_get_args;
        ret = pd_flow_get(&pd_func_args1);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Responder Aug Flow get failed session {}",
                          args->session->hal_handle);
            return ret;
        }
    }

    return HAL_RET_OK;
}

hal_ret_t
pd_session_age_reset (pd_func_args_t *pd_func_args)
{
    pd_conv_sw_clock_to_hw_clock_args_t clock_args;
    pd_session_age_reset_args_t         *args = pd_func_args->pd_session_age_reset;
    session_t                           *session = args->session;
    pd_session_t                        *session_pd = (session ? session->pd : NULL);
    pd_func_args_t                      pd_clock_fn_args = {0};
    timespec_t                          ts;
    uint64_t                            sw_ns = 0, clock = 0; 

    if (session_pd == NULL) {
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_VERBOSE("Session:{} iAugVld:{} rVld:{} rAugVld:{} iID:{} iAugID:{} rID:{} rAugID:{}",
                      session->hal_handle, session_pd->iflow_aug.valid, session_pd->rflow.valid,
                      session_pd->rflow_aug.valid, session_pd->iflow.assoc_hw_id,
                      session_pd->iflow_aug.assoc_hw_id, session_pd->rflow.assoc_hw_id,
                      session_pd->rflow_aug.assoc_hw_id);

    // Get the hw clock
    clock_gettime(CLOCK_MONOTONIC, &ts);
    sdk::timestamp_to_nsecs(&ts, &sw_ns);
    clock_args.hw_tick = &clock;
    clock_args.sw_ns   = sw_ns;
    pd_clock_fn_args.pd_conv_sw_clock_to_hw_clock = &clock_args;
    pd_conv_sw_clock_to_hw_clock(&pd_clock_fn_args);

    p4pd_flow_stats_table_entry_last_seen_ts(session_pd->iflow.assoc_hw_id, clock);

    if (session_pd->iflow_aug.valid) {
        p4pd_flow_stats_table_entry_last_seen_ts(session_pd->iflow_aug.assoc_hw_id, clock);
    }
    if (session_pd->rflow.valid) {
        p4pd_flow_stats_table_entry_last_seen_ts(session_pd->rflow.assoc_hw_id, clock);
    }
    if (session_pd->rflow_aug.valid) {
        p4pd_flow_stats_table_entry_last_seen_ts(session_pd->rflow_aug.assoc_hw_id, clock);
    }

    return HAL_RET_OK;
}
 
typedef struct flow_atomic_stats_ {
    uint64_t permit_bytes:64;
    uint64_t permit_packets:64;
    uint64_t drop_bytes:64;
    uint64_t drop_packets:64;
} __PACK__ flow_atomic_stats_t;

//------------------------------------------------------------------------------
// get all flow related information
//------------------------------------------------------------------------------
hal_ret_t
pd_flow_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t ret = HAL_RET_OK;
    pd_conv_hw_clock_to_sw_clock_args_t clock_args = {0};
    pd_flow_get_args_t *args = pd_func_args->pd_flow_get;
    sdk_ret_t sdk_ret;
    flow_stats_actiondata_t d = {0};
    flow_info_actiondata_t f = {0};
    sldirectmap *info_table = NULL;
    sldirectmap *stats_table = NULL;
    pd_flow_t pd_flow;
    session_t *session = NULL;
    sdk_table_api_params_t params;

    if (args->pd_session == NULL ||
        ((pd_session_t *)args->pd_session)->session == NULL) {
        return HAL_RET_INVALID_ARG;
    }

    session = (session_t *)((pd_session_t *)args->pd_session)->session;
    stats_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_FLOW_STATS);
    SDK_ASSERT(stats_table != NULL);

    if (args->aug == false) {
        if (args->role == FLOW_ROLE_INITIATOR) {
            pd_flow = args->pd_session->iflow;
        } else {
            pd_flow = args->pd_session->rflow;
        }
    } else {
        if (args->role == FLOW_ROLE_INITIATOR) {
            pd_flow = args->pd_session->iflow_aug;
        } else {
            pd_flow = args->pd_session->rflow_aug;
        }
    }

    bzero(&params, sizeof(sdk_table_api_params_t));
    params.handle.pindex(pd_flow.assoc_hw_id);
    params.actiondata = &d;
    // read the d-vector
    sdk_ret = stats_table->get(&params);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Error reading stats action entry flow {} hw-id {} ret {}",
                      session->hal_handle, pd_flow.assoc_hw_id, ret);
        return ret;
    }

    args->flow_state->packets = BYTES_TO_UINT64(d.action_u.flow_stats_flow_stats.permit_packets);
    args->flow_state->bytes = BYTES_TO_UINT64(d.action_u.flow_stats_flow_stats.permit_bytes);
    args->flow_state->drop_packets = BYTES_TO_UINT64(d.action_u.flow_stats_flow_stats.drop_packets);
    args->flow_state->drop_bytes = BYTES_TO_UINT64(d.action_u.flow_stats_flow_stats.drop_bytes);

    clock_args.hw_tick = d.action_u.flow_stats_flow_stats.last_seen_timestamp;
    clock_args.hw_tick = (clock_args.hw_tick << 16);
    clock_args.sw_ns = &args->flow_state->last_pkt_ts;
    pd_func_args->pd_conv_hw_clock_to_sw_clock = &clock_args;
    pd_conv_hw_clock_to_sw_clock(pd_func_args);

    info_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_FLOW_INFO);
    SDK_ASSERT(info_table != NULL);

    bzero(&params, sizeof(sdk_table_api_params_t));
    params.handle.pindex(pd_flow.assoc_hw_id);
    params.actiondata = &f;
    sdk_ret = info_table->get(&params);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret == HAL_RET_OK) {
        if (f.action_id == FLOW_INFO_FLOW_HIT_DROP_ID) {
            clock_args.hw_tick = f.action_u.flow_info_flow_hit_drop.start_timestamp;

        } else {
            clock_args.hw_tick = f.action_u.flow_info_flow_info.start_timestamp;
        }
        clock_args.hw_tick = (clock_args.hw_tick << 16);
        clock_args.sw_ns = &args->flow_state->create_ts;
        pd_func_args->pd_conv_hw_clock_to_sw_clock = &clock_args;
        pd_conv_hw_clock_to_sw_clock(pd_func_args);
    }

    return ret;
}

// Optimized pd_flow_get() version for Flow-age thread to conserve 
// compute cycles
hal_ret_t
pd_flow_get_for_age_thread (pd_func_args_t *pd_func_args, flow_t *flow_p,
                            uint16_t current_age_timer_ticks)
{
    hal_ret_t                            ret;
    pd_conv_hw_clock_to_sw_clock_args_t  clock_args;
    pd_flow_get_args_t                  *args;
    flow_stats_actiondata_t              d;
    sldirectmap                         *stats_table;
    pd_flow_t                            pd_flow;
    flow_telemetry_state_t              *flow_telemetry_state_p;
    sdk_ret_t                            sdk_ret;
    timespec_t                           ctime;
    uint64_t                             ctime_ns;
    uint64_t                             current_hw_drop_packets;
    uint64_t                             pps = 0, bw = 0;
    uint64_t                             flow_table_packets, flow_table_bytes;
    uint32_t                             delta_packets, delta_bytes;
    sdk_table_api_params_t               params;

    args = pd_func_args->pd_flow_get;
    if (args->pd_session == NULL ||
        ((pd_session_t *)args->pd_session)->session == NULL) {
        return HAL_RET_INVALID_ARG;
    }

    if (args->role == FLOW_ROLE_INITIATOR) {
        pd_flow = args->pd_session->iflow;
    } else {
        pd_flow = args->pd_session->rflow;
    }

    // read the d-vector
    stats_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_FLOW_STATS);
    SDK_ASSERT(stats_table != NULL);

    bzero(&params, sizeof(sdk_table_api_params_t));
    params.handle.pindex(pd_flow.assoc_hw_id);
    params.actiondata = &d;
    sdk_ret = stats_table->get(&params);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        session_t *session;

        session = (session_t *)((pd_session_t *)args->pd_session)->session;
        HAL_TRACE_ERR(
        "Error reading stats action entry flow {} hw-id {} ret {}",
        session->hal_handle, pd_flow.assoc_hw_id, ret);
        return ret;
    }

    //Retrieve Last-seen-packet-timestamp in this Flow-context
    clock_args.hw_tick = d.action_u.flow_stats_flow_stats.last_seen_timestamp;
    clock_args.hw_tick = (clock_args.hw_tick << 16);
    clock_args.sw_ns = &args->flow_state->last_pkt_ts;
    pd_func_args->pd_conv_hw_clock_to_sw_clock = &clock_args;
    pd_conv_hw_clock_to_sw_clock(pd_func_args);

    //Flow-Proto-States are created dynamically on-demand based on the
    // drop_packets symptoms observed in Flow-stats-table D-vector
    //
    // Flow-Age thread creates such states in HBM (for exposition via gRPC)
    current_hw_drop_packets = BYTES_TO_UINT64(d.action_u.flow_stats_flow_stats.
                                              drop_packets);
    flow_telemetry_state_p = flow_p->flow_telemetry_state_p;
    if (flow_telemetry_state_p == NULL) {

        // Gather Flow Raw Stats in scratch buffer
        args->flow_state->packets = BYTES_TO_UINT64(d.action_u.
                                    flow_stats_flow_stats.permit_packets);
        args->flow_state->bytes = BYTES_TO_UINT64(d.action_u.
                                  flow_stats_flow_stats.permit_bytes);
        args->flow_state->drop_packets = current_hw_drop_packets;
        args->flow_state->drop_bytes = BYTES_TO_UINT64(d.action_u.
                                       flow_stats_flow_stats.drop_bytes);
        args->flow_state->exception_bmap = d.action_u.
                                           flow_stats_flow_stats.drop_reason;

        // Create FlowDropProto on first Non-firewall-policy flow-drop seen
        flow_p->flow_telemetry_create_flags = flow_p->
                                              flow_telemetry_enable_flags;
        if (current_hw_drop_packets == 0 ||
            d.action_u.flow_stats_flow_stats.drop_reason == 
           (1 << DROP_FLOW_HIT)) {
            flow_p->flow_telemetry_create_flags &= ~(1 << FLOW_TELEMETRY_DROP);
        }
        return ret;
    }

    // Extract Raw-Flow-stats from HW for exposition via gRPC
    clock_gettime(CLOCK_MONOTONIC, &ctime);
    sdk::timestamp_to_nsecs(&ctime, &ctime_ns);

    if (flow_p->flow_telemetry_enable_flags & (1 << FLOW_TELEMETRY_RAW)) {
        // Get current snapshot of Packets/Bytes
        flow_table_packets = BYTES_TO_UINT64(
                             d.action_u.flow_stats_flow_stats.permit_packets);
        flow_table_bytes = BYTES_TO_UINT64(
                           d.action_u.flow_stats_flow_stats.permit_bytes);

        // Compute Delta-Packets/Bytes since last capture
        delta_packets = (uint32_t) flow_table_packets - flow_telemetry_state_p->
                                   u1.raw_metrics.last_flow_table_packets;
        delta_bytes = (uint32_t) flow_table_bytes - flow_telemetry_state_p->
                                 u1.raw_metrics.last_flow_table_bytes;
        flow_telemetry_state_p->u1.raw_metrics.last_flow_table_packets =
                                (uint32_t) flow_table_packets;
        flow_telemetry_state_p->u1.raw_metrics.last_flow_table_bytes = 
                                (uint32_t) flow_table_bytes;

        // Packets / Bytes are accumulated to handle Telemetry-HBM-resource
        // Re-use case
        flow_telemetry_state_p->u1.raw_metrics.packets += delta_packets;
        flow_telemetry_state_p->u1.raw_metrics.bytes += delta_bytes;
    }

    // Extract Drop-Flow-stats from HW for exposition via gRPC
    if (current_hw_drop_packets) {
        uint32_t last_drops;

        // Update soft-drop-stats only if needed, i.e. when newer drops seen
        last_drops = flow_telemetry_state_p->u1.drop_metrics.
                                             last_flow_table_packets;
        if (last_drops != current_hw_drop_packets) {
            // Sense for first-time occurring drop-events
            if (flow_telemetry_state_p->u1.drop_metrics.packets == 0)
                flow_telemetry_state_p->u1.drop_metrics.first_timestamp = 
                                        ctime_ns;
            flow_telemetry_state_p->u1.drop_metrics.last_timestamp = ctime_ns;

            // Get current snapshot of Packets/Bytes
            flow_table_packets = BYTES_TO_UINT64(
                                 d.action_u.flow_stats_flow_stats.drop_packets);
            flow_table_bytes = BYTES_TO_UINT64(
                               d.action_u.flow_stats_flow_stats.drop_bytes);

            // Compute Delta-Packets/Bytes since last capture
            delta_packets = (uint32_t) flow_table_packets - 
            flow_telemetry_state_p->u1.drop_metrics.last_flow_table_packets;
            delta_bytes = (uint32_t) flow_table_bytes - flow_telemetry_state_p->
                                     u1.drop_metrics.last_flow_table_bytes;
            flow_telemetry_state_p->u1.drop_metrics.last_flow_table_packets =
                                    (uint32_t) flow_table_packets;
            flow_telemetry_state_p->u1.drop_metrics.last_flow_table_bytes =
                                    (uint32_t) flow_table_bytes;

            // Packets / Bytes are accumulated to handle Telemetry-HBM-resource
            // Re-use case
            flow_telemetry_state_p->u1.drop_metrics.packets += delta_packets;
            flow_telemetry_state_p->u1.drop_metrics.bytes += delta_bytes;
            flow_telemetry_state_p->u1.drop_metrics.reason |= 
                                   d.action_u.flow_stats_flow_stats.drop_reason;
        }
    }

    // Derive Performance-Flow-stats for exposition via gRPC, if enabled
    if (flow_p->flow_telemetry_enable_flags & (1<<FLOW_TELEMETRY_PERFORMANCE)) {
        if (flow_telemetry_state_p->u2.last_age_timer_ticks) {
            uint32_t flow_rate_compute_time;

            flow_rate_compute_time = current_age_timer_ticks - 
                                flow_telemetry_state_p->u2.last_age_timer_ticks;
            switch (flow_rate_compute_time) {
                case FLOW_RATE_COMPUTE_TIME_1SEC:
                    pps = (flow_telemetry_state_p->u1.raw_metrics.packets -
                           flow_telemetry_state_p->u1.raw_metrics.last_packets);
                    bw = (flow_telemetry_state_p->u1.raw_metrics.bytes - 
                          flow_telemetry_state_p->u1.raw_metrics.last_bytes);
                    break;
                case FLOW_RATE_COMPUTE_TIME_2SEC:
                    pps = (flow_telemetry_state_p->u1.raw_metrics.packets -
                      flow_telemetry_state_p->u1.raw_metrics.last_packets) >> 1;
                    bw = (flow_telemetry_state_p->u1.raw_metrics.bytes - 
                        flow_telemetry_state_p->u1.raw_metrics.last_bytes) >> 1;
                    break;
                case FLOW_RATE_COMPUTE_TIME_4SEC:
                    pps = (flow_telemetry_state_p->u1.raw_metrics.packets -
                      flow_telemetry_state_p->u1.raw_metrics.last_packets) >> 2;
                    bw = (flow_telemetry_state_p->u1.raw_metrics.bytes - 
                        flow_telemetry_state_p->u1.raw_metrics.last_bytes) >> 2;
                    break;
                case FLOW_RATE_COMPUTE_TIME_8SEC:
                    pps = (flow_telemetry_state_p->u1.raw_metrics.packets -
                      flow_telemetry_state_p->u1.raw_metrics.last_packets) >> 3;
                    bw = (flow_telemetry_state_p->u1.raw_metrics.bytes - 
                        flow_telemetry_state_p->u1.raw_metrics.last_bytes) >> 3;
                    break;
                default:
                    // Non-optimal case ==> divide operation
                    //
                    // For optimal cases, we do shift operations as above
                    pps = (flow_telemetry_state_p->u1.raw_metrics.packets -
                           flow_telemetry_state_p->u1.raw_metrics.last_packets)/
                           flow_rate_compute_time;
                    bw = (flow_telemetry_state_p->u1.raw_metrics.bytes - 
                          flow_telemetry_state_p->u1.raw_metrics.last_bytes) / 
                          flow_rate_compute_time;
                    break;
            }

            if (pps > flow_telemetry_state_p->u1.performance_metrics.peak_pps) {
                flow_telemetry_state_p->u1.performance_metrics.peak_pps = pps;
                flow_telemetry_state_p->u1.performance_metrics.
                                        peak_pps_timestamp = ctime_ns;
            }
            if (bw > flow_telemetry_state_p->u1.performance_metrics.peak_bw) {
                flow_telemetry_state_p->u1.performance_metrics.peak_bw = bw;
                flow_telemetry_state_p->u1.performance_metrics.
                                        peak_bw_timestamp = ctime_ns;
            }
        }
        flow_telemetry_state_p->u1.raw_metrics.last_packets = 
                                flow_telemetry_state_p->u1.raw_metrics.packets;
        flow_telemetry_state_p->u1.raw_metrics.last_bytes = 
                                flow_telemetry_state_p->u1.raw_metrics.bytes;
        flow_telemetry_state_p->u2.last_age_timer_ticks = 
                                current_age_timer_ticks;
    }
                
    // Derive Behavioral-Flow-stats for exposition via gRPC, if enabled
    if (flow_p->flow_telemetry_enable_flags & (1<<FLOW_TELEMETRY_BEHAVIORAL)) {

        if (pps > flow_telemetry_state_p->u1.behavioral_metrics.pps_threshold) {
            // Sense for first-time occurring event and update timestamp 
            // accordingly
            if (flow_telemetry_state_p->u1.behavioral_metrics.
                                        pps_threshold_exceed_events == 0)
                flow_telemetry_state_p->u1.behavioral_metrics.
                          pps_threshold_exceed_event_first_timestamp = ctime_ns;
          //flow_telemetry_state_p->u1.behavioral_metrics.
          //               pps_threshold_exceed_event_last_timestamp = ctime_ns;

            flow_telemetry_state_p->u1.behavioral_metrics.
                                    pps_threshold_exceed_events++;
        }

        if (bw > flow_telemetry_state_p->u1.behavioral_metrics.bw_threshold) {
            // Sense for first-time occurring event and update timestamp 
            // accordingly
            if (flow_telemetry_state_p->u1.behavioral_metrics.
                                        bw_threshold_exceed_events == 0)
                flow_telemetry_state_p->u1.behavioral_metrics.
                           bw_threshold_exceed_event_first_timestamp = ctime_ns;
          //flow_telemetry_state_p->u1.behavioral_metrics.
          //                bw_threshold_exceed_event_last_timestamp = ctime_ns;

            flow_telemetry_state_p->u1.behavioral_metrics.
                                    bw_threshold_exceed_events++;
        }
    }

    // Support on-the-fly FlowStats Telemetry Disablement
    flow_p->flow_telemetry_create_flags |= 
    ~flow_p->flow_telemetry_enable_flags&flow_telemetry_state_p->present_flags;

    return ret;
}

// Optimized pd_session_get() version for Flow-age thread to conserve 
// compute cycles
hal_ret_t
pd_session_get_for_age_thread (pd_func_args_t *pd_func_args)
{
    hal_ret_t                               ret;
    sdk_ret_t                               sdk_ret;
    pd_session_get_args_t                  *args;
    pd_session_t                           *pd_session;
    pd_flow_get_args_t                      flow_get_args;
    pd_func_args_t                          pd_func_args1;
    session_state_actiondata_t              d;
    sldirectmap                            *session_state_table;
    session_state_t                        *ss;
    session_state_tcp_session_state_info_t  info;
    sdk_table_api_params_t                 params;

    args = pd_func_args->pd_session_get;
    if (args->session == NULL || args->session->pd == NULL) {
        return HAL_RET_INVALID_ARG;
    }
    pd_session = args->session->pd;

    ss = args->session_state;
    if (args->session->conn_track_en) {
        session_state_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_SESSION_STATE);
        SDK_ASSERT(session_state_table != NULL);

        bzero(&params, sizeof(sdk_table_api_params_t));
        params.handle.pindex(pd_session->session_state_idx);
        params.actiondata = &d;
        sdk_ret = session_state_table->get(&params);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret == HAL_RET_OK) {
            info = d.action_u.session_state_tcp_session_state_info;
            ss->iflow_state.state = (session::FlowTCPState)info.iflow_tcp_state;
            ss->rflow_state.state = (session::FlowTCPState)info.rflow_tcp_state;
        }
    }

    flow_get_args.pd_session = pd_session;
    flow_get_args.role = FLOW_ROLE_INITIATOR;
    flow_get_args.flow_state = &ss->iflow_state;
    flow_get_args.aug = false;
    pd_func_args1.pd_flow_get = &flow_get_args;
    ret = pd_flow_get_for_age_thread(&pd_func_args1, args->session->iflow,
                                     ss->current_age_timer_ticks);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Initiator Flow get failed session {}",
                      args->session->hal_handle);
        return ret;
    }

    if (args->session->rflow) {
        flow_get_args.pd_session = pd_session;
        flow_get_args.role = FLOW_ROLE_RESPONDER;
        flow_get_args.flow_state = &ss->rflow_state;
        flow_get_args.aug = false;
        pd_func_args1.pd_flow_get = &flow_get_args;
        ret = pd_flow_get_for_age_thread(&pd_func_args1, args->session->rflow,
                                         ss->current_age_timer_ticks);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Responder Flow get failed session {}",
                          args->session->hal_handle);
            return ret;
        }
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------------
// Add a bypass flow info entry
//------------------------------------------------------------------------------------
hal_ret_t
pd_add_cpu_bypass_flow_info (uint32_t *flow_info_hwid)
{
    timespec_t                           ts;
    pd_conv_sw_clock_to_hw_clock_args_t  clock_args;
    pd_func_args_t                       pd_clock_fn_args = {0};
    flow_info_actiondata_t               d = { 0};
    hal_ret_t                            ret = HAL_RET_OK;
    sdk_ret_t                            sdk_ret;
    sldirectmap                         *info_table = NULL;
    uint64_t                             sw_ns = 0, clock = 0;
    sdk_table_api_params_t               params;

    // Get the hw clock
    clock_gettime(CLOCK_MONOTONIC, &ts);
    sdk::timestamp_to_nsecs(&ts, &sw_ns);
    clock_args.hw_tick = &clock;
    clock_args.sw_ns = sw_ns;
    pd_clock_fn_args.pd_conv_sw_clock_to_hw_clock = &clock_args;
    pd_conv_sw_clock_to_hw_clock(&pd_clock_fn_args);

    ret = p4pd_add_upd_flow_stats_table_entry(flow_info_hwid, 0, 0, 0, 0, clock, false);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    info_table = (sldirectmap *)g_hal_state_pd->dm_table(P4TBL_ID_FLOW_INFO);
    SDK_ASSERT(info_table != NULL);

    d.action_id = FLOW_INFO_FLOW_INFO_FROM_CPU_ID;

    bzero(&params, sizeof(sdk_table_api_params_t));
    params.handle.pindex(*flow_info_hwid);
    params.actiondata = &d;
    // insert the entry
    sdk_ret = info_table->insert_withid(&params);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("flow info table write failure, idx : {}, err : {}",
                       *flow_info_hwid, ret);
        return ret;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------------
// Get bypass flow info entry
//------------------------------------------------------------------------------------
hal_ret_t
pd_get_cpu_bypass_flowid (pd_func_args_t *pd_func_args)
{
    uint32_t                             flow_info_hwid = 0;
    pd_get_cpu_bypass_flowid_args_t     *args = pd_func_args->pd_get_cpu_bypass_flowid;

    if (!g_hal_state_pd->cpu_bypass_flowid()) {
        pd_add_cpu_bypass_flow_info(&flow_info_hwid);
        g_hal_state_pd->set_cpu_bypass_flowid(flow_info_hwid);
    }

    args->hw_flowid = g_hal_state_pd->cpu_bypass_flowid();

    return HAL_RET_OK;
}

// 
// Read the flow hash table given flow key
//
hal_ret_t
pd_flow_hash_get (pd_func_args_t *pd_func_args) {
    pd_flow_hash_get_args_t *args = pd_func_args->pd_flow_hash_get;
    hal_ret_t ret = HAL_RET_OK;
    flow_hash_info_entry_t key;

    key.clear();
    p4pd_fill_flow_hash_key(&args->key,
            args->lkp_inst, key);

    HAL_TRACE_VERBOSE("Hash entry {}", hex_str((uint8_t*)&key, sizeof(flow_hash_info_entry_t)));

    ret = g_hal_state_pd->flow_table_pd_get()->get(&key, args->rsp);
    if (ret != HAL_RET_OK) {
        return HAL_RET_OK;
    }

    return ret;
}

}    // namespace pd
}    // namespace hal
