#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/hal/src/session.hpp"

namespace hal {
namespace net {

static inline hal_ret_t
update_flow_from_qos_spec(fte::ctx_t& ctx)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_HEADER_REWRITE};
    qos_actions_t src_if_tx_qos_actions;

    src_if_tx_qos_actions = ctx.sif()->tx_qos_actions;

    //QoS action
    if (src_if_tx_qos_actions.pcp_write_en) {
        HEADER_SET_FLD(flowupd.header_rewrite, ether, dot1p,
                       src_if_tx_qos_actions.pcp);
    }
    if (src_if_tx_qos_actions.dscp_write_en) {
        if (ctx.key().flow_type == hal::FLOW_TYPE_V4) {
            HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dscp,
                           src_if_tx_qos_actions.dscp);
        } else {
            HEADER_SET_FLD(flowupd.header_rewrite, ipv6, dscp,
                           src_if_tx_qos_actions.dscp);
        }
    }
    return ctx.update_flow(flowupd);
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
