//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <net/ethernet.h>
#include "nic/p4/common/defines.h"
#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/vrf_api.hpp"


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

    eth_hdr = GET_L2_HEADER(ctx.pkt(), cpu_hdr);
    for (int i = 0; i < ETHER_ADDR_LEN; i++) {
        if (eth_hdr->dmac[i] != 0xff) {
            return false;
        }
    }
    return true;
}

static bool
is_multicast_dmac(fte::ctx_t &ctx) {
    const fte::cpu_rxhdr_t* cpu_hdr = ctx.cpu_rxhdr();
    ether_header_t *eth_hdr;
    if (!ctx.pkt()) {
        return false;
    }

    eth_hdr = (ether_header_t*)(ctx.pkt() + cpu_hdr->l2_offset);
    if (eth_hdr->dmac[0]&0x01) {
        return true;
    }

    return false;
}

static inline hal_ret_t
update_src_if(fte::ctx_t&ctx)
{
    bool src_local = 0;
    bool dst_local = 0; 
    bool broadcast_pkt = is_broadcast(ctx);
    bool mcast_dmac = is_multicast_dmac(ctx);
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_ACTION};

    src_local = ((ctx.sep() && ctx.sep()->ep_flags & EP_FLAGS_LOCAL) || (hal::is_mytep(*(ctx.svrf()), (&ctx.key().sip)))); 
    dst_local =  (ctx.dep() && ctx.dep()->ep_flags & EP_FLAGS_LOCAL);
    if_t *sif;

    if (broadcast_pkt || 
        ((hal::g_hal_state->forwarding_mode() == HAL_FORWARDING_MODE_SMART_HOST_PINNED)
          && mcast_dmac)) {
        return HAL_RET_OK;
    }

    // Ignore ingress-checks for IPFIX pkts
    if ((ctx.session() && ctx.session()->is_ipfix_flow) ||
            (ctx.cpu_rxhdr() && (ctx.cpu_rxhdr()->src_lif == HAL_LIF_CPU) &&
            (ctx.cpu_rxhdr()->src_app_id == P4PLUS_APPTYPE_TELEMETRY))) {
        HAL_TRACE_VERBOSE("IPFIX pkt, do not enforce ingress-checks");
        return HAL_RET_OK;
    }

    // Drop Multicast packets 
    // for smart switch mode
    if (mcast_dmac) {
        flowupd.action = session::FLOW_ACTION_DROP;
        HAL_TRACE_ERR("Dropping packet for multicast dmac");
        return ctx.update_flow(flowupd);
    } 

    // drop remote to remote sessions
    if (!src_local &&  !dst_local) {
        flowupd.action = session::FLOW_ACTION_DROP;
        HAL_TRACE_ERR("Dropping packet for remote to remote");
        return ctx.update_flow(flowupd);
    }

    if (ctx.cpu_rxhdr() && (ctx.cpu_rxhdr()->src_lif == HAL_LIF_CPU) &&
        (ctx.cpu_rxhdr()->src_app_id == P4PLUS_APPTYPE_IPSEC)) {
        HAL_TRACE_VERBOSE("Pkt from IPSec app, do not enforce ingress-checks");
        return HAL_RET_OK;
    }

    if (dst_local && ctx.dif() == NULL) {
        // If we allow flow create here we would be sending the packets destined
        // to local workload to pinned uplink and we dont want to do that.
        HAL_TRACE_ERR("Dest is local but no pinning yet. Dropping the packet");
        flowupd.action = session::FLOW_ACTION_DROP;
        return ctx.update_flow(flowupd);
    }

    // no check needed if src is local or no pinned intf for dep
    // or if we actually got the packet from local EP and we are running
    // the pipeline with flow swapped. This could happen if we are running
    // the pipeline on existing sessions but received the packet fromo reverse flow
    // if (src_local || ctx.dep()->pinned_if_handle == HAL_HANDLE_INVALID) {
    if (src_local || ep_get_pinned_uplink(ctx.dep()) == NULL ||
        (ctx.role() == hal::FLOW_ROLE_INITIATOR && 
         ctx.is_flow_swapped() && dst_local)) {
        HAL_TRACE_VERBOSE("Source is local");
        return HAL_RET_OK;
    }

    sif = ep_get_pinned_uplink(ctx.dep());
    SDK_ASSERT_RETURN(sif, HAL_RET_IF_NOT_FOUND);

    // Drop the packet if the pkt src_if is not the expected if
    pd::pd_if_get_hw_lif_id_args_t args;
    pd::pd_func_args_t pd_func_args = {0};
    args.pi_if = sif;
    pd_func_args.pd_if_get_hw_lif_id = &args;
    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_IF_GET_HW_LIF_ID, &pd_func_args);

    if (ctx.role() == hal::FLOW_ROLE_INITIATOR && ctx.cpu_rxhdr() &&
                    ctx.cpu_rxhdr()->src_lif != args.hw_lif_id) {
        HAL_TRACE_ERR("Dropping for srclif mismatch.. {} {}", args.hw_lif_id, ctx.cpu_rxhdr()->src_lif);
        ctx.set_drop();
        return HAL_RET_FTE_SRC_LIF_MISMATCH;
    }

    // set the expected sif for both iflow & rflow
    flowupd = {type: fte::FLOWUPD_INGRESS_INFO};
    flowupd.ingress_info.expected_sif = sif;
    return ctx.update_flow(flowupd);
}

fte::pipeline_action_t
ingress_checks_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;

    /*
     * In the IPsec/L3 tunnel cases, the SRC-LIF of the post-decapsulated packets will be CPU-LIF,
     * which we dont want to do the SRC-LIF checks for.
     * TODO: This is a temp fix and Ramesh will be looking into the proper fix to override the
     * expected SRC-LIF from the corresponding tunnel plugins.
     */
    if (!ctx.l3_tunnel_flow()) {
        if (hal::is_forwarding_mode_host_pinned()) {
            ret = update_src_if(ctx);
        }
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

