#include "fte_ctx.hpp"
#include "fte_flow.hpp"

namespace fte {

hal_ret_t
ctx_init(ctx_t &ctx, phv_t *phv, uint8_t *pkt, size_t pkt_len)
{
    hal_ret_t ret;

    ctx = {};

    ctx.phv = phv;
    ctx.pkt = pkt;
    ctx.pkt_len = pkt_len;
    ctx.arm_lifq = {phv->lif, phv->qtype, phv->qid};

    ret = flow_init_flows(ctx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("fte - failed to init flows, err={}", ret);
        return ret;
    }

    return HAL_RET_OK;
}

static inline hal_ret_t
_update_flow(ctx_t&, hal::flow_cfg_t *flow, const flow_action_t&)
{
    // TODO(goli)
    return HAL_RET_OK;
}

hal_ret_t
ctx_update_iflow(ctx_t& ctx, const flow_action_t& action)
{
    return _update_flow(ctx, &ctx.iflow, action);
}

hal_ret_t
ctx_update_rflow(ctx_t& ctx, const flow_action_t& action)
{
    return _update_flow(ctx, &ctx.rflow, action);
}


// Update flow of the current packet (i or r)
hal_ret_t
ctx_update_flow(ctx_t& ctx, const flow_action_t& action)
{
    return _update_flow(ctx, ctx.flow, action);
}

// Update reverse flow of the current packet's flow (i or r)
hal_ret_t
ctx_update_reverse_flow(ctx_t& ctx, const flow_action_t& action)
{
    if (ctx.flow == &ctx.iflow) {
        return _update_flow(ctx, &ctx.rflow, action);
    } else {
        return _update_flow(ctx, &ctx.iflow, action);
    }
}



} // namespace fte
