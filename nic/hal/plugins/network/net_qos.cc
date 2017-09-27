#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/hal/src/session.hpp"

namespace hal {
namespace net {

static inline hal_ret_t
update_flow_from_qos_spec(fte::ctx_t& ctx, const session::FlowInfo& flow_info)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_HEADER_REWRITE};

    //No rewrite on ingress as per qos_extract_action_from_spec() in src/qos.cc
    if (flow_info.has_eg_qos_actions()) {
        //QoS action
        if (flow_info.eg_qos_actions().marking_spec().pcp_rewrite_en()) {
            HEADER_SET_FLD(flowupd.header_rewrite, ether, dot1p,
                flow_info.eg_qos_actions().marking_spec().pcp());
        }
        //HAL_TRACE_DEBUG("Set pcp() : {}",  flow_info.eg_qos_actions().marking_spec().pcp());
        if (flow_info.eg_qos_actions().marking_spec().dscp_rewrite_en()) {
            HEADER_SET_FLD(flowupd.header_rewrite, ipv4, dscp,
                flow_info.eg_qos_actions().marking_spec().dscp());
        }
        //HAL_TRACE_DEBUG("Set dscp(): {}", flow_info.eg_qos_actions().marking_spec().dscp());
    }
    return ctx.update_flow(flowupd);
}

fte::pipeline_action_t
qos_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret;

    if (ctx.protobuf_request()) {
        if (ctx.role() == hal::FLOW_ROLE_INITIATOR) {
            //Lookup for qos action Based on the key
            HAL_TRACE_DEBUG("Initiator flow");
            ret = update_flow_from_qos_spec(ctx,
                    ctx.sess_spec()->initiator_flow().flow_data().flow_info());
        } else {
            ret = update_flow_from_qos_spec(ctx,
                    ctx.sess_spec()->responder_flow().flow_data().flow_info());
        }
        if (ret != HAL_RET_OK) {
            ctx.set_feature_status(ret);
            return fte::PIPELINE_END;
        }
    }

    return fte::PIPELINE_CONTINUE;

}

}
}
