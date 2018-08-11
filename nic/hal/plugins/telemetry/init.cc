//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"
#include "sdk/slab.hpp"
#include "nic/include/hal_mem.hpp"

namespace hal {
namespace plugins {
namespace telemetry {

static hal_ret_t
telemetry_mem_slab_init (void)
{
    return HAL_RET_OK;
}

extern "C" hal_ret_t
telemetry_init (hal_cfg_t *hal_cfg)
{
    hal_ret_t ret;

    fte::feature_info_t info = {
        state_size: sizeof(telemetry_info_t),
    };
    fte::register_feature(FTE_FEATURE_TELEMETRY, telemetry_exec, info);

    HAL_TRACE_DEBUG("Registering feature: {}", FTE_FEATURE_TELEMETRY);

    if ((ret = telemetry_mem_slab_init()) != HAL_RET_OK) {
        return ret;
    }

    return ret;
}

extern "C" void
telemetry_exit()
{
    fte::unregister_feature(FTE_FEATURE_TELEMETRY);
}

}  // namespace telemetry
}  // namespace plugins
}  // namespace hal
