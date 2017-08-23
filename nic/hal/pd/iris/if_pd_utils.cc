#include "if_pd_utils.hpp"
#include "lif_pd.hpp"
#include "uplinkif_pd.hpp"
#include "uplinkpc_pd.hpp"
#include "nwsec_pd.hpp"
#include <interface_api.hpp>

using namespace hal;

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// Given a PI If, get its hw lif id
// ----------------------------------------------------------------------------
uint32_t
if_get_hw_lif_id(if_t *pi_if) 
{
    lif_t           *pi_lif = NULL;
    pd_lif_t        *pd_lif = NULL;
    pd_uplinkif_t   *pd_upif = NULL;
    pd_uplinkpc_t   *pd_uppc = NULL;
    intf::IfType    if_type;
    uint32_t        hw_lif_id = 0;

    HAL_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            pi_lif = if_get_lif(pi_if);
            HAL_ASSERT(pi_lif != NULL);

            pd_lif = (pd_lif_t *)lif_get_pd_lif(pi_lif);
            HAL_ASSERT(pi_lif != NULL);

            hw_lif_id =  pd_lif->hw_lif_id;
            break;
        case intf::IF_TYPE_UPLINK:
            pd_upif = (pd_uplinkif_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_upif != NULL);

            hw_lif_id = pd_upif->hw_lif_id;
            break;
        case intf::IF_TYPE_UPLINK_PC:
            pd_uppc = (pd_uplinkpc_t *)if_get_pd_if((hal::if_t *)pi_if);
            HAL_ASSERT(pd_uppc != NULL);

            hw_lif_id = pd_uppc->hw_lif_id;
            break;
        case intf::IF_TYPE_TUNNEL:
            break;
        default:
            HAL_ASSERT(0);
    }

    return hw_lif_id;
}

// ----------------------------------------------------------------------------
// Given an Uplink If or PC, get its ifpc id
// ----------------------------------------------------------------------------
uint32_t
if_get_uplink_ifpc_id(if_t *pi_if) 
{
    pd_uplinkif_t   *pd_upif = NULL;
    pd_uplinkpc_t   *pd_uppc = NULL;
    intf::IfType    if_type;
    uint32_t        upifpc_id = 0;

    HAL_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            upifpc_id = 0;
            break;
        case intf::IF_TYPE_UPLINK:
            pd_upif = (pd_uplinkif_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_upif != NULL);

            upifpc_id = pd_upif->up_ifpc_id;
            break;
        case intf::IF_TYPE_UPLINK_PC:
            pd_uppc = (pd_uplinkpc_t *)if_get_pd_if((hal::if_t *)pi_if);
            HAL_ASSERT(pd_uppc != NULL);

            upifpc_id = pd_uppc->up_ifpc_id;
            break;
        case intf::IF_TYPE_TUNNEL:
            break;
        default:
            HAL_ASSERT(0);
    }

    return upifpc_id;
}

// ----------------------------------------------------------------------------
// Get a PI Tenant from if - Applicable only for Enic. Dont call this for Upl.
// ----------------------------------------------------------------------------
tenant_t *
if_get_pi_tenant(if_t *pi_if)
{
    tenant_t        *pi_tenant = NULL;
    intf::IfType    if_type;


    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            pi_tenant = find_tenant_by_id(pi_if->tid);
            HAL_ASSERT_RETURN(pi_tenant != NULL, NULL);
            break;
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            HAL_ASSERT(0);
            break;
        case intf::IF_TYPE_TUNNEL:
            break;
        default:
            HAL_ASSERT(0);
    }
    return pi_tenant;
}

// ----------------------------------------------------------------------------
// Get a PI Tenant from L2 Seg
// ----------------------------------------------------------------------------
tenant_t *
l2seg_get_pi_tenant(l2seg_t *pi_l2seg)
{
    tenant_t    *pi_tenant = NULL;

    pi_tenant = find_tenant_by_id(pi_l2seg->tenant_id);
    HAL_ASSERT_RETURN(pi_tenant != NULL, NULL);

    return pi_tenant;
}


// ----------------------------------------------------------------------------
// Given a PI Tenant, get the nwsec profile hw id
// ----------------------------------------------------------------------------
uint32_t 
ten_get_nwsec_prof_hw_id(tenant_t *pi_ten)
{
    nwsec_profile_t     *pi_nwsec = NULL;
    pd_nwsec_profile_t  *pd_nwsec = NULL;
    uint32_t            nwsec_hw_id = 0;

    pi_nwsec = find_nwsec_profile_by_handle(pi_ten->nwsec_profile_handle);
    if (!pi_nwsec) {
        goto end;
    }

    pd_nwsec = (pd_nwsec_profile_t *)pi_nwsec->pd;

    nwsec_hw_id = pd_nwsec->nwsec_hw_id;

end:
    return nwsec_hw_id;
}

uint8_t *memrev(uint8_t *block, size_t elnum)
{
     uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
     return block;
}


}   // namespace pd
}   // namespace hal
