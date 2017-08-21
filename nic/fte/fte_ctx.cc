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
flowupd_action_(ctx_t& ctx, hal::flow_cfg_t *flow, const flow_update_t& flowupd)
{
    flow->action = flowupd.action.deny ? session::FLOW_ACTION_DROP : session::FLOW_ACTION_ALLOW;

    // Update the ctx if the flow is the current pkt flow
    if (ctx.flow == flow) {
        ctx.drop = flowupd.action.deny;
    }

    return HAL_RET_OK;
}

static inline hal_ret_t
flowupd_conn_track_(ctx_t& ctx, hal::flow_cfg_t *flow, const flow_update_t& flowupd)
{
    flow->state = flowupd.conn_track.state;
    if (ctx.flow == flow) {
        ctx.session_cfg.conn_track_en = flowupd.conn_track.enable;
        ctx.session_cfg.syn_ack_delta = flowupd.conn_track.syn_ack_delta;
    }
    return HAL_RET_OK;
}

static inline hal_ret_t
flowupd_fwding_info_(ctx_t& ctx, hal::flow_cfg_t *flow, const flow_update_t& flowupd)
{
    flow->qid_en = flowupd.fwding_info.qid_en;
    if (flow->qid_en) {
        flow->lif = flowupd.fwding_info.lif;
        flow->qtype = flowupd.fwding_info.qtype;
        flow->qid = flowupd.fwding_info.qid;
    }

    return HAL_RET_OK;
}

static inline hal_ret_t
update_flow_(ctx_t& ctx, hal::flow_cfg_t *flow, const flow_update_t& flowupd)
{
    if (flow->role == hal::FLOW_ROLE_RESPONDER && ctx.rflow_valid == false)
        return HAL_RET_OK;

    switch (flowupd.type) {
    case FLOWUPD_ACTION:
        return flowupd_action_(ctx, flow, flowupd);
    case FLOWUPD_HEADER_MODIFY:
        return HAL_RET_OK;
    case FLOWUPD_HEADER_PUSH:
        return HAL_RET_OK;
    case FLOWUPD_HEADER_POP:
        return HAL_RET_OK;
    case FLOWUPD_CONN_TRACK:
        return flowupd_conn_track_(ctx, flow, flowupd);
    case FLOWUPD_FWDING_INFO:
        return flowupd_fwding_info_(ctx, flow, flowupd);
    }

    return HAL_RET_INVALID_ARG;
}

hal_ret_t
ctx_update_iflow(ctx_t& ctx, const flow_update_t& flowupd)
{
    return update_flow_(ctx, &ctx.iflow, flowupd);
}

hal_ret_t
ctx_update_rflow(ctx_t& ctx, const flow_update_t& flowupd)
{
    return update_flow_(ctx, &ctx.rflow, flowupd);
}


// Update flow of the current packet (i or r)
hal_ret_t
ctx_update_flow(ctx_t& ctx, const flow_update_t& flowupd)
{
    return update_flow_(ctx, ctx.flow, flowupd);
}

// Update reverse flow of the current packet's flow (i or r)
hal_ret_t
ctx_update_reverse_flow(ctx_t& ctx, const flow_update_t& flowupd)
{
    if (ctx.flow == &ctx.iflow) {
        return update_flow_(ctx, &ctx.rflow, flowupd);
    } else {
        return update_flow_(ctx, &ctx.iflow, flowupd);
    }
}



} // namespace fte
