//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"

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


// ----------------------------------------------------------------------------
// Returns if the ip is my tep ip 
// ----------------------------------------------------------------------------
bool
is_mytep (vrf_t& vrf, const ipvx_addr_t *ipaddr)
{
    return (memcmp(ipaddr, &vrf.mytep_ip.addr, sizeof(ipvx_addr_t)) == 0);
}
}
