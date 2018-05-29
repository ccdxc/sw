//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/fte.hpp"

namespace hal {
namespace dol {

fte::pipeline_action_t dol_test_exec(fte::ctx_t &ctx);

const std::string FTE_FEATURE_DOL_TEST("pensando.io/dol:test");

extern "C" hal_ret_t dol_init(hal_cfg_t *hal_cfg) {
    fte::register_feature(FTE_FEATURE_DOL_TEST, dol_test_exec);
    return HAL_RET_OK;
}

extern "C" void dol_exit() {
    fte::unregister_feature(FTE_FEATURE_DOL_TEST);
}

}
}
