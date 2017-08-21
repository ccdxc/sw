#include "net_plugin.hpp"
#include <interface_api.hpp>
#include <pd_api.hpp>

namespace hal {
namespace net {

static inline intf::LifQType 
derive_flow_lifq_type(const fte::phv_t *phv)
{
    //TODO(goli) derive qtype from packet meta
    return intf::LIF_QUEUE_TYPE_TX;
}

static inline hal_ret_t
update_fwding_info(fte::flow_update_t &flowupd, intf::LifQType qtype, hal::if_t *dif)
{
    hal_ret_t ret;
    uint8_t q_off;
    uint32_t qid;

    flowupd = {};
    flowupd.type = fte::FLOWUPD_FWDING_INFO;

    flowupd.fwding_info.lif = hal::pd::if_get_hw_lif_id(dif);
    if (qtype == intf::LIF_QUEUE_TYPE_NONE) {
        return HAL_RET_OK;
    }

    if (hal::intf_get_if_type(dif) != intf::IF_TYPE_ENIC) {
        return HAL_RET_OK;
    }

    ret = hal::if_get_qid_qoff(dif, qtype, &q_off, &qid);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    flowupd.fwding_info.qid_en = true;
    flowupd.fwding_info.qtype = q_off;
    flowupd.fwding_info.qid = qid;

    return HAL_RET_OK;
}

fte::pipeline_action_t
fwding_exec(fte::ctx_t& ctx)
{
    HAL_TRACE_DEBUG("Invoking fwding feature");

    hal_ret_t ret;
    fte::flow_update_t flowupd;

    intf::LifQType lif_qtype = derive_flow_lifq_type(ctx.phv);

    // Update iflow
    if (ctx.dif) {
        ret = update_fwding_info(flowupd, lif_qtype, ctx.dif);
        if (ret != HAL_RET_OK) {
            return fte::PIPELINE_END; // TODO(goli)abort
        }
    } else {
        // TODO(goli) check hostpinning mode and find uplink based on sep
    }

    ctx_update_iflow(ctx, flowupd);

    //Update rflow
    if (ctx.rflow_valid) {
        if (ctx.sif) {
            update_fwding_info(flowupd, lif_qtype, ctx.sif);
            if (ret != HAL_RET_OK) {
                return fte::PIPELINE_END; // TODO(goli)abort
            }
        } else {
            // TODO(goli) check hostpinning and find uplink based on dep
        }

        ctx_update_rflow(ctx, flowupd);
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
