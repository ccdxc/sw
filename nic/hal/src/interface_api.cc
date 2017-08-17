#include <interface.hpp>
#include <pd.hpp>
#include <nwsec.hpp>
#include <interface_api.hpp>

namespace hal {

// ----------------------------------------------------------------------------
// LIF API: Get LIF's Port number
// ----------------------------------------------------------------------------
uint32_t 
lif_get_port_num(lif_t *pi_lif)
{
    return pi_lif->port_num;
}

// ----------------------------------------------------------------------------
// LIF API: Get LIF's ID
// ----------------------------------------------------------------------------
uint32_t
lif_get_lif_id(lif_t *pi_lif)
{
    return pi_lif->lif_id;
}

// ----------------------------------------------------------------------------
// LIF API: Set PD LIF in PI LIF
// ----------------------------------------------------------------------------
void 
lif_set_pd_lif(lif_t *pi_lif, void *pd_lif)
{
    pi_lif->pd_lif = pd_lif;
}

// ----------------------------------------------------------------------------
// Returns PD Lif
// ----------------------------------------------------------------------------
void *
lif_get_pd_lif(lif_t *pi_lif)
{
    return pi_lif->pd_lif;
}
// ----------------------------------------------------------------------------
// IF API: Get If Type
// ----------------------------------------------------------------------------
intf::IfType
intf_get_if_type(if_t *pi_if)
{
    return pi_if->if_type;
}

// ----------------------------------------------------------------------------
// IF API: Get IF's ID
// ----------------------------------------------------------------------------
uint32_t
if_get_if_id(if_t *pi_if)
{
    return pi_if->if_id;
}

// ----------------------------------------------------------------------------
// UplinkIF API: Get UplinkIF's Port number
// ----------------------------------------------------------------------------
uint32_t 
uplinkif_get_port_num(if_t *pi_if)
{
    return pi_if->uplink_port_num;
}

// ----------------------------------------------------------------------------
// IF API: Set PD IF in PI IF to Uplink PD
// ----------------------------------------------------------------------------
void 
if_set_pd_if(if_t *pi_if, void *pd_if)
{
    pi_if->pd_if = pd_if;
}

// ----------------------------------------------------------------------------
// Returns PD If
// ----------------------------------------------------------------------------
void *
if_get_pd_if(if_t *pi_if)
{
    return pi_if->pd_if;
}

// ----------------------------------------------------------------------------
// Returns true if l2seg is native on if
// ----------------------------------------------------------------------------
bool
is_l2seg_native(l2seg_t *l2seg, if_t *pi_if)
{
    // Valid only for Uplink
    if (pi_if->if_type == intf::IF_TYPE_UPLINK &&
            pi_if->native_l2seg == l2seg->seg_id) {
            return TRUE;
    }

    return FALSE;
}

lif_t *
if_get_lif(if_t *pi_if)
{
    if (!pi_if) {
        return NULL;
    }
    // May return NULL for UplinkIf/PC
    if (pi_if->if_type == intf::IF_TYPE_ENIC) {
        return find_lif_by_handle(pi_if->lif_handle);
    } else {
        return NULL;
    }
}

// ----------------------------------------------------------------------------
// Returns enic type of interface
// ----------------------------------------------------------------------------
intf::IfEnicType 
if_get_enicif_type(if_t *pi_if)
{
    // Check if if is enicif
    if (pi_if->if_type == intf::IF_TYPE_ENIC) {
        return pi_if->enic_type;
    } else {
        return intf::IF_ENIC_TYPE_NONE;
    }
}

// ----------------------------------------------------------------------------
// Get the encap vlan
// ----------------------------------------------------------------------------
vlan_id_t 
if_get_encap_vlan(if_t *pi_if)
{
    return pi_if->encap_vlan;
}

// ----------------------------------------------------------------------------
// Get enic ifs mac address
// ----------------------------------------------------------------------------
mac_addr_t *
if_get_mac_addr(if_t *pi_if)
{
    return &(pi_if->mac_addr);
}

// ----------------------------------------------------------------------------
// Get the pd segment for enicif. Assumption a ENIC if has only one l2seg to it.
// ----------------------------------------------------------------------------
void *
if_enicif_get_pd_l2seg(if_t *pi_if)
{
    l2seg_t *pi_seg = NULL;

    pi_seg = (l2seg_t *)if_enicif_get_pi_l2seg(pi_if);
    HAL_ASSERT_RETURN(pi_seg != NULL, NULL);

    return pi_seg->pd;
}

// ----------------------------------------------------------------------------
// Get the pi segment for enicif. Assumption a ENIC if has only one l2seg to it.
// ----------------------------------------------------------------------------
void *
if_enicif_get_pi_l2seg(if_t *pi_if)
{
    l2seg_t *pi_seg = NULL;

    // Check if if is enicif
    if (pi_if->if_type == intf::IF_TYPE_ENIC) {
        pi_seg = find_l2seg_by_id(pi_if->l2seg_id);
#if 0
        if (pi_if->enic_type == intf::IF_ENIC_TYPE_USEG) {
            pi_seg = find_l2seg_by_id(pi_if->l2seg_id);
        } else if (pi_if->enic_type == intf::IF_ENIC_TYPE_PVLAN) {
            // There is no segment for isolated.
            pi_seg = find_l2seg_by_id(pi_if->primary_vlan);
        } else if (pi_if->enic_type == intf::IF_ENIC_TYPE_DIRECT) {
            pi_seg = find_l2seg_by_id(pi_if->native_l2seg_id);
        } else {
            return NULL;
        }
#endif
    } else {
        return NULL;
    }

    return pi_seg;
}

// ----------------------------------------------------------------------------
// Returns nwsec for the enicif
// ----------------------------------------------------------------------------
void *
if_enicif_get_pd_nwsec(if_t *pi_if)
{
    nwsec_profile_t     *pi_nwsec = NULL;

    pi_nwsec = (nwsec_profile_t *)if_enicif_get_pi_nwsec(pi_if);
    if (pi_nwsec) {
        return pi_nwsec->pd;
    } else {
        return NULL;
    }
}

// ----------------------------------------------------------------------------
// Returns nwsec for the interface
// ----------------------------------------------------------------------------
void *
if_enicif_get_pi_nwsec(if_t *pi_if)
{
    tenant_t            *pi_tenant = NULL;
    nwsec_profile_t     *pi_nwsec = NULL;

    // Check if if is enicif
    if (pi_if->if_type == intf::IF_TYPE_ENIC) {
        pi_tenant = find_tenant_by_id(pi_if->tid);
        HAL_ASSERT_RETURN(pi_tenant != NULL, NULL);
        pi_nwsec = find_nwsec_profile_by_handle(pi_tenant->nwsec_profile_handle);
        if (!pi_nwsec) {
            return NULL;
        }
        return pi_nwsec;
    } else {
        return NULL;
    }
}

// ----------------------------------------------------------------------------
// Returns ipsg_en for enicif
// ----------------------------------------------------------------------------
uint32_t
if_enicif_get_ipsg_en(if_t *pi_if)
{
    nwsec_profile_t     *pi_nwsec = NULL;

    pi_nwsec = (nwsec_profile_t *)if_enicif_get_pi_nwsec(pi_if);
    if (!pi_nwsec) {
        return 0;
    }

    return pi_nwsec->ipsg_en;
}


} // namespace hal
