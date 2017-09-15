#include "net_plugin.hpp"
#include <interface_api.hpp>
#include <pd_api.hpp>
#include <net/ethernet.h>

namespace hal {
namespace net {

static inline hal_ret_t
update_tunnel_info(fte::ctx_t&ctx, hal::if_t *dif, hal::l2seg_t *dl2seg)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_HEADER_PUSH};

    if (dif->if_type != intf::IF_TYPE_TUNNEL) {
        return HAL_RET_OK;
    }

    // TODO(goli) set appropriate header fields
    HEADER_SET_FLD(flowupd.header_push, ether, dmac, ether_addr{});
    HEADER_SET_FLD(flowupd.header_push, ether, smac, ether_addr{});
    HEADER_SET_FLD(flowupd.header_push, ipv4, sip, ipv4_addr_t{});
    HEADER_SET_FLD(flowupd.header_push, ipv4, dip, ipv4_addr_t{});
    
    switch (dl2seg->fabric_encap.type) {
    case types::encapType::ENCAP_TYPE_VXLAN:
        HEADER_SET_FLD(flowupd.header_push, vxlan, tenant_id, dl2seg->fabric_encap.val);
        break;
    default:
        return HAL_RET_INVALID_ARG;
    }

    return ctx.update_flow(flowupd);
}

fte::pipeline_action_t
tunnel_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret;

    if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
        ret = update_tunnel_info(ctx, ctx.dif(), ctx.dl2seg());
    } else {
        ret = update_tunnel_info(ctx, ctx.sif(), ctx.sl2seg());
    }

    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END; 
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace net
} // namesapce hal
