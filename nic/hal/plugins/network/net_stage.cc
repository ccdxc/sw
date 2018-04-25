//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "net_plugin.hpp"

namespace hal {
namespace net {

fte::pipeline_action_t
stage_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret;

    ret = ctx.advance_to_next_stage();
    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace net
} // namespace hal
