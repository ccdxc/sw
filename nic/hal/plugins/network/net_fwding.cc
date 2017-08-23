#include "net_plugin.hpp"
#include <interface_api.hpp>
#include <pd_api.hpp>

namespace hal {
namespace net {

static inline hal_ret_t
update_fwding_info(fte::flow_update_t &flowupd, hal::if_t *dif)
{
    flowupd = {};
    flowupd.type = fte::FLOWUPD_FWDING_INFO;

    flowupd.fwding_info.lif = hal::pd::if_get_hw_lif_id(dif);

    if (hal::intf_get_if_type(dif) != intf::IF_TYPE_ENIC) {
        return HAL_RET_OK;
    }

    // TODO Update fwding_info

    return HAL_RET_OK;
}

fte::pipeline_action_t
fwding_exec(fte::ctx_t& ctx)
{
    HAL_TRACE_DEBUG("Invoking fwding feature");

    hal_ret_t ret;
    fte::flow_update_t flowupd;

    // Update iflow
    if (ctx.dif) {
        ret = update_fwding_info(flowupd, ctx.dif);
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
            ret = update_fwding_info(flowupd, ctx.sif);
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
