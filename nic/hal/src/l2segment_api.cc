// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/hal/src/interface.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/include/l2segment_api.hpp"
#include "nic/hal/src/nwsec.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/hal/src/lif_manager.hpp"

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
// Returns tunnel encap type
// ----------------------------------------------------------------------------
types::encapType 
l2seg_get_tunnel_encap_type(l2seg_t *seg)
{
    return seg->tunnel_encap.type;
}

// ----------------------------------------------------------------------------
// Returns tunnel encap value
// ----------------------------------------------------------------------------
uint32_t 
l2seg_get_tunnel_encap_val(l2seg_t *seg)
{
    return seg->tunnel_encap.val;
}

// ----------------------------------------------------------------------------
// Returns Handle of the pinned uplink interface, if any
// ----------------------------------------------------------------------------
hal_handle_t
l2seg_get_pinned_uplink(l2seg_t *seg)
{
    return seg->pinned_uplink;
}

// ----------------------------------------------------------------------------
// Returns wire encap type
// ----------------------------------------------------------------------------
types::encapType 
l2seg_get_wire_encap_type(l2seg_t *seg)
{
    return seg->wire_encap.type;
}

// ----------------------------------------------------------------------------
// Returns wire encap value
// ----------------------------------------------------------------------------
uint32_t 
l2seg_get_wire_encap_val(l2seg_t *seg)
{
    return seg->wire_encap.val;
}

// ----------------------------------------------------------------------------
// Returns nwsec for the l2segment
// ----------------------------------------------------------------------------
void *
l2seg_get_pi_nwsec(l2seg_t *l2seg)
{
    vrf_t            *pi_vrf = NULL;
    nwsec_profile_t     *pi_nwsec = NULL;

    // Check if if is enicif
    pi_vrf = vrf_lookup_by_handle(l2seg->vrf_handle);
    HAL_ASSERT_RETURN(pi_vrf != NULL, NULL);
    pi_nwsec = find_nwsec_profile_by_handle(pi_vrf->nwsec_profile_handle);
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

// ----------------------------------------------------------------------------
// Returns bcast_fwd_policy
// ----------------------------------------------------------------------------
uint32_t
l2seg_get_bcast_fwd_policy(l2seg_t *pi_l2seg)
{
    return (uint32_t)pi_l2seg->bcast_fwd_policy;
}

// ----------------------------------------------------------------------------
// Returns bcast_oif_list
// ----------------------------------------------------------------------------
oif_list_id_t
l2seg_get_bcast_oif_list(l2seg_t *pi_l2seg)
{
    return pi_l2seg->bcast_oif_list;
}

// ----------------------------------------------------------------------------
// Returns bcast_oif_list
// ----------------------------------------------------------------------------
ip_addr_t
*l2seg_get_gipo(l2seg_t *pi_l2seg)
{
    return &(pi_l2seg->gipo);
}

// ----------------------------------------------------------------------------
// Returns router mac
// ----------------------------------------------------------------------------
mac_addr_t *l2seg_get_rtr_mac(l2seg_t *pi_l2seg)
{
    network_t                   *nw = NULL;
    hal_handle_t                *p_hdl_id = NULL;

    for (const void *ptr : *pi_l2seg->nw_list) {
        p_hdl_id = (hal_handle_t *)ptr;
        nw = find_network_by_handle(*p_hdl_id);
        return &nw->rmac_addr;
    }

    return NULL;
}

}
