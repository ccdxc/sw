//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {
namespace plugins {
namespace network {


static inline hal_ret_t
update_flow_for_rdma (fte::ctx_t& ctx)
{
    //hal_ret_t ret = HAL_RET_OK;
    flow_key_t flow_key = ctx.get_key();
    bool rdma_enabled = FALSE;

    HAL_TRACE_DEBUG("net-rdma: flow {}", flow_key);

    if ((flow_key.flow_type != hal::FLOW_TYPE_V4) &&
        (flow_key.flow_type != hal::FLOW_TYPE_V6)) {
        HAL_TRACE_DEBUG("net-rdma: flow type {} is not v4/v6. ignoring", flow_key.flow_type);
        return HAL_RET_OK;
    }
        
    if (flow_key.proto != types::IPPROTO_UDP) {
        HAL_TRACE_DEBUG("net-rdma: flow proto is not UDP. ignoring");
        return HAL_RET_OK;
    }

    if ((flow_key.sport != UDP_PORT_ROCE_V2) && 
        (flow_key.dport != UDP_PORT_ROCE_V2)) {
        HAL_TRACE_DEBUG("net-rdma: dport {} is not that of ROCEV2. ignoring", flow_key.sport);
        return HAL_RET_OK;
    }

#if 0 
    hal::if_t *dif = ctx.dif();
    hal::if_t *sif = ctx.sif();

      //Disabling this check becuase today, P4 code doesn't know
      //the dest_lif before masking the sport and hence doing just
      // based on the RDMA DPORT
    if (dif && (dif->if_type == intf::IF_TYPE_ENIC)) {
        hal::lif_t *dlif = if_get_lif(dif);
        rdma_enabled = lif_get_enable_rdma(dlif);
    } else if (sif && (sif->if_type == intf::IF_TYPE_ENIC)) {
        hal::lif_t *slif = if_get_lif(sif);
        rdma_enabled = lif_get_enable_rdma(slif);
    }
#endif

    if (rdma_enabled == FALSE) {
        HAL_TRACE_DEBUG("net-rdma: RDMA is not enabled on ENIC. ignoring");
        return HAL_RET_OK;
    }

    fte::flow_update_t flowupd = {type: fte::FLOWUPD_LKP_KEY};

    flowupd.key = flow_key;
    flowupd.key.dport = UDP_PORT_ROCE_V2;
    flowupd.key.sport = 0;

    return ctx.update_flow(flowupd);
}

fte::pipeline_action_t
rdma_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;

    ret = update_flow_for_rdma(ctx);
    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace network
} // namespace plugins
} // namespace hal
