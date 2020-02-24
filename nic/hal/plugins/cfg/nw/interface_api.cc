//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "lib/ht/ht.hpp"

namespace hal {

#define HAL_BOND0_ACTIVE_IF_FILENAME "/sys/class/net/bond0/bonding/active_slave"

uint32_t
lif_get_lif_id (lif_t *pi_lif)
{
    return pi_lif->lif_id;
}

uint8_t
lif_get_qtype (lif_t *pi_lif, intf::LifQPurpose purpose)
{
    SDK_ASSERT_RETURN(purpose <= intf::LifQPurpose_MAX, 0);

    return pi_lif->qinfo[purpose].type;
}

void
lif_set_pd_lif (lif_t *pi_lif, void *pd_lif)
{
    pi_lif->pd_lif = pd_lif;
}

void *
lif_get_pd_lif (lif_t *pi_lif)
{
    if (pi_lif) {
        return pi_lif->pd_lif;
    }
    return NULL;
}

qos_class_t *
lif_get_tx_qos_class (lif_t *pi_lif)
{
    hal_handle_t qos_class_handle;

    if (!pi_lif) {
        return NULL;
    }
    qos_class_handle = pi_lif->qos_info.tx_qos_class_handle;
    return find_qos_class_by_handle(qos_class_handle);
}

qos_class_t *
lif_get_rx_qos_class (lif_t *pi_lif)
{
    hal_handle_t qos_class_handle;

    if (!pi_lif) {
        return NULL;
    }
    qos_class_handle = pi_lif->qos_info.rx_qos_class_handle;
    return find_qos_class_by_handle(qos_class_handle);
}

intf::IfType
intf_get_if_type (if_t *pi_if)
{
    if (!pi_if) {
        return intf::IfType::IF_TYPE_NONE;
    }
    return pi_if->if_type;
}

uint32_t
if_get_if_id (if_t *pi_if)
{
    return pi_if->if_id;
}

hal_handle_t
if_get_hal_handle (if_t *pi_if)
{
    return pi_if->hal_handle;
}

uint32_t
uplinkif_get_port_num (if_t *pi_if)
{
    return pi_if->uplink_port_num;
}

void
if_set_pd_if (if_t *pi_if, void *pd_if)
{
    pi_if->pd_if = pd_if;
}

//----------------------------------------------------------------------------
// returns PD If
//----------------------------------------------------------------------------
void *
if_get_pd_if (if_t *pi_if)
{
    return pi_if->pd_if;
}

//----------------------------------------------------------------------------
// returns true if l2seg is native on if
//----------------------------------------------------------------------------
bool
is_l2seg_native (l2seg_t *l2seg, if_t *pi_if)
{
    // Valid only for Uplink
    if ((pi_if->if_type == intf::IF_TYPE_UPLINK ||
            pi_if->if_type == intf::IF_TYPE_UPLINK_PC) &&
            (pi_if->native_l2seg == l2seg->seg_id ||
             (l2seg->wire_encap.type == types::ENCAP_TYPE_DOT1Q &&
              (l2seg->wire_encap.val == NATIVE_VLAN_ID || 
               (l2seg->wire_encap.val >= NATIVE_TELEMETRY_VLAN_ID_START && 
                l2seg->wire_encap.val <= NATIVE_TELEMETRY_VLAN_ID_END))))) {
        return TRUE;
    }

    return FALSE;
}

lif_t *
if_get_lif (if_t *pi_if)
{
    if (!pi_if) {
        return NULL;
    }
    if (pi_if->if_type == intf::IF_TYPE_ENIC) {
        return find_lif_by_handle(pi_if->lif_handle);
    } else if (pi_if->if_type == intf::IF_TYPE_CPU) {
        return find_lif_by_handle(pi_if->lif_handle);
    } else if (pi_if->if_type == intf::IF_TYPE_APP_REDIR) {
        return find_lif_by_handle(pi_if->lif_handle);
    } else {
        return NULL;
    }
}

//----------------------------------------------------------------------------
// returns enic type of interface
//----------------------------------------------------------------------------
intf::IfEnicType
if_get_enicif_type (if_t *pi_if)
{
    // Check if if is enicif
    if (pi_if->if_type == intf::IF_TYPE_ENIC) {
        return pi_if->enic_type;
    } else {
        return intf::IF_ENIC_TYPE_NONE;
    }
}

//----------------------------------------------------------------------------
// returns enic's lif type
//----------------------------------------------------------------------------
types::LifType
if_get_enicif_lif_type (if_t *pi_if)
{
    lif_t *lif = NULL;
    // Check if if is enicif
    if (pi_if->if_type == intf::IF_TYPE_ENIC) {
        if (pi_if->enic_type == intf::IF_ENIC_TYPE_USEG ||
            pi_if->enic_type == intf::IF_ENIC_TYPE_PVLAN) {
            return types::LIF_TYPE_HOST;
        } else {
            lif = if_get_lif(pi_if);
            if (lif) {
                return lif->type;
            }
        }
    }
    return types::LIF_TYPE_NONE;
}

//----------------------------------------------------------------------------
// get the encap vlan
//----------------------------------------------------------------------------
vlan_id_t
if_get_encap_vlan (if_t *pi_if)
{
    return pi_if->encap_vlan;
}

//----------------------------------------------------------------------------
// get enic ifs mac address
//----------------------------------------------------------------------------
mac_addr_t *
if_get_mac_addr (if_t *pi_if)
{
    return &(pi_if->mac_addr);
}

//----------------------------------------------------------------------------
// get the uplink for the Host Pinned mode
//----------------------------------------------------------------------------
uint32_t
if_enicif_get_host_pinned_uplink (if_t *pi_if)
{

    if (g_num_uplink_ifs) {
        uint8_t hashval = (sdk::lib::hash_algo::fnv_hash(if_get_mac_addr(pi_if), ETH_ADDR_LEN) % g_num_uplink_ifs);
        return g_uplink_if_ids[hashval];
    }
    return 0;
}

//----------------------------------------------------------------------------
// get the pd segment for enicif. Assumption a ENIC if has only one l2seg to it.
//----------------------------------------------------------------------------
void *
if_enicif_get_pd_l2seg (if_t *pi_if)
{
    l2seg_t *pi_seg = NULL;

    pi_seg = (l2seg_t *)if_enicif_get_pi_l2seg(pi_if);
    SDK_ASSERT_RETURN(pi_seg != NULL, NULL);

    return pi_seg->pd;
}

//----------------------------------------------------------------------------
// get the pi segment for enicif. Assumption a ENIC if has only one l2seg to it.
//----------------------------------------------------------------------------
void *
if_enicif_get_pi_l2seg (if_t *pi_if)
{
    l2seg_t *pi_seg = NULL;

    // Check if if is enicif
    if (pi_if->if_type == intf::IF_TYPE_ENIC) {
        pi_seg = l2seg_lookup_by_handle(pi_if->l2seg_handle);
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
// returns nwsec for the enicif
//----------------------------------------------------------------------------
void *
if_enicif_get_pd_nwsec (if_t *pi_if)
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
if_enicif_get_pi_nwsec (if_t *pi_if)
{
    vrf_t              *pi_vrf = NULL;
    nwsec_profile_t    *pi_nwsec = NULL;

    if (pi_if->if_type == intf::IF_TYPE_ENIC) {
        if (pi_if->tid == 0) {
            return NULL;
        }
        pi_vrf = vrf_lookup_by_id(pi_if->tid);
        SDK_ASSERT_RETURN(pi_vrf != NULL, NULL);
        pi_nwsec =
            find_nwsec_profile_by_handle(pi_vrf->nwsec_profile_handle);
        if (!pi_nwsec) {
            return NULL;
        }
        return pi_nwsec;
    } else {
        return NULL;
    }
}

//----------------------------------------------------------------------------
// returns ipsg_en for enicif
//----------------------------------------------------------------------------
uint32_t
if_enicif_get_ipsg_en (if_t *pi_if)
{
    nwsec_profile_t     *pi_nwsec = NULL;

    pi_nwsec = (nwsec_profile_t *)if_enicif_get_pi_nwsec(pi_if);
    if (!pi_nwsec) {
        return 0;
    }

    return pi_nwsec->ipsg_en;
}

//-----------------------------------------------------------------------------
// get native vlan id for classic enic
//-----------------------------------------------------------------------------
hal_ret_t
if_enicif_get_native_l2seg_clsc_vlan (if_t *pi_if, uint32_t *vlan_id)
{
    hal_ret_t           ret     = HAL_RET_OK;
    intf::IfType        if_type = intf::IF_TYPE_NONE;
    l2seg_t             *pi_seg = NULL;

    SDK_ASSERT(pi_if != NULL && vlan_id != NULL);

    if_type  = intf_get_if_type(pi_if);
    *vlan_id = 0;

    if (if_type != intf::IF_TYPE_ENIC) {
        HAL_TRACE_ERR("Native l2seg classic is only for ENIC, if_type : {}",
                      if_type);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    if (pi_if->native_l2seg_clsc != HAL_HANDLE_INVALID) {
        pi_seg = l2seg_lookup_by_handle(pi_if->native_l2seg_clsc);
        if (pi_seg == NULL) {
            HAL_TRACE_ERR("Native l2seg is not present: l2seg_hdl : {}",
                          pi_if->native_l2seg_clsc);
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }

        // Check if wire encap is vlan
        if (pi_seg->wire_encap.type != types::ENCAP_TYPE_DOT1Q) {
            HAL_TRACE_ERR("Native l2seg doesn't have wire encap");
            ret = HAL_RET_INVALID_ARG;
            goto end;
        }

        *vlan_id = pi_seg->wire_encap.val;
    }

end:

    return ret;
}

if_t *
lif_get_pinned_uplink (lif_t *lif)
{
    hal_handle_t        uplink_hdl = HAL_HANDLE_INVALID;
    if_t                *uplink_if = NULL;

    uplink_hdl = lif->pinned_uplink;

    if (uplink_hdl != HAL_HANDLE_INVALID) {
        uplink_if = find_if_by_handle(uplink_hdl);
        if (!uplink_if) {
            HAL_TRACE_ERR("Unable to find if for if_hdl {}", uplink_hdl);
        }
    } else {
        HAL_TRACE_DEBUG("No pinned uplink for lif {}", lif->lif_id);
    }
    return uplink_if;
}

//----------------------------------------------------------------------------
// Pinned IF for Enic IF in Classic Mode
//----------------------------------------------------------------------------
hal_ret_t
if_enicif_get_pinned_if (if_t *pi_if, if_t **uplink_if)
{
    hal_ret_t           ret     = HAL_RET_OK;
    intf::IfType        if_type = intf::IF_TYPE_NONE;
    lif_t               *lif    = NULL;
    hal_handle_t        uplink_hdl = HAL_HANDLE_INVALID;

    SDK_ASSERT(pi_if != NULL);

    *uplink_if = NULL;

    if_type = intf_get_if_type(pi_if);
    if (if_type != intf::IF_TYPE_ENIC) {
        HAL_TRACE_ERR("Pinned if is only for ENIC, if_type : {}", if_type);
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
            HAL_TRACE_ERR("Unable to find lif for lif_hdl {}",
                          pi_if->lif_handle);
            ret = HAL_RET_LIF_NOT_FOUND;
            goto end;
        }
        uplink_hdl = lif->pinned_uplink;
    }

    if (uplink_hdl != HAL_HANDLE_INVALID) {
        *uplink_if = find_if_by_handle(uplink_hdl);
        if (!*uplink_if) {
            HAL_TRACE_ERR("Unable to find if for if_hdl {}", uplink_hdl);
            ret = HAL_RET_IF_NOT_FOUND;
            goto end;
        }
    } else {
        HAL_TRACE_DEBUG("No pinned uplink for enic_if_id {}", pi_if->if_id);
        ret = HAL_RET_INVALID_OP;
    }

end:

    return ret;
}

//----------------------------------------------------------------------------
// returns hwlif id
//----------------------------------------------------------------------------
uint32_t
if_allocate_hwlif_id (void)
{
    sdk_ret_t ret = SDK_RET_OK;
    uint32_t hw_lif_id = INVALID_INDEXER_INDEX;

    ret = lif_manager()->alloc_id(&hw_lif_id, 1);
    if (ret != SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to allocate hw_lif_id : {}", hw_lif_id);
        return INVALID_INDEXER_INDEX;
    }
    return hw_lif_id;
}

void
if_free_hwlif_id(uint32_t hwlif_id)
{
    // lif_manager()->DeleteLIF(hwlif_id);
    lif_manager()->remove(hwlif_id);
}

//----------------------------------------------------------------------------
// given a PI If and L2 Seg, get its encap vlan
//----------------------------------------------------------------------------
uint32_t
if_l2seg_get_encap_vlan (if_t *pi_if, l2seg_t *pi_l2seg)
{
    intf::IfType    if_type;

    SDK_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            return pi_if->encap_vlan;
            break;
        case intf::IF_TYPE_UPLINK:
        case intf::IF_TYPE_UPLINK_PC:
            // Get wire encap from l2seg
            if (pi_l2seg->wire_encap.type == types::ENCAP_TYPE_DOT1Q) {
                return pi_l2seg->wire_encap.val;
            } else {
                // TODO: Handle if its VXLAN
            }
            break;
        case intf::IF_TYPE_TUNNEL:
            // TODO: Handle Tunnels
            SDK_ASSERT(0);
            break;
        default:
            SDK_ASSERT(0);
    }

    return 0;
}

//----------------------------------------------------------------------------
// returns the encap used for l2seg on an if.
// Assumption: Ingress & Egress are same.
//----------------------------------------------------------------------------
hal_ret_t
if_l2seg_get_encap (if_t *pi_if, l2seg_t *pi_l2seg, uint8_t *vlan_v,
                    uint16_t *vlan_id)
{
    if (!pi_if && !pi_l2seg) {
        return HAL_RET_INVALID_ARG;
    }

    switch(pi_if->if_type) {
    case intf::IF_TYPE_ENIC:
        switch (pi_if->enic_type) {
        case intf::IF_ENIC_TYPE_USEG:
        case intf::IF_ENIC_TYPE_PVLAN:
            *vlan_v = TRUE;
            *vlan_id = if_l2seg_get_encap_vlan(pi_if, pi_l2seg);
            break;
        case intf::IF_ENIC_TYPE_CLASSIC:
            if (pi_if->native_l2seg_clsc == pi_l2seg->hal_handle) {
                *vlan_v = FALSE;
                *vlan_id = 0;
            } else {
                *vlan_v = TRUE;
                *vlan_id = 0; // Retain original vlan for classic case
            }
            break;
        case intf::IF_ENIC_TYPE_DIRECT:
            *vlan_v = FALSE;
            *vlan_id = 0;
            break;
        default:
            SDK_ASSERT(0);
            break;
    }
    break;

    case intf::IF_TYPE_UPLINK:
    case intf::IF_TYPE_UPLINK_PC:
        if (is_l2seg_native(pi_l2seg, pi_if)) {
            *vlan_v = FALSE;
            *vlan_id = 0;
        } else {
            *vlan_v = TRUE;
            *vlan_id = if_l2seg_get_encap_vlan(pi_if, pi_l2seg);
        }
        if (*vlan_id == NATIVE_VLAN_ID) {
            *vlan_v = FALSE;
            *vlan_id = 0;
        }
        break;

    case intf::IF_TYPE_TUNNEL:
        // DO NOT REMOVE SDK_ASSERT
        // This API is not expected to be called for tunnel-if
        SDK_ASSERT(0);
        break;

    default:
        SDK_ASSERT(0);
    }

    HAL_TRACE_VERBOSE("{}, L2seg: {}, vlan_v: {}, vlan_id: {}",
                      pi_if->if_id, pi_l2seg->seg_id,
                      *vlan_v, *vlan_id);

    return HAL_RET_OK;
}

//----------------------------------------------------------------------------
// LIF API: Get LIF's rdma enable status
//----------------------------------------------------------------------------
bool
lif_get_enable_rdma (lif_t *pi_lif)
{
    return pi_lif->enable_rdma;
}

//----------------------------------------------------------------------------
// LIF API: Set rdma enable in PI LIF
//----------------------------------------------------------------------------
void
lif_set_enable_rdma (lif_t *pi_lif, bool enable_rdma)
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

    // LIFQState *qstate = lif_manager()->GetLIFQState(hw_lif_id);
    lif_qstate_t *qstate = lif_manager()->get_lif_qstate(hw_lif_id);

    if (qstate == NULL)
        goto end;

    // for (i = 0; i < kNumQTypes; i++) {
    for (i = 0; i < 8; i++) {
        total_qcount += qstate->type[i].num_queues;
    }

end:
    return total_qcount;

}

if_t *
inband_mgmt_get_active_if (void)
{
    FILE *fptr = fopen(HAL_BOND0_ACTIVE_IF_FILENAME, "r");
    char ifname_str[LIF_NAME_LEN] = {0};
    if_t *act_if = NULL;
    lif_t *lif = NULL;

    if (!fptr) {
        HAL_TRACE_DEBUG("Failed to open bond0 active link file");
        goto end;
    }
    fscanf(fptr, "%s", ifname_str);
    lif = find_lif_by_name(ifname_str);
    if (!lif) {
        HAL_TRACE_DEBUG("Failed to get lif for ifname {}",
                        ifname_str);
        goto end;
    }
    act_if = find_if_by_handle(lif->pinned_uplink);
    if (!act_if) {
        HAL_TRACE_ERR("Failed to get pinned uplink hdl {} of lif",
                      lif->pinned_uplink);
        goto end;
    }

end:
    if (fptr) {
        fclose(fptr);
    }
    return act_if;
}

} // namespace hal

