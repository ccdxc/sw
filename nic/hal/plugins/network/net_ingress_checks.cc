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
perform_ingress_checks(fte::ctx_t&ctx)
{
    bool broadcast_pkt = is_broadcast(ctx);
    bool mcast_dmac = is_multicast_dmac(ctx);
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_ACTION};

    if (broadcast_pkt) {
        HAL_TRACE_ERR("Ignore session create");
        ctx.set_ignore_session_create(true);
        return HAL_RET_OK;
    }

    // Drop Multicast packets 
    // for smart switch mode
    if (mcast_dmac) {
        flowupd.action = session::FLOW_ACTION_DROP;
        ctx.set_ignore_session_create(true);
        HAL_TRACE_ERR("Dropping packet for multicast dmac");
        ctx.update_flow(flowupd);
    }

    return HAL_RET_OK;
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
        ret = perform_ingress_checks(ctx);
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

