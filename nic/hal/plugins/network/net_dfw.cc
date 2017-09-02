#include "net_plugin.hpp"

namespace hal {
namespace net {

static inline bool
pol_check_sg_policy(fte::ctx_t &ctx)
{
    // TODO(goli) check Security policy
    return true;
}

static inline bool
conn_tracking_configured(fte::ctx_t &ctx)
{
    // TODO(goli) check Security profile
    return true;
}

fte::pipeline_action_t
dfw_exec(fte::ctx_t& ctx)
{
    HAL_TRACE_DEBUG("Invoking firewall feature");

    fte::flow_update_t flowupd = {type: fte::FLOWUPD_ACTION};

    if (!pol_check_sg_policy(ctx)) {
        flowupd.action.deny = true;
    } else {
        flowupd.action.deny = false;
    }
    ctx.update_iflow(flowupd);
    ctx.update_rflow(flowupd);

    if (ctx.key().proto == types::IPPROTO_TCP &&
        conn_tracking_configured(ctx)) {

        // iflow
        flowupd = {type: fte::FLOWUPD_CONN_TRACK};
        flowupd.conn_track.enable = true;
        flowupd.conn_track.state = session::FLOW_TCP_STATE_TCP_SYN_RCVD;
        flowupd.conn_track.syn_ack_delta = 0;
        ctx.update_iflow(flowupd);

        //rflow
        flowupd.conn_track.state = session::FLOW_TCP_STATE_INIT;
        ctx.update_rflow(flowupd);
    }

    return fte::PIPELINE_CONTINUE;
}

}
}
