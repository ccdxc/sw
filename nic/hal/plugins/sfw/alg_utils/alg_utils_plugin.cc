//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/fte.hpp"

namespace hal {
namespace plugins {
namespace alg_utils {

extern "C" hal_ret_t alg_utils_init(hal_cfg_t *hal_cfg) {
    return HAL_RET_OK;
}

extern "C" void alg_utils_exit() {
}

}
}
}
