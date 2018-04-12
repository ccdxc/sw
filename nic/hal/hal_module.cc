// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/hal_module.hpp"
#include "nic/hal/src/nw/nw.hpp"
#include "nic/hal/src/mcast/multicast.hpp"
#include "nic/hal/src/aclqos/acl.hpp"
#include "nic/hal/src/firewall/nwsec.hpp"
#include "nic/hal/src/l4lb/l4lb.hpp"
#include "nic/hal/src/telemetry/telemetry.hpp"
#include "nic/hal/src/stats/stats.hpp"

namespace hal {

hal_module_meta    *g_module_meta[HAL_MODULE_ID_MAX];

static void
hal_module_meta_init (void)
{
    g_module_meta[HAL_MODULE_ID_NW] =
        new hal_module_meta(hal_nw_init_cb,
                            hal_nw_cleanup_cb);
    g_module_meta[HAL_MODULE_ID_MULTICAST] =
        new hal_module_meta(hal_mcast_init_cb,
                            hal_mcast_cleanup_cb);
    g_module_meta[HAL_MODULE_ID_ACLQOS] =
        new hal_module_meta(hal_aclqos_init_cb,
                            hal_aclqos_cleanup_cb);
    g_module_meta[HAL_MODULE_ID_FIREWALL] =
        new hal_module_meta(hal_fw_init_cb,
                            hal_fw_cleanup_cb);
    g_module_meta[HAL_MODULE_ID_DOS] =
        new hal_module_meta(hal_dos_init_cb,
                            hal_dos_cleanup_cb);
    g_module_meta[HAL_MODULE_ID_L4LB] =
        new hal_module_meta(hal_l4lb_init_cb,
                            hal_l4lb_cleanup_cb);
    g_module_meta[HAL_MODULE_ID_TELEMETRY] =
        new hal_module_meta(hal_telemetry_init_cb,
                            hal_telemetry_cleanup_cb);
    g_module_meta[HAL_MODULE_ID_STATS] =
        new hal_module_meta(hal_stats_init_cb,
                            hal_stats_cleanup_cb);
}

//------------------------------------------------------------------------------
// do per module initialization
// NOTE: if there is some inter-module dependency, init them in proper order but
// ideally we should avoid such dependencies at this stage
//------------------------------------------------------------------------------
hal_ret_t
hal_module_init (void)
{
    hal_ret_t           ret;
    uint32_t            module_id;
    module_init_cb_t    init_cb;

    // build module meta
    hal_module_meta_init();

    // do module initialization
    for (module_id = (uint32_t)HAL_MODULE_ID_MIN;
         module_id < (uint32_t)HAL_MODULE_ID_MAX; module_id++) {
        if (g_module_meta[module_id] &&
            (init_cb = g_module_meta[module_id]->init_cb())) {
            HAL_TRACE_DEBUG("Initializing module {}", module_id);
            ret = init_cb();
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to initialize module {}, err {}",
                              module_id, ret);
                return ret;
            }
        }
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// do per module initialization
// NOTE: if there is some inter-module dependency, init them in proper order but
// ideally we should avoid such dependencies at this stage
//------------------------------------------------------------------------------
hal_ret_t
hal_module_cleanup (void)
{
    hal_ret_t              ret;
    uint32_t               module_id;
    module_cleanup_cb_t    cleanup_cb;

    // do module initialization
    for (module_id = (uint32_t)HAL_MODULE_ID_MIN;
         module_id < (uint32_t)HAL_MODULE_ID_MAX; module_id++) {
        if ((cleanup_cb = g_module_meta[module_id]->cleanup_cb())) {
            HAL_TRACE_DEBUG("Cleaning up module {}", module_id);
            ret = cleanup_cb();
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("Failed to cleanup module {}, err {}",
                              module_id, ret);
                return ret;
            }
        }
    }
    return HAL_RET_OK;
}

}    // namespace hal
