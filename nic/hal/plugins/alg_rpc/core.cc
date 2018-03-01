/*
 * core.cc
 */

#include "core.hpp"
#include "nic/hal/plugins/sfw/core.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"

namespace hal {
namespace plugins {
namespace alg_rpc {

using namespace hal::plugins::sfw;
using namespace hal::plugins::alg_utils;

void incr_parse_error(l4_alg_status_t *sess) {
    HAL_ATOMIC_INC_UINT32(&((rpc_info_t *)sess->info)->parse_errors, 1);
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

    alg_state = ctx.feature_session_state();
    if (alg_state != NULL) {
        l4_sess = (l4_alg_status_t *)alg_status(alg_state);
        app_sess = l4_sess->app_session;
        if (l4_sess->isCtrl == TRUE) {
            if (ctx.force_delete() || (dllist_empty(&app_sess->exp_flow_lhead)\
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
            } else if ((ctx.session()->iflow->state == session::FLOW_TCP_STATE_FIN_RCVD) ||
                       (ctx.session()->rflow &&
                        (ctx.session()->rflow->state == session::FLOW_TCP_STATE_FIN_RCVD))) {
                /*
                 * We received FIN/RST on the control session
                 * We let the HAL cleanup happen while we keep the
                 * app_session state if there are data sessions
                 */
                l4_sess->session = NULL;
                return fte::PIPELINE_CONTINUE;
            } else {
               /*
                * Dont cleanup if control session is timed out
                * we need to keep it around until the data session
                * goes away
                */
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
                      l4_alg_status_t, l4_sess_lentry))->session == NULL) {
            /*
             * If this was the last session hanging and there is no
             * HAL session for control session. This is the right time
             * to clean it
             */
            g_rpc_state->cleanup_app_session(l4_sess->app_session);
        }
    }
   
    return fte::PIPELINE_CONTINUE;
}

uint8_t *alloc_rpc_pkt(void) {
    return ((uint8_t *)HAL_CALLOC(hal::HAL_MEM_ALLOC_ALG, MAX_ALG_RPC_PKT_SZ));
}

/*
 * Expected flow callback. FTE issues this callback with the expected flow data
 */
hal_ret_t expected_flow_handler(fte::ctx_t &ctx, expected_flow_t *wentry) {
    l4_alg_status_t      *entry = NULL;
    rpc_info_t           *rpc_info = NULL;
    sfw_info_t           *sfw_info = (sfw_info_t*)\
                            ctx.feature_state(FTE_FEATURE_SFW);

    entry = (l4_alg_status_t *)wentry;
    rpc_info = (rpc_info_t *)entry->info;
    if (entry->isCtrl != TRUE) {
        sfw_info->skip_sfw = rpc_info->skip_sfw;
        HAL_TRACE_DEBUG("Expected flow handler - skip sfw {}", sfw_info->skip_sfw);
    }
    ctx.set_feature_name(FTE_FEATURE_ALG_RPC.c_str());
    ctx.register_feature_session_state(&entry->fte_feature_state);

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
    key.dir   = 0; 
    key.sport = 0;
    key.dip = rpc_info->ip;
    key.dport = rpc_info->dport;
    key.proto = (types::IPProtocol)rpc_info->prot;
    key.flow_type = (rpc_info->addr_family == IP_PROTO_IPV6)?FLOW_TYPE_V6:FLOW_TYPE_V4;
    ret = g_rpc_state->alloc_and_insert_exp_flow(l4_sess->app_session,
                                                  key, &exp_flow, true, timeout);
    HAL_ASSERT(ret == HAL_RET_OK);
    exp_flow->entry.handler = expected_flow_handler;
    exp_flow->alg = l4_sess->alg;
    exp_flow->info = g_rpc_state->alg_info_slab()->alloc();
    HAL_ASSERT(exp_flow->info != NULL);
    exp_flow_info = (rpc_info_t *)exp_flow->info;
    exp_flow_info->skip_sfw = TRUE;
    if (exp_flow->alg == nwsec::APP_SVC_MSFT_RPC) {
        memcpy(&exp_flow_info->uuid, &rpc_info->uuid, sizeof(rpc_info->uuid));
    } else {
        exp_flow_info->prog_num = rpc_info->prog_num;
    }
    exp_flow_info->vers = rpc_info->vers;
    exp_flow_info->callback = cb;
    
    // Need to add the entry with a timer
    // Todo(Pavithra) add timer to every RPC ALG entry
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
        g_rpc_state->alg_info_slab()->free((rpc_info_t *)l4_sess->info);
    }
}

/*
 * RPC Exec
 */
fte::pipeline_action_t alg_rpc_exec(fte::ctx_t &ctx) {
    fte::feature_session_state_t *alg_state = NULL;
    hal_ret_t                     ret = HAL_RET_OK;
    sfw_info_t                   *sfw_info = (sfw_info_t*)\
                                    ctx.feature_state(FTE_FEATURE_SFW);
    l4_alg_status_t              *l4_sess = NULL;

    if (ctx.protobuf_request()) {
        return fte::PIPELINE_CONTINUE;
    }

    alg_state = ctx.feature_session_state();
    if (alg_state != NULL) 
        l4_sess = (l4_alg_status_t *)alg_status(alg_state);

    HAL_TRACE_DEBUG("Firewall Info ALG Proto: {} l4_sess {:p}", 
                    sfw_info->alg_proto, (void *)l4_sess);
    if (sfw_info->alg_proto == nwsec::APP_SVC_MSFT_RPC ||
        (l4_sess && l4_sess->alg == nwsec::APP_SVC_MSFT_RPC)) {
        ret = alg_msrpc_exec(ctx, sfw_info, l4_sess); 
    } else if (sfw_info->alg_proto == nwsec::APP_SVC_SUN_RPC ||
         (l4_sess && l4_sess->alg == nwsec::APP_SVC_SUN_RPC)) {
        ret = alg_sunrpc_exec(ctx, sfw_info, l4_sess);
    }

    if (ret != HAL_RET_OK)
        return fte::PIPELINE_END;

    return fte::PIPELINE_CONTINUE;
}


} // namespace alg_tftp
} // namespace plugins
} // namespace hal
