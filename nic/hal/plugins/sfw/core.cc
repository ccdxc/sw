#include "nic/fte/fte.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/src/nwsec_group.hpp"
#include "nic/hal/src/nwsec.hpp"
#include "core.hpp"

namespace hal {
namespace plugins {
namespace sfw {

#define DEFAULT_MSS 536 // RFC6691
#define DEFAULT_WINDOW_SIZE 512

hal_ret_t
net_sfw_match_rules(fte::ctx_t                  &ctx,
                    nwsec_policy_rules_t        *nwsec_plcy_rules,
                    net_sfw_match_result_t *match_rslt)
{
    flow_key_t          flow_key;
    nwsec_policy_svc_t  *nwsec_plcy_svc = NULL, *matched_svc = NULL;
    dllist_ctxt_t       *lnode = NULL;

    flow_key = ctx.key();
    HAL_TRACE_DEBUG(" proto {} dport {}", flow_key.proto, flow_key.dport);
    dllist_for_each(lnode, &nwsec_plcy_rules->fw_svc_list_head) {
        nwsec_plcy_svc = dllist_entry(lnode, nwsec_policy_svc_t, lentry);
        if (nwsec_plcy_svc != NULL) {
            //ToDo (lseshan) - identify the wildcard better way
            if (nwsec_plcy_svc->ipproto == 0) {
                // Match all
                matched_svc = nwsec_plcy_svc;
                HAL_TRACE_DEBUG("Wild card Rule match {} {}", match_rslt->action, match_rslt->log);
                break;
                
            } else if (nwsec_plcy_svc->ipproto == flow_key.proto) {        //Compare proto
                if (nwsec_plcy_svc->ipproto == IPPROTO_ICMP ||
                    nwsec_plcy_svc->ipproto == IPPROTO_ICMPV6) {
                    //Check the icmp message type
                    if (nwsec_plcy_svc->icmp_msg_type == flow_key.icmp_type)
                    {
                        matched_svc = nwsec_plcy_svc;
                        break;
                    }
                } else {
                    //Check the port match
                    if (nwsec_plcy_svc->dst_port == flow_key.dport) {
                        matched_svc = nwsec_plcy_svc;
                        break;
                    }
                }
            }
        }
    }

    // TODO: Can nwsec_plcy_svc be NULL?
    if(!nwsec_plcy_svc || matched_svc) { // svc is wildcard or matched a specific service
        dllist_ctxt_t *lnode2 = NULL;
        nwsec_policy_appid_t *appid_policy = NULL;
        if(!dllist_empty(&nwsec_plcy_rules->appid_list_head)) {
            dllist_for_each(lnode2, &nwsec_plcy_rules->appid_list_head) {
                appid_policy = dllist_entry(lnode2, nwsec_policy_appid_t, lentry);
                if(appid_policy) {
                    if(!ctx.appid_started()) {
                        ctx.set_appid_needed();
                        return HAL_RET_OK;
                    }

                    // Phase II invocation of dfw in flow miss pipeline or phase I invocation of dfw in l7 flow-hit pipeline
                    hal::appid_info_t appid_info = ctx.appid_info();
                    for(int i = 0; i < appid_info.id_count_; i++) {
                        if(appid_policy->appid == appid_info.ids_[i]) {
                            match_rslt->valid  = 1;
                            if(matched_svc) match_rslt->alg = matched_svc->alg;
                            match_rslt->action = (session::FlowAction)nwsec_plcy_rules->action;
                            match_rslt->log    = nwsec_plcy_rules->log;
                            return HAL_RET_OK;
                        }
                    }

                }
            }
        } else if (matched_svc) {
            if(!ctx.appid_started()) ctx.set_appid_not_needed();
            match_rslt->valid  = 1;
            match_rslt->alg = matched_svc->alg;
            match_rslt->action = (session::FlowAction)nwsec_plcy_rules->action;
            match_rslt->log    = nwsec_plcy_rules->log;
            return HAL_RET_OK;
        }
    }

    return HAL_RET_FTE_RULE_NO_MATCH;
}

hal_ret_t
net_sfw_check_policy_pair(fte::ctx_t                    &ctx,
                          uint32_t                      src_sg,
                          uint32_t                      dst_sg,
                          net_sfw_match_result_t   *match_rslt)
{
    nwsec_policy_cfg_t    *nwsec_plcy_cfg = NULL;
    dllist_ctxt_t         *lnode = NULL;
    nwsec_policy_rules_t  *nwsec_plcy_rules = NULL;
    hal_ret_t             ret = HAL_RET_OK;

    nwsec_policy_key_t plcy_key;

    plcy_key.sg_id = src_sg;
    plcy_key.peer_sg_id = dst_sg;

    HAL_TRACE_DEBUG("fte::Lookup Policy for SG Pair: {} {}", src_sg, dst_sg);

    nwsec_plcy_cfg = nwsec_policy_cfg_lookup_by_key(plcy_key);
    if (nwsec_plcy_cfg == NULL) {
        HAL_TRACE_ERR("Failed to get the src_sg: {} peer_sg {}", src_sg, dst_sg);
        return HAL_RET_SG_NOT_FOUND;
    }

    dllist_for_each(lnode, &nwsec_plcy_cfg->rules_head) {
        nwsec_plcy_rules = dllist_entry(lnode, nwsec_policy_rules_t, lentry);
        ret = net_sfw_match_rules(ctx, nwsec_plcy_rules, match_rslt);
        if (ret == HAL_RET_OK && match_rslt->valid) {
            return ret;
        }
    }
    return ret;
}

hal_ret_t
net_sfw_pol_check_sg_policy(fte::ctx_t                  &ctx,
                            net_sfw_match_result_t *match_rslt)
{
    ep_t        *sep = NULL;
    ep_t        *dep = NULL;
    hal_ret_t    ret;


    if (ctx.protobuf_request()) {
        match_rslt->valid  = 1;
        match_rslt->action = ctx.sess_spec()->initiator_flow().flow_data().flow_info().flow_action();
        return HAL_RET_OK;
    }

    if (ctx.drop_flow()) {
        match_rslt->valid = 1;
        match_rslt->action = session::FLOW_ACTION_DROP;
        return HAL_RET_OK;
    }

    //ToDo (lseshan) - For now if a sep or dep is not found allow
    // Eventually use prefix to find the sg
    if (ctx.sep() == NULL ||  (ctx.dep() == NULL)) {
        match_rslt->valid = 1;
        match_rslt->action = session::FLOW_ACTION_ALLOW;
        return HAL_RET_OK;
    }

    sep  = ctx.sep();
    dep  = ctx.dep();
    for (int i = 0; i < sep->sgs.sg_id_cnt; i++) {
        for (int j = 0; j < dep->sgs.sg_id_cnt; j++) {
            ret = net_sfw_check_policy_pair(ctx,
                                            sep->sgs.arr_sg_id[i],
                                            dep->sgs.arr_sg_id[j],
                                            match_rslt);
            if (ret == HAL_RET_OK) {
                if (match_rslt->valid) {
                    return ret;
                }
            } else {
                // Handle error case
            }
        }
    }

    if(ctx.flow_miss()) {
        if(!ctx.appid_started())
            ctx.set_appid_not_needed();
    }
    // ToDo (lseshan) Handle SP miss condition
    // For now hardcoding to ALLOW but we have read default action and act
    // accordingly
    match_rslt->valid = 1;
    match_rslt->action = session::FLOW_ACTION_ALLOW;

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// extract all the TCP related state from session spec
//------------------------------------------------------------------------------
static hal_ret_t
net_sfw_extract_session_state_from_spec (fte::flow_state_t *flow_state,
                                 const session::FlowData& flow_data)
{
    auto conn_track_info = flow_data.conn_track_info();
    flow_state->state = flow_data.flow_info().tcp_state();
    flow_state->tcp_seq_num = conn_track_info.tcp_seq_num();
    flow_state->tcp_ack_num = conn_track_info.tcp_ack_num();
    flow_state->tcp_win_sz = conn_track_info.tcp_win_sz();
    flow_state->tcp_win_scale = conn_track_info.tcp_win_scale();
    flow_state->tcp_mss = conn_track_info.tcp_mss();
    flow_state->create_ts = conn_track_info.flow_create_ts();
    flow_state->last_pkt_ts = flow_state->create_ts;
    flow_state->packets = conn_track_info.flow_packets();
    flow_state->bytes = conn_track_info.flow_bytes();
    flow_state->exception_bmap = conn_track_info.exception_bits();

    return HAL_RET_OK;
}


static inline bool
net_sfw_conn_tracking_configured(fte::ctx_t &ctx)
{

    if (ctx.protobuf_request()) {
        return ctx.sess_spec()->conn_track_en();
    }

    if (ctx.key().proto != types::IPPROTO_TCP) {
        return false;
    }

    // lookup Security profile
    if (ctx.vrf()->nwsec_profile_handle  != HAL_HANDLE_INVALID) {
        hal::nwsec_profile_t  *nwsec_prof =
            find_nwsec_profile_by_handle(ctx.vrf()->nwsec_profile_handle);
        if (nwsec_prof != NULL) {
            return nwsec_prof->cnxn_tracking_en;
        }
    }
    return false;
}

fte::pipeline_action_t
sfw_exec(fte::ctx_t& ctx)
{
    hal_ret_t               ret;
    net_sfw_match_result_t  match_rslt = {};
    sfw_info_t              *sfw_info = (sfw_info_t*)ctx.feature_state();

    // security policy action
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_ACTION};

    // If appid_needed is set, we are in phase2 invocation of dfw
    // We need to proceed further only if appid feature has some new data or it has reached a terminal state
    if(ctx.appid_started() && 
       (!ctx.appid_updated() && !ctx.appid_completed())) {
        return fte::PIPELINE_CONTINUE;
    }

    // ToDo (lseshan) - for now handling only ingress rules
    // Need to select SPs based on the flow direction
    HAL_TRACE_DEBUG("Firewall lookup {}", (sfw_info->skip_sfw)?"skipped":"begin");
    if (ctx.role() == hal::FLOW_ROLE_INITIATOR && !sfw_info->skip_sfw && !sfw_info->sfw_done) {
        ret = net_sfw_pol_check_sg_policy(ctx, &match_rslt);
        if (ret == HAL_RET_OK) {
            if (match_rslt.valid) {
                flowupd.action  = match_rslt.action;
                sfw_info->alg_proto = match_rslt.alg;
                sfw_info->sfw_done = true;
                //ctx.log         = match_rslt.log;
            } else {
                // ToDo ret value was ok but match_rslt.valid is 0
                // to handle the case if it happens
            }
        }
    } else {
        //Responder Role: Not checking explicitly
        if (ctx.drop_flow()) {
            flowupd.action = session::FLOW_ACTION_DROP;
        } else {
            flowupd.action = session::FLOW_ACTION_ALLOW;
        }
    }

    ret = ctx.update_flow(flowupd);
    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    //Todo(lseshan) Move connection tracking to different function
    // connection tracking
    if (!net_sfw_conn_tracking_configured(ctx)) {
        return fte::PIPELINE_CONTINUE;
    }

    //iflow
    flowupd = {type: fte::FLOWUPD_FLOW_STATE};

    if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
        if (ctx.protobuf_request()) {
            net_sfw_extract_session_state_from_spec(&flowupd.flow_state,
                                            ctx.sess_spec()->initiator_flow().flow_data());
            flowupd.flow_state.syn_ack_delta = ctx.sess_spec()->iflow_syn_ack_delta();
        } else {
            const fte::cpu_rxhdr_t *rxhdr = ctx.cpu_rxhdr();
            flowupd.flow_state.state = session::FLOW_TCP_STATE_INIT;
            // Expectation is to program the next expected seq num.
            flowupd.flow_state.tcp_seq_num = ntohl(rxhdr->tcp_seq_num) + 1;
            flowupd.flow_state.tcp_ack_num = ntohl(rxhdr->tcp_ack_num);
            flowupd.flow_state.tcp_win_sz = ntohs(rxhdr->tcp_window);

            // TCP Options
            if (rxhdr->flags & CPU_FLAGS_TCP_OPTIONS_PRESENT) {
                // MSS Option
                flowupd.flow_state.tcp_mss = DEFAULT_MSS;
                if (rxhdr->tcp_options & CPU_TCP_OPTIONS_MSS) {
                    flowupd.flow_state.tcp_mss = ntohs(rxhdr->tcp_mss);
                }

                // Window Scale option
                flowupd.flow_state.tcp_ws_option_sent = 0;
                if (rxhdr->tcp_options & CPU_TCP_OPTIONS_WINDOW_SCALE) {
                    flowupd.flow_state.tcp_ws_option_sent = 1;
                    flowupd.flow_state.tcp_win_scale = rxhdr->tcp_ws;
                }

                // timestamp option
                flowupd.flow_state.tcp_ts_option_sent = 0;
                if (rxhdr->tcp_options & CPU_TCP_OPTIONS_TIMESTAMP) {
                    flowupd.flow_state.tcp_ts_option_sent = 1;
                }
                // sack_permitted option
                flowupd.flow_state.tcp_sack_perm_option_sent = 0;
                if (rxhdr->tcp_options & CPU_TCP_OPTIONS_SACK_PERMITTED) {
                    flowupd.flow_state.tcp_sack_perm_option_sent = 1;
                }
                
            }
        }
    } else { /* rflow */
        if (ctx.protobuf_request()) {
            net_sfw_extract_session_state_from_spec(&flowupd.flow_state,
                                            ctx.sess_spec()->responder_flow().flow_data());
        } else {
            flowupd.flow_state.state = session::FLOW_TCP_STATE_INIT;
            flowupd.flow_state.tcp_mss = DEFAULT_MSS;
            flowupd.flow_state.tcp_win_sz = DEFAULT_WINDOW_SIZE; //This is to allow the SYN Packet to go through.
        }
    }

    ret = ctx.update_flow(flowupd);
    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

}  // namespace sfw
}  // namespace plugins
}  // namespace hal
