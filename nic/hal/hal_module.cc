// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/hal_module.hpp"
#include "nic/hal/src/nw/nw.hpp"
#include "nic/hal/src/mcast/multicast.hpp"
#include "nic/hal/src/aclqos/acl.hpp"
#include "nic/hal/src/firewall/nwsec.hpp"
#include "nic/hal/src/l4lb/l4lb.hpp"
#include "nic/hal/src/telemetry/telemetry.hpp"

namespace hal {

hal_module_meta    *g_module_meta[HAL_MODULE_ID_MAX];

void
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
}

}    // namespace hal
