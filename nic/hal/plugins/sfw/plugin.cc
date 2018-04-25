//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/fte.hpp"
#include "core.hpp"

namespace hal {
namespace plugins {
namespace sfw {

fte::pipeline_action_t sfw_exec(fte::ctx_t &ctx);
fte::pipeline_action_t conntrack_exec(fte::ctx_t &ctx);

extern "C" hal_ret_t sfw_init() {
    fte::feature_info_t info = {
        state_size: sizeof(sfw_info_t),
    };
    fte::register_feature(FTE_FEATURE_SFW, sfw_exec, info);
    fte::register_feature(FTE_FEATURE_CONNTRACK, conntrack_exec);
    return HAL_RET_OK;
}

extern "C" void sfw_exit() {
    fte::unregister_feature(FTE_FEATURE_SFW);
}

}  // namespace sfw
}  // namespace plugins
}  // namespace hal
