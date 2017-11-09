#include "nic/hal/src/interface.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/src/nwsec.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/src/lif_manager.hpp"

namespace hal {

//----------------------------------------------------------------------------
// LIF API: Get LIF's ID
//----------------------------------------------------------------------------
uint32_t
lif_get_lif_id(lif_t *pi_lif)
{
    return pi_lif->lif_id;
}

//-----------------------------------------------------------------------------
// LIF API: Get qtype from purpose
//-----------------------------------------------------------------------------
uint8_t
lif_get_qtype(lif_t *pi_lif, intf::LifQPurpose purpose)
{
    HAL_ASSERT_RETURN(purpose <= intf::LifQPurpose_MAX, 0);

    return pi_lif->qtypes[purpose];
}

//----------------------------------------------------------------------------
// LIF API: Set PD LIF in PI LIF
//----------------------------------------------------------------------------
void 
lif_set_pd_lif(lif_t *pi_lif, void *pd_lif)
{
    pi_lif->pd_lif = pd_lif;
}

//----------------------------------------------------------------------------
// Returns PD Lif
//----------------------------------------------------------------------------
void *
lif_get_pd_lif(lif_t *pi_lif)
{
    return pi_lif->pd_lif;
}

//----------------------------------------------------------------------------
// IF API: Get If Type
//----------------------------------------------------------------------------
intf::IfType
intf_get_if_type(if_t *pi_if)
{
    if (!pi_if)
        return intf::IfType::IF_TYPE_NONE;
    return pi_if->if_type;
}

//----------------------------------------------------------------------------
// IF API: Get IF's ID
//----------------------------------------------------------------------------
uint32_t
if_get_if_id(if_t *pi_if)
{
    return pi_if->if_id;
}

//----------------------------------------------------------------------------
// UplinkIF API: Get UplinkIF's Port number
//----------------------------------------------------------------------------
uint32_t 
uplinkif_get_port_num(if_t *pi_if)
{
    return pi_if->uplink_port_num;
}

//----------------------------------------------------------------------------
// IF API: Set PD IF in PI IF to Uplink PD
//----------------------------------------------------------------------------
void 
if_set_pd_if(if_t *pi_if, void *pd_if)
{
    pi_if->pd_if = pd_if;
}

//----------------------------------------------------------------------------
// Returns PD If
//----------------------------------------------------------------------------
void *
if_get_pd_if(if_t *pi_if)
{
    return pi_if->pd_if;
}

//----------------------------------------------------------------------------
// Returns true if l2seg is native on if
//----------------------------------------------------------------------------
bool
is_l2seg_native(l2seg_t *l2seg, if_t *pi_if)
{
    // Valid only for Uplink
    if ((pi_if->if_type == intf::IF_TYPE_UPLINK ||
            pi_if->if_type == intf::IF_TYPE_UPLINK_PC) &&
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
    } else if (pi_if->if_type == intf::IF_TYPE_CPU) {
        return find_lif_by_handle(pi_if->lif_handle);
    } else {
        return NULL;
    }
}

//----------------------------------------------------------------------------
// Returns enic type of interface
//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
// Get the encap vlan
//----------------------------------------------------------------------------
vlan_id_t 
if_get_encap_vlan(if_t *pi_if)
{
    return pi_if->encap_vlan;
}

//----------------------------------------------------------------------------
// Get enic ifs mac address
//----------------------------------------------------------------------------
mac_addr_t *
if_get_mac_addr(if_t *pi_if)
{
    return &(pi_if->mac_addr);
}

//----------------------------------------------------------------------------
// Get the pd segment for enicif. Assumption a ENIC if has only one l2seg to it.
//----------------------------------------------------------------------------
void *
if_enicif_get_pd_l2seg(if_t *pi_if)
{
    l2seg_t *pi_seg = NULL;

    pi_seg = (l2seg_t *)if_enicif_get_pi_l2seg(pi_if);
    HAL_ASSERT_RETURN(pi_seg != NULL, NULL);

    return pi_seg->pd;
}

//----------------------------------------------------------------------------
// Get the pi segment for enicif. Assumption a ENIC if has only one l2seg to it.
//----------------------------------------------------------------------------
void *
if_enicif_get_pi_l2seg(if_t *pi_if)
{
    l2seg_t *pi_seg = NULL;

    // Check if if is enicif
    if (pi_if->if_type == intf::IF_TYPE_ENIC) {
        pi_seg = find_l2seg_by_handle(pi_if->l2seg_handle);
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

//----------------------------------------------------------------------------
// Returns nwsec for the enicif
//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
// Returns nwsec for the interface
//----------------------------------------------------------------------------
void *
if_enicif_get_pi_nwsec(if_t *pi_if)
{
    tenant_t            *pi_tenant = NULL;
    nwsec_profile_t     *pi_nwsec = NULL;

    // Check if if is enicif
    if (pi_if->if_type == intf::IF_TYPE_ENIC) {
        pi_tenant = tenant_lookup_by_id(pi_if->tid);
        HAL_ASSERT_RETURN(pi_tenant != NULL, NULL);
        pi_nwsec =
            find_nwsec_profile_by_handle(pi_tenant->nwsec_profile_handle);
        if (!pi_nwsec) {
            return NULL;
        }
        return pi_nwsec;
    } else {
        return NULL;
    }
}

//----------------------------------------------------------------------------
// Returns ipsg_en for enicif
//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
// Returns hwlif id
//----------------------------------------------------------------------------
hal_ret_t
if_enicif_get_pinned_if(if_t *pi_if, if_t **uplink_if)
{
    hal_ret_t           ret     = HAL_RET_OK;
    intf::IfType        if_type = intf::IF_TYPE_NONE;
    lif_t               *lif    = NULL;
    hal_handle_t        uplink_hdl = HAL_HANDLE_INVALID;

    HAL_ASSERT(pi_if != NULL);

    *uplink_if = NULL;

    if_type = intf_get_if_type(pi_if);
    if (if_type != intf::IF_TYPE_ENIC) {
        HAL_TRACE_ERR("{}:pinned if is only for ENIC. if_type:{}", 
                      __FUNCTION__, if_type);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // Enic has pinned uplink, return it 
    if (pi_if->pinned_uplink != HAL_HANDLE_INVALID) {
        uplink_hdl = pi_if->pinned_uplink;
    } else {
        // Take from lif
        lif = find_lif_by_handle(pi_if->lif_handle);
        if (!lif) {
            HAL_TRACE_ERR("{}:unable to find lif for lif_hdl:{}",
                          __FUNCTION__, pi_if->lif_handle);
            ret = HAL_RET_LIF_NOT_FOUND;
            goto end;
        }
        uplink_hdl = lif->pinned_uplink;
    }

    if (uplink_hdl != HAL_HANDLE_INVALID) {
        *uplink_if = find_if_by_handle(uplink_hdl);
        if (!*uplink_if) {
            HAL_TRACE_ERR("{}:unable to find if for if_hdl:{}", 
                          __FUNCTION__, uplink_hdl);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        }
    } else {
        HAL_TRACE_ERR("{}:no pinned uplink for enic_if_id:{}",
                      __FUNCTION__, pi_if->if_id);
        ret = HAL_RET_INVALID_OP;
    }

end:
    return ret;
}


//----------------------------------------------------------------------------
// Returns hwlif id
//----------------------------------------------------------------------------
uint32_t
if_allocate_hwlif_id()
{
    int32_t hw_lif_id = -1;

    hw_lif_id = g_lif_manager->LIFRangeAlloc(-1, 1);
    if (hw_lif_id < 0) {
        HAL_TRACE_ERR("Failed to allocate hw_lif_id : {}", hw_lif_id);
        return INVALID_INDEXER_INDEX;
    }
    return (uint32_t)hw_lif_id;
}

//----------------------------------------------------------------------------
// Given a PI If and L2 Seg, get its encap vlan
//----------------------------------------------------------------------------
uint32_t
if_l2seg_get_encap_vlan(if_t *pi_if, l2seg_t *pi_l2seg)
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
            HAL_ASSERT(0);
            break;
        default:
            HAL_ASSERT(0);
    }

    return 0;
}

//----------------------------------------------------------------------------
// Returns the encap used for l2seg on an if. 
// Assumption: Ingress & Egress are same.
//----------------------------------------------------------------------------
hal_ret_t   
if_l2seg_get_encap(if_t *pi_if, l2seg_t *pi_l2seg, uint8_t *vlan_v,
                           uint16_t *vlan_id)
{
    if (!pi_if && !pi_l2seg) {
        return HAL_RET_INVALID_ARG;
    }

    switch(pi_if->if_type) {
        case intf::IF_TYPE_ENIC:
            if (pi_if->enic_type == intf::IF_ENIC_TYPE_USEG || 
                    pi_if->enic_type == intf::IF_ENIC_TYPE_PVLAN) {
                *vlan_v = 1;
                *vlan_id = if_l2seg_get_encap_vlan(pi_if, pi_l2seg);
            } else {
                // Direct
                *vlan_v = 0;
                *vlan_id = 0;
            }
            break;
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            if (is_l2seg_native(pi_l2seg, pi_if)) {
                *vlan_v = 0;
                *vlan_id = 0;
            } else {
                *vlan_v = 1;
                *vlan_id = if_l2seg_get_encap_vlan(pi_if, pi_l2seg);
            }
            break;
        case intf::IF_TYPE_TUNNEL:
            // DO NOT REMOVE HAL_ASSERT
            // This API is not expected to be called for tunnel-if
            HAL_ASSERT(0);
            break;
        default:
            HAL_ASSERT(0);
    }

    return HAL_RET_OK;
}

//----------------------------------------------------------------------------
// LIF API: Get LIF's rdma enable status
//----------------------------------------------------------------------------
bool
lif_get_enable_rdma(lif_t *pi_lif)
{
    return pi_lif->enable_rdma;
}

//----------------------------------------------------------------------------
// LIF API: Set rdma enable in PI LIF
//----------------------------------------------------------------------------
void 
lif_set_enable_rdma(lif_t *pi_lif, bool enable_rdma)
{
    pi_lif->enable_rdma = enable_rdma;
}


//----------------------------------------------------------------------------
// LIF API: Get total number of LIF queues across all qtypes.
//----------------------------------------------------------------------------
uint32_t
lif_get_total_qcount (uint32_t hw_lif_id) 
{
    uint32_t total_qcount = 0, i = 0;

    LIFQState *qstate = g_lif_manager->GetLIFQState(hw_lif_id);

    if (qstate == NULL) 
        goto end;

    for (i = 0; i < kNumQTypes; i++) {
        total_qcount += qstate->type[i].num_queues;
    }

end:
    return total_qcount;
}
} // namespace hal

