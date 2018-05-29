//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"

namespace hal {
namespace plugins {
namespace alg_sip {

extern "C" hal_ret_t alg_sip_init(hal_cfg_t *hal_cfg) {
    fte::register_feature(FTE_FEATURE_ALG_SIP, alg_sip_exec);
    return HAL_RET_OK;
}

extern "C" void alg_sip_exit() {
    fte::unregister_feature(FTE_FEATURE_ALG_SIP);
}

}  // namespace alg_sip
}  // namespace plugins
}  // namespace hal
