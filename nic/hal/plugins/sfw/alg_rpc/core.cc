//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------


#include "core.hpp"
#include "nic/include/hal_mem.hpp"
#include "nic/hal/plugins/sfw/core.hpp"

namespace hal {
namespace plugins {
namespace alg_rpc {

using namespace hal::plugins::sfw;
using namespace hal::plugins::alg_utils;

void incr_parse_error(rpc_info_t *info) {
    SDK_ATOMIC_INC_UINT32(&info->parse_errors, 1);
}

void incr_data_sess(rpc_info_t *info) {
    SDK_ATOMIC_INC_UINT32(&info->data_sess, 1);
}

void incr_max_pkt_sz(rpc_info_t  *info) {
    SDK_ATOMIC_INC_UINT32(&info->maxpkt_sz_exceeded, 1);
}

void incr_num_exp_flows(rpc_info_t *info) {
    SDK_ATOMIC_INC_UINT32(&info->num_exp_flows, 1);
}

/*
 * APP Session get handler
 */
fte::pipeline_action_t alg_rpc_session_get_cb(fte::ctx_t &ctx) {
    fte::feature_session_state_t  *alg_state = NULL;
    SessionGetResponse            *sess_resp = ctx.sess_get_resp();
    l4_alg_status_t               *l4_sess =  NULL;

    if (!ctx.sess_get_resp() || ctx.role() != hal::FLOW_ROLE_INITIATOR)
        return fte::PIPELINE_CONTINUE;

    alg_state = ctx.feature_session_state();
    if (alg_state == NULL)
        return fte::PIPELINE_CONTINUE;

    l4_sess = (l4_alg_status_t *)alg_status(alg_state);
    if (l4_sess == NULL || (l4_sess->alg != nwsec::APP_SVC_SUN_RPC &&
        l4_sess->alg != nwsec::APP_SVC_MSFT_RPC))
        return fte::PIPELINE_CONTINUE;

    sess_resp->mutable_status()->set_alg(l4_sess->alg);

    if (l4_sess->isCtrl == true) {
        rpc_info_t *info = ((rpc_info_t *)l4_sess->info);
        if (info) {
            sess_resp->mutable_status()->mutable_rpc_info()->\
                                 set_parse_error(info->parse_errors);
            sess_resp->mutable_status()->mutable_rpc_info()->\
                                 set_num_data_sess(info->data_sess);
            sess_resp->mutable_status()->mutable_rpc_info()->\
                    set_maxpkt_size_exceeded(info->maxpkt_sz_exceeded);
            sess_resp->mutable_status()->mutable_rpc_info()->\
                                 set_num_exp_flows(info->num_exp_flows);
        }
        sess_resp->mutable_status()->mutable_rpc_info()->\
                                set_iscontrol(true);
    } else {
        sess_resp->mutable_status()->mutable_rpc_info()->\
                                set_iscontrol(false);
    }

    return fte::PIPELINE_CONTINUE;
}

/*
 *  Session delete handler
 *
 *  If it is a data session, go ahead and cleanup the L4 session.
 *  Check if there are no more data session/expected flows hanging
 *  off of the app session. If there is none, cleanup the app session
 *
 *  If it is the ctrl session:
 *  (1) check if there are no data session or expected flows. If so,
 *  cleanup app session.
 *  (2) Check if we got here due to TCP FIN/RST being received. If so,
 *  we want the HAL session cleanup to proceed without cleaning up the
 *  Ctrl session in ALG
 *  (3) If we got here due to session idle timout, then make sure that
 *  we dont clean up the HAL session as well.
 */
fte::pipeline_action_t alg_rpc_session_delete_cb(fte::ctx_t &ctx) {
    fte::feature_session_state_t  *alg_state = NULL;
    l4_alg_status_t               *l4_sess =  NULL;
    app_session_t                 *app_sess = NULL;

    if (ctx.role() != hal::FLOW_ROLE_INITIATOR)
        return fte::PIPELINE_CONTINUE;

    alg_state = ctx.feature_session_state();
    if (alg_state == NULL)
        return fte::PIPELINE_CONTINUE;

    l4_sess = (l4_alg_status_t *)alg_status(alg_state);
    if (l4_sess == NULL || (l4_sess->alg != nwsec::APP_SVC_SUN_RPC &&
        l4_sess->alg != nwsec::APP_SVC_MSFT_RPC))
        return fte::PIPELINE_CONTINUE;

    ctx.flow_log()->alg = l4_sess->alg;

    app_sess = l4_sess->app_session;
    if (l4_sess->isCtrl == true) {
        if (ctx.force_delete() == true|| (dllist_empty(&app_sess->exp_flow_lhead)\
             && dllist_count(&app_sess->l4_sess_lhead) == 1 &&
            ((l4_alg_status_t *)dllist_entry(app_sess->l4_sess_lhead.next,\
                             l4_alg_status_t, l4_sess_lentry)) == l4_sess)) {
            /*
             * Clean up app session if (a) its a force delete or
             * (b) if there are no expected flows or L4 data sessions
             * hanging off of this ctrl session.
             */
            g_rpc_state->cleanup_app_session(l4_sess->app_session);
            return fte::PIPELINE_CONTINUE;
        } else if ((ctx.key().proto==IP_PROTO_UDP) ||
                   (ctx.session()->iflow->state >= session::FLOW_TCP_STATE_FIN_RCVD) ||
                   (ctx.session()->rflow &&
                    (ctx.session()->rflow->state >= session::FLOW_TCP_STATE_FIN_RCVD))) {
            alg_utils::l4_alg_status_t   *ctrl_l4_sess = g_rpc_state->get_ctrl_l4sess(app_sess);

            /*
             * We received FIN/RST on the control session
             * we dont want to cleanup this one as we need this 
             * for firewall policy evaluation
             */
            ctx.set_feature_status(HAL_RET_INVALID_CTRL_SESSION_OP);
             // Mark this entry for deletion so we cleanup
             // when we clean up the data sessions
             if (ctrl_l4_sess)
                 ctrl_l4_sess->entry.deleting = true;
             return fte::PIPELINE_END;
        } else {
            /*
             * Dont cleanup if control session is timed out
             * we need to keep it around until the data session
             * goes away
             */
            ctx.set_feature_status(HAL_RET_INVALID_CTRL_SESSION_OP);
            return fte::PIPELINE_END;
        }
    }
    /*
     * Cleanup the data session that is getting timed out
     */
    g_rpc_state->cleanup_l4_sess(l4_sess);
    if (dllist_empty(&app_sess->exp_flow_lhead) &&
        dllist_count(&app_sess->l4_sess_lhead) == 1 &&
        ((l4_alg_status_t *)dllist_entry(app_sess->l4_sess_lhead.next,\
                l4_alg_status_t, l4_sess_lentry))->sess_hdl == HAL_HANDLE_INVALID) {
        alg_utils::l4_alg_status_t   *ctrl_l4_sess = g_rpc_state->get_ctrl_l4sess(app_sess);
        /*
         * If this was the last session hanging and there is no
         * HAL session for control session. This is the right time
         * to clean it
         */ 
        if (ctrl_l4_sess != NULL && ctrl_l4_sess->isCtrl == true &&
            ctrl_l4_sess->entry.deleting == true) {
            hal::session_t *session = hal::find_session_by_handle(ctrl_l4_sess->sess_hdl);

            if (session != NULL &&
                (session->iflow->config.key.proto == IP_PROTO_UDP ||
                 session->iflow->state == session::FLOW_TCP_STATE_BIDIR_FIN_RCVD)) {
                if (session->fte_id == fte::fte_id()) {
                    session_delete_in_fte(session->hal_handle);
                } else {
                    fte::session_delete_async(session);
                }
            }
        }
    } 

    return fte::PIPELINE_CONTINUE;
}

uint8_t *alloc_rpc_pkt(void) {
    return ((uint8_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_ALG, MAX_ALG_RPC_PKT_SZ));
}

#if 0
static inline void __str_to_uuid(char *uuid_str, uuid_t *uuid) {
    char *tok = NULL;
    uint16_t val = 0;

    tok = strtok(uuid_str, "-");
    uuid->time_lo = (tok != NULL)?strtoul(tok, NULL, 16):0;
    if (tok == NULL) return;
    tok = strtok(NULL, "-");
    uuid->time_mid = (tok != NULL)?strtoul(tok, NULL, 16):0;
    if (tok == NULL) return;
    tok = strtok(NULL, "-");
    uuid->time_hi_vers = (tok != NULL)?strtoul(tok, NULL, 16):0;
    if (tok == NULL) return;
    tok = strtok(NULL, "-");
    val = (tok != NULL)?strtoul(tok, NULL, 16):0;
    uuid->clock_seq_hi = (val & 0xFF00)>>8;
    uuid->clock_seq_lo = (val & 0xFF);
    tok = strtok(NULL, "-");
    if (tok == NULL) return;
    uint64_t node = strtoul(tok, NULL, 16);
    for (uint8_t idx=5; idx>=0; idx--) {
         uuid->node[idx] = node&0xFF;
         node = node >> 8;
    }
}
#endif

void copy_sfw_info(sfw_info_t *sfw_info, l4_alg_status_t *l4_sess) {
    uint32_t pgmid_list_sz = 0;
    rpc_info_t *rpc_info = (rpc_info_t *)l4_sess->info;

    l4_sess->idle_timeout = sfw_info->idle_timeout;
    if (sfw_info->alg_proto == nwsec::APP_SVC_SUN_RPC) {
        rpc_programid_t *rpc = NULL;
        rpc_info->pgmid_sz = sfw_info->alg_opts.opt.sunrpc_opts.programid_sz;
        pgmid_list_sz = (sizeof(hal::rpc_programid_t)*\
                          sfw_info->alg_opts.opt.sunrpc_opts.programid_sz);
        rpc_info->pgm_ids = (hal::rpc_programid_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_ALG, pgmid_list_sz);
        rpc = (hal::rpc_programid_t *)rpc_info->pgm_ids;
        for (uint8_t idx=0; idx<sfw_info->alg_opts.opt.sunrpc_opts.programid_sz; idx++) {
             rpc[idx].program_id = sfw_info->alg_opts.opt.sunrpc_opts.program_ids[idx].program_id;
             rpc[idx].timeout = sfw_info->alg_opts.opt.sunrpc_opts.program_ids[idx].timeout;
        }
    } else if (sfw_info->alg_proto == nwsec::APP_SVC_MSFT_RPC) {
        rpc_uuid_t *rpc = NULL;

        rpc_info->pgmid_sz = sfw_info->alg_opts.opt.msrpc_opts.uuid_sz;
        pgmid_list_sz = (sizeof(hal::rpc_uuid_t)*\
                                 sfw_info->alg_opts.opt.msrpc_opts.uuid_sz);
        rpc_info->pgm_ids = (rpc_uuid_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_ALG, pgmid_list_sz);
        rpc = (hal::rpc_uuid_t *)rpc_info->pgm_ids;
        for (uint8_t idx=0; idx<sfw_info->alg_opts.opt.msrpc_opts.uuid_sz; idx++) {
             memcpy(rpc[idx].uuid, sfw_info->alg_opts.opt.msrpc_opts.uuids[idx].uuid, UUID_SZ);
             rpc[idx].timeout = sfw_info->alg_opts.opt.msrpc_opts.uuids[idx].timeout;
        }
    }
}

/*
 * Expected flow callback. FTE issues this callback with the expected flow data
 */
hal_ret_t expected_flow_handler(fte::ctx_t &ctx, expected_flow_t *wentry) {
    l4_alg_status_t      *entry = NULL, *l4_sess = NULL;
    rpc_info_t           *rpc_info = NULL;
    sfw_info_t           *sfw_info = sfw::sfw_feature_state(ctx);
    hal_ret_t             ret = HAL_RET_OK;

    entry = (l4_alg_status_t *)wentry;
    rpc_info = (rpc_info_t *)entry->info;
    if (entry->isCtrl != true && sfw_info != NULL) {
        sfw_info->skip_sfw = rpc_info->skip_sfw;
        sfw_info->idle_timeout = entry->idle_timeout;
        HAL_TRACE_DEBUG("Expected flow handler - skip sfw {}", sfw_info->skip_sfw);
    }
    ctx.set_feature_name(FTE_FEATURE_ALG_RPC.c_str());
    ctx.register_feature_session_state(&entry->fte_feature_state);

    flow_update_t flowupd = {type: FLOWUPD_SFW_INFO};
    flowupd.sfw_info.skip_sfw_reval = 1;
    flowupd.sfw_info.sfw_rule_id = entry->rule_id;
    flowupd.sfw_info.sfw_action = (uint8_t)nwsec::SECURITY_RULE_ACTION_ALLOW;
    ret = ctx.update_flow(flowupd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to update sfw action");
    }

    ctx.flow_log()->sfw_action = nwsec::SECURITY_RULE_ACTION_ALLOW;
    ctx.flow_log()->rule_id = entry->rule_id;
    ctx.flow_log()->alg = entry->alg;
    l4_sess = g_rpc_state->get_ctrl_l4sess(entry->app_session);
    if (l4_sess)
        ctx.flow_log()->parent_session_id = l4_sess->sess_hdl;

    return HAL_RET_OK;
}

void insert_rpc_expflow(fte::ctx_t& ctx, l4_alg_status_t *l4_sess, rpc_cb_t cb,
                        uint32_t timeout) {
    hal::flow_key_t  key = ctx.key();
    rpc_info_t      *rpc_info = NULL, *exp_flow_info = NULL;
    l4_alg_status_t *exp_flow = NULL;
    hal_ret_t        ret = HAL_RET_OK;

    rpc_info = (rpc_info_t *)l4_sess->info;
    memset(&key.sip, 0, sizeof(ipvx_addr_t));
    /*
     * Reason we mask out the direction is that EPM
     * query could be made by one client and other client
     * could be using this pinhole to reach the server
     * If Naples is used for firewall, the clients could be
     * from outside or inside
     */
    key.sport = 0;
    key.dip = rpc_info->ip;
    key.dport = rpc_info->dport;
    key.proto = (types::IPProtocol)rpc_info->prot;
    key.flow_type = (rpc_info->addr_family == IP_PROTO_IPV6)?FLOW_TYPE_V6:FLOW_TYPE_V4;
    if (!timeout) {
        ret = g_rpc_state->alloc_and_insert_exp_flow(l4_sess->app_session,
                                                  key, &exp_flow, false, 0, true);
    } else {
        ret = g_rpc_state->alloc_and_insert_exp_flow(l4_sess->app_session,
                                                  key, &exp_flow, true, timeout, true);
    }
    SDK_ASSERT((ret == HAL_RET_OK || ret == HAL_RET_ENTRY_EXISTS));
    if (ret == HAL_RET_OK) {
        exp_flow->entry.handler = expected_flow_handler;
        exp_flow->alg = l4_sess->alg;
        exp_flow->idle_timeout = timeout;
        exp_flow->rule_id = (ctx.session())?ctx.session()->sfw_rule_id:0;
        exp_flow->info = g_rpc_state->alg_info_slab()->alloc();
        SDK_ASSERT(exp_flow->info != NULL);
        exp_flow_info = (rpc_info_t *)exp_flow->info;
        exp_flow_info->skip_sfw = true;
        exp_flow_info->vers = rpc_info->vers;
        exp_flow_info->callback = cb;
        incr_num_exp_flows(rpc_info);
    }

    // Need to add the entry with a timer
    HAL_TRACE_DEBUG("Inserting RPC entry with key: {}", key);
}

/*
 * RPC info cleanup handler
 */
void rpcinfo_cleanup_hdlr(l4_alg_status_t *l4_sess) {
    rpc_info_t *rpc_info = NULL;
    if (l4_sess->info != NULL) {
        rpc_info = (rpc_info_t *)l4_sess->info;
        /*
         * Free the packet if it was alloced
         */
        if (rpc_info->pkt_len && rpc_info->pkt)
            HAL_FREE(hal::HAL_MEM_ALLOC_ALG, rpc_info->pkt);

        if (rpc_info->pgmid_sz && rpc_info->pgm_ids)
            HAL_FREE(hal::HAL_MEM_ALLOC_ALG, rpc_info->pgm_ids);

        g_rpc_state->alg_info_slab()->free((rpc_info_t *)l4_sess->info);
    }

    if (l4_sess->sess_hdl != HAL_HANDLE_INVALID &&
        !dllist_empty(&l4_sess->fte_feature_state.session_feature_lentry))
        dllist_del(&l4_sess->fte_feature_state.session_feature_lentry);
}

/*
 * RPC Exec
 */
fte::pipeline_action_t alg_rpc_exec(fte::ctx_t &ctx) {
    fte::feature_session_state_t *alg_state = NULL;
    hal_ret_t                     ret = HAL_RET_OK;
    sfw_info_t                   *sfw_info = sfw::sfw_feature_state(ctx);
    l4_alg_status_t              *l4_sess = NULL;

    if (hal::g_hal_state->is_flow_aware() || ctx.protobuf_request()) {
        return fte::PIPELINE_CONTINUE;
    }

    alg_state = ctx.feature_session_state();
    if (alg_state != NULL)
        l4_sess = (l4_alg_status_t *)alg_status(alg_state);

    if (sfw_info->alg_proto == nwsec::APP_SVC_MSFT_RPC ||
        (l4_sess && l4_sess->alg == nwsec::APP_SVC_MSFT_RPC)) {
        ret = alg_msrpc_exec(ctx, sfw_info, l4_sess);
    } else if (sfw_info->alg_proto == nwsec::APP_SVC_SUN_RPC ||
         (l4_sess && l4_sess->alg == nwsec::APP_SVC_SUN_RPC)) {
        ret = alg_sunrpc_exec(ctx, sfw_info, l4_sess);
    }

    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}


} // namespace alg_tftp
} // namespace plugins
} // namespace hal
