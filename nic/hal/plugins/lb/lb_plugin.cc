//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/fte.hpp"

namespace hal {
namespace lb {

fte::pipeline_action_t lb_exec(fte::ctx_t &ctx);

const std::string FTE_FEATURE_LB("pensando.io/lb:lb");

extern "C" hal_ret_t lb_init(hal_cfg_t *hal_cfg) {
    fte::register_feature(FTE_FEATURE_LB, lb_exec);
    return HAL_RET_OK;
}

extern "C" void lb_exit() {
    fte::unregister_feature(FTE_FEATURE_LB);
}

}
}
