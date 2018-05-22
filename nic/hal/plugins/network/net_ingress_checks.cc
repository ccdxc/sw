//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <net/ethernet.h>
#include "nic/p4/include/common_defines.h"
#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {
namespace plugins {
namespace network {


static bool
is_broadcast(fte::ctx_t &ctx) {
    const fte::cpu_rxhdr_t* cpu_hdr = ctx.cpu_rxhdr();
    ether_header_t *eth_hdr;
    if (!ctx.pkt()) {
        return false;
    }

    eth_hdr = (ether_header_t*)(ctx.pkt() + cpu_hdr->l2_offset);
    for (int i = 0; i < ETHER_ADDR_LEN; i++) {
        if (eth_hdr->dmac[i] != 0xff) {
            return false;
        }
    }
    return true;
}

static inline hal_ret_t
update_src_if(fte::ctx_t&ctx)
{
    bool src_local = (ctx.sep() && ctx.sep()->ep_flags & EP_FLAGS_LOCAL);
    bool dst_local = (ctx.dep() && ctx.dep()->ep_flags & EP_FLAGS_LOCAL);
    bool broadcast_pkt = is_broadcast(ctx);
    if_t *sif;

    if (broadcast_pkt) {
        return HAL_RET_OK;
    }
    // drop remote to remote sessions
    if (!src_local &&  !dst_local) {
        fte::flow_update_t flowupd = {type: fte::FLOWUPD_ACTION};
        flowupd.action = session::FLOW_ACTION_DROP;
        HAL_TRACE_DEBUG("Dropping packet for remote to remote");
        return ctx.update_flow(flowupd);
    }

    // no check needed if src is local or no pinned intf for dep
    if (src_local || ctx.dep()->pinned_if_handle == HAL_HANDLE_INVALID) {
        return HAL_RET_OK;
    }

    sif = hal::find_if_by_handle(ctx.dep()->pinned_if_handle);
    HAL_ASSERT_RETURN(sif, HAL_RET_IF_NOT_FOUND);

    // Drop the packet if the pkt src_if is not the expected if
    pd::pd_if_get_hw_lif_id_args_t args;
    args.pi_if = sif;
    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_IF_GET_HW_LIF_ID, (void *)&args);

    if (ctx.role() == hal::FLOW_ROLE_INITIATOR && ctx.cpu_rxhdr() &&
        ctx.cpu_rxhdr()->src_lif != args.hw_lif_id) {
        // ctx.cpu_rxhdr()->src_lif != hal::pd::if_get_hw_lif_id(sif)) {
        HAL_TRACE_DEBUG("Dropping for srclif mismatch.. {} {}", args.hw_lif_id, ctx.cpu_rxhdr()->src_lif);
        ctx.set_drop();
    }

    // set the expected sif for both iflow & rflow
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_INGRESS_INFO};
    flowupd.ingress_info.expected_sif = sif;
    return ctx.update_flow(flowupd);
}

fte::pipeline_action_t
ingress_checks_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;

    if (hal::is_forwarding_mode_host_pinned()) {
        ret = update_src_if(ctx);
    }

    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace network
} // namespace plugins
} // namespace hal

