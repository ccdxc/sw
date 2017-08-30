#include <l2segment.hpp>
#include <l2segment_api.hpp>
#include <nwsec.hpp>

namespace hal {


// ----------------------------------------------------------------------------
// Returns L2 segment PD
// ----------------------------------------------------------------------------
void *
l2seg_get_pd(l2seg_t *seg)
{
    return seg->pd;
}

// ----------------------------------------------------------------------------
// Returns L2 segment ID
// ----------------------------------------------------------------------------
l2seg_id_t
l2seg_get_l2seg_id(l2seg_t *seg)
{
    return seg->seg_id;
}

// ----------------------------------------------------------------------------
// Returns access encap type
// ----------------------------------------------------------------------------
types::encapType 
l2seg_get_acc_encap_type(l2seg_t *seg)
{
    return seg->access_encap.type;
}

// ----------------------------------------------------------------------------
// Returns access encap value
// ----------------------------------------------------------------------------
uint32_t 
l2seg_get_acc_encap_val(l2seg_t *seg)
{
    return seg->access_encap.val;
}

// ----------------------------------------------------------------------------
// Returns fabric encap type
// ----------------------------------------------------------------------------
types::encapType 
l2seg_get_fab_encap_type(l2seg_t *seg)
{
    return seg->fabric_encap.type;
}

// ----------------------------------------------------------------------------
// Returns fabric encap value
// ----------------------------------------------------------------------------
uint32_t 
l2seg_get_fab_encap_val(l2seg_t *seg)
{
    return seg->fabric_encap.val;
}

// ----------------------------------------------------------------------------
// Returns nwsec for the l2segment
// ----------------------------------------------------------------------------
void *
l2seg_get_pi_nwsec(l2seg_t *l2seg)
{
    tenant_t            *pi_tenant = NULL;
    nwsec_profile_t     *pi_nwsec = NULL;

    // Check if if is enicif
    pi_tenant = find_tenant_by_id(l2seg->tenant_id);
    HAL_ASSERT_RETURN(pi_tenant != NULL, NULL);
    pi_nwsec = find_nwsec_profile_by_handle(pi_tenant->nwsec_profile_handle);
    if (!pi_nwsec) {
        return NULL;
    }
    return pi_nwsec;
}

// ----------------------------------------------------------------------------
// Returns ipsg_en for l2seg
// ----------------------------------------------------------------------------
uint32_t
l2seg_get_ipsg_en(l2seg_t *pi_l2seg)
{
    nwsec_profile_t     *pi_nwsec = NULL;

    pi_nwsec = (nwsec_profile_t *)l2seg_get_pi_nwsec(pi_l2seg);
    if (!pi_nwsec) {
        return 0;
    }

    return pi_nwsec->ipsg_en;
}

// ----------------------------------------------------------------------------
// Returns infra L2 segment
// ----------------------------------------------------------------------------
l2seg_t *
l2seg_get_infra_l2seg()
{
    return (l2seg_t *)g_hal_state->infra_l2seg();
}





}
