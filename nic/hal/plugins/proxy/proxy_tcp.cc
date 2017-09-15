#include <proxy.hpp>
#include "proxy_plugin.hpp"

namespace hal {
namespace proxy {

static inline hal_ret_t
update_fwding_info(fte::ctx_t&ctx, proxy_flow_info_t* pfi)
{
    fte::flow_update_t flowupd = {type: fte::FLOWUPD_FWDING_INFO};

    // update fwding info
    flowupd.fwding.lport = pfi->proxy->lport_id;
    flowupd.fwding.qid_en = true;
    flowupd.fwding.qtype = pfi->proxy->qtype;
    if (ctx.role() ==  hal::FLOW_ROLE_INITIATOR) {
        flowupd.fwding.qid = pfi->qid1;
    } else {
        flowupd.fwding.qid = pfi->qid2;
    }

    return ctx.update_flow(flowupd);
}

fte::pipeline_action_t
tcp_exec(fte::ctx_t& ctx)
{
    hal_ret_t               ret = HAL_RET_OK;
    proxy_flow_info_t*      pfi = NULL;
    flow_key_t              flow_key = ctx.key();

    // Ignore direction. Always set it to 0
    flow_key.dir = 0;

    // get the flow info for the tcp proxy service 
    pfi = proxy_get_flow_info(types::PROXY_TYPE_TCP,
                              &flow_key);

    if(!pfi) {
        HAL_TRACE_DEBUG("tcp-proxy: not enabled for flow: {}", ctx.key());
        return fte::PIPELINE_CONTINUE;
    }

    // Update iflow
    ret = update_fwding_info(ctx, pfi);

    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END; 
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
