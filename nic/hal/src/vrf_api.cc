// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/src/vrf.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// Returns infra vrf
// ----------------------------------------------------------------------------
vrf_t *
vrf_get_infra_vrf()
{
    hal_handle_t    infra_vrf_handle = g_hal_state->infra_vrf_handle();
    vrf_t           *vrf = NULL;

    if (infra_vrf_handle != HAL_HANDLE_INVALID) {
        vrf = vrf_lookup_by_handle(infra_vrf_handle);
        if (vrf == NULL) {
            HAL_TRACE_ERR("Unable to find infra vrf");
            goto end;
        }
        return vrf;
    }

end:
    return NULL;
}

}
