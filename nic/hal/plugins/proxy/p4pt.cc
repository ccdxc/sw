#include "nic/hal/src/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"

namespace hal {
namespace proxy {

fte::pipeline_action_t
p4pt_exec(fte::ctx_t& ctx)
{
    hal_ret_t               ret = HAL_RET_OK;
    flow_key_t              flow_key = ctx.key();

    fte::flow_update_t flowupd = {type: fte::FLOWUPD_MCAST_COPY};
    if(flow_key.proto == IPPROTO_TCP &&
        (flow_key.sport == 3260 || flow_key.dport == 3260)) {
        HAL_TRACE_DEBUG("p4pt flow");
        flowupd.mcast_info.mcast_en = 1;
        flowupd.mcast_info.mcast_ptr = 4; // P4_NW_MCAST_INDEX_P4PT_COPY defined in nic/p4/nw/include/defines.h
        flowupd.mcast_info.proxy_mcast_ptr = 0;
        ret = ctx.update_flow(flowupd);
        if (ret != HAL_RET_OK) {
            ctx.set_feature_status(ret);
            return fte::PIPELINE_END; 
        }
    }
    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
