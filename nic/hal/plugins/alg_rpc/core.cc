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

void insert_rpc_expflow(fte::ctx_t& ctx, l4_alg_status_t *l4_sess, rpc_cb_t cb) {
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
                                                       key, &exp_flow);
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
