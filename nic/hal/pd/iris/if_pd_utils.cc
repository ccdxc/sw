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
// Given a PI If and L2 Seg, get its encap vlan
// ----------------------------------------------------------------------------
uint32_t
if_get_encap_vlan(if_t *pi_if, l2seg_t *pi_l2seg)
{
    intf::IfType    if_type;

    HAL_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            return pi_if->encap_vlan;
            break;
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            // Get fabric encap from l2seg
            if (pi_l2seg->fabric_encap.type == types::ENCAP_TYPE_DOT1Q) {
                return pi_l2seg->fabric_encap.val;
            } else {
                // TODO: Handle if its VXLAN
            }
            break;
        case intf::IF_TYPE_TUNNEL:
            // TODO: Handle Tunnels
            break;
        default:
            HAL_ASSERT(0);
    }

    return 0;
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
// Get a qid, qoffset from pi if and qtype
// ----------------------------------------------------------------------------
hal_ret_t
if_get_qid_qoff(if_t *pi_if, intf::LifQType qtype, 
                uint8_t *q_off, uint32_t *qid)
{
    hal_ret_t       ret = HAL_RET_QUEUE_NOT_FOUND;
    intf::IfType    if_type;
    lif_t           *pi_lif = NULL;
    dllist_ctxt_t   *lnode = NULL;
    lif_queue_t     *pi_lif_q;

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            pi_lif = if_get_lif(pi_if);
            HAL_ASSERT_RETURN(pi_lif != NULL, HAL_RET_INVALID_ARG);
            
            dllist_for_each(lnode, &(pi_lif->qlist_head)) {
                pi_lif_q = (lif_queue_t*)((char *)lnode - 
                        offsetof(lif_queue_t, qlist_entry));

                if (pi_lif_q->queue_type == qtype) {
                    *q_off = pi_lif_q->queue_offset;
                    *qid = pi_lif_q->queue_id;

                    ret = HAL_RET_OK;
                    goto end;
                }
            }

            break;
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            ret = HAL_RET_INVALID_ARG;
            break;
        case intf::IF_TYPE_TUNNEL:
            ret = HAL_RET_INVALID_ARG;
            break;
        default:
            HAL_ASSERT(0);
    }

end:
    return ret;
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


}   // namespace pd
}   // namespace hal
