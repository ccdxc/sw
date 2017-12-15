#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/hal/src/session.hpp"

namespace hal {
namespace net {

static inline hal_ret_t
update_flow_from_qos_spec(fte::ctx_t& ctx)
{
    // TODO: For uplink and the proxy flows, set the qos-class-id
    return HAL_RET_OK;
}

fte::pipeline_action_t
qos_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("Initiator flow");
    if (ctx.sif() && ctx.sif()->if_type == intf::IF_TYPE_ENIC) {
        HAL_TRACE_DEBUG("Enic interface:Update Qos");
        ret = update_flow_from_qos_spec(ctx);
    }
    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

}
}
