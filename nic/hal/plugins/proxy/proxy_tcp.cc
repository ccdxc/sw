#include <proxy.hpp>
#include "proxy_plugin.hpp"

namespace hal {
namespace proxy {

static inline hal_ret_t
update_fwding_info(fte::ctx_t&ctx, hal::flow_role_t role, proxy_flow_info_t* pfi)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};

    // update fwding info
    // FIXME: Update LPort oncce getLport for TCP LIF API is available
    //flowupd.fwding.lport = hal::pd::if_get_lport_id(dif);
    flowupd.fwding.qid_en = true;
    flowupd.fwding.qtype = pfi->proxy->qtype;
    flowupd.fwding.qid = pfi->qid1;

    return ctx.update_flow(role, flowupd);
}

fte::pipeline_action_t
tcp_exec(fte::ctx_t& ctx)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxy_flow_info_t*      pfi = NULL;

    // get the flow info for the tcp proxy service 
    pfi = proxy_get_flow_info(types::PROXY_TYPE_TCP,
                              &ctx.key());

    if(!pfi) {
        HAL_TRACE_DEBUG("tcp-proxy: not enabled for flow: {}", ctx.key());
        return fte::PIPELINE_CONTINUE;
    }

    // Update iflow
    // FIXME:Program i-Flow bs R-flow based on hFlow vs n-Flow.
    // There is no way to identify that now, so programming only iFlow. 
    // Change it once FTE adds  a way to identify the role. 
    ret = update_fwding_info(ctx, hal::FLOW_ROLE_INITIATOR, pfi);
    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END; 
    }

    /*
    //Update rflow
    if (ctx.valid_rflow()) {
        ret = update_fwding_info(ctx, hal::FLOW_ROLE_RESPONDER);
        if (ret != HAL_RET_OK) {
            ctx.set_feature_status(ret);
            return fte::PIPELINE_END; 
        }
    }
    */
    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
