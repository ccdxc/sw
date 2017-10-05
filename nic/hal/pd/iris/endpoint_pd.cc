#include <netinet/ether.h>
#include "nic/include/hal_lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/include/endpoint_api.hpp"
#include "nic/include/interface_api.hpp"
#include "nic/hal/pd/iris/endpoint_pd.hpp"
#include "nic/hal/pd/iris/lif_pd.hpp"
#include "nic/hal/pd/iris/uplinkif_pd.hpp"
#include "nic/hal/pd/iris/uplinkpc_pd.hpp"
#include "nic/hal/src/l2segment.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/hal/pd/iris/l2seg_pd.hpp"
#include "nic/p4/nw/include/defines.h"
#include "nic/hal/pd/iris/if_pd_utils.hpp"
#include "nic/hal/src/utils.hpp"

using namespace hal;

namespace hal {
namespace pd {

// ----------------------------------------------------------------------------
// EP Create
// ----------------------------------------------------------------------------
hal_ret_t 
pd_ep_create(pd_ep_args_t *args)
{
    hal_ret_t            ret = HAL_RET_OK;; 
    pd_ep_t             *pd_ep;
    mac_addr_t           *mac;

    mac = ep_get_mac_addr(args->ep);

    HAL_TRACE_DEBUG("pd-ep::{}: creating pd state for ep: {}", 
                    __FUNCTION__, ep_l2_key_to_str(args->ep));
                    
    // Create ep PD
    pd_ep = ep_pd_alloc_init();
    if (pd_ep == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    ep_link_pi_pd(pd_ep, args->ep);

    // Create EP L3 entry PDs
    ret = ep_pd_alloc_ip_entries(args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-ep::{}: unable to alloc. ip entries: ep:{}",
                      __FUNCTION__, ep_l2_key_to_str(args->ep));
        goto end;
    }

    // Allocate Resources
    ret = ep_pd_alloc_res(pd_ep);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-EP::{}: Unable to alloc. resources for EP: {}:{}", 
                      __FUNCTION__, ep_get_l2segid(args->ep), 
                ether_ntoa((struct ether_addr*)*mac));
        goto end;
    }

    // Program HW
    ret = ep_pd_program_hw(pd_ep);

end:
    if (ret != HAL_RET_OK) {
        // unlink_pi_pd(pd_ep, args->ep);
        // ep_pd_free(pd_ep);
        ep_pd_cleanup(pd_ep);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// EP Update 
// ----------------------------------------------------------------------------
hal_ret_t 
pd_ep_update (pd_ep_upd_args_t *pd_ep_upd_args)
{
    hal_ret_t           ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("pd-ep::{}: updating pd state for ep:{}", 
                    __FUNCTION__,
                    ep_l2_key_to_str(pd_ep_upd_args->ep));

    if (pd_ep_upd_args->iplist_change) {
        ret = pd_ep_upd_iplist_change(pd_ep_upd_args);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD Endpoint Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_ep_delete (pd_ep_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_ep_t    *ep_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->ep != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->ep->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-ep:{}:deleting pd state for ep {}",
                    __FUNCTION__, ep_l2_key_to_str(args->ep));
    ep_pd = (pd_ep_t *)args->ep->pd;

    // deprogram hw
    ret = ep_pd_depgm_ipsg_tbl_ip_entries(args->ep,
                                          &(args->ep->ip_list_head));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-EP: Failed to depgm IPSG, ret:{}", ret);
        goto end;
    }

    // free up the resource and memory
    ret = ep_pd_cleanup(ep_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-ep:{}:failed pd ep delete",
                      __FUNCTION__);
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD Endpoint Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD Endpoint
//  Note:
//      - Just free up whatever PD has. 
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
ep_pd_cleanup(pd_ep_t *ep_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!ep_pd) {
        // Nothing to do
        goto end;
    }

    // Releasing resources
    ret = ep_pd_dealloc_res(ep_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-ep:{}: unable to dealloc res for ep: {}", 
                      __FUNCTION__, 
                      (ep_l2_key_to_str((ep_t *)(ep_pd->pi_ep))));
        goto end;
    }

    // Free up IPs PD State
    ret = ep_pd_delete_pd_ip_entries((ep_t *)ep_pd->pi_ep,
                                     &((ep_t *)(ep_pd->pi_ep))->ip_list_head);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-ep: failed to free pd ip entries. ret:{}", ret);
        goto end;
    }


    // Delinking PI<->PD
    ep_delink_pi_pd(ep_pd, (ep_t *)ep_pd->pi_ep);

    // Freeing PD
    ep_pd_free(ep_pd);
end:
    return ret;
}


// ----------------------------------------------------------------------------
// EP Update: Handling ip list change
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_upd_iplist_change (pd_ep_upd_args_t *pd_ep_upd_args)
{
    hal_ret_t       ret = HAL_RET_OK;

    HAL_TRACE_DEBUG("pd-ep:{} ip-list change: ", __FUNCTION__);

    // Allocated PD State for new IP entries
    ret = ep_pd_alloc_pd_ip_entries(pd_ep_upd_args->add_iplist);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-ep: failed to alloced pd ip entries "
                "for new ip. ret:{}", ret);
        goto end;
    }

    // Program IPSG entries for new IPs
    ret = ep_pd_pgm_ipsg_tbl_ip_entries(pd_ep_upd_args->ep,
                                        pd_ep_upd_args->add_iplist);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-ep: failed to pgm IPSG"
                "for new ip. ret:{}", ret);
        goto end;
    }

    // Deprogram IPSG entries for deleted IPs
    ret = ep_pd_depgm_ipsg_tbl_ip_entries(pd_ep_upd_args->ep,
                                        pd_ep_upd_args->del_iplist);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-ep: failed to depgm IPSG"
                "for deleted ip. ret:{}", ret);
        goto end;
    }

    // free up delete IPs PD state
    ret = ep_pd_delete_pd_ip_entries(pd_ep_upd_args->ep,
                                     pd_ep_upd_args->del_iplist);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-ep: failed to free pd ip entries. ret:{}", ret);
        goto end;
    }


end:
    return ret;
}


hal_ret_t
ep_pd_alloc_pd_ip_entries (dllist_ctxt_t *pi_ep_list)
{
    hal_ret_t       ret = HAL_RET_OK;
    dllist_ctxt_t   *lnode = NULL;
    ep_ip_entry_t   *pi_ip_entry = NULL;

    // Walk through ip entries
    dllist_for_each(lnode, pi_ep_list) {
        pi_ip_entry = dllist_entry(lnode, ep_ip_entry_t, ep_ip_lentry);

        pi_ip_entry->pd = (pd_ep_ip_entry_t *)g_hal_state_pd->
                          ep_pd_ip_entry_slab()->alloc();
        if (!pi_ip_entry->pd) {
            ret = HAL_RET_OOM;
            goto end;
        }

        // Link PI to PD
        pi_ip_entry->pd->pi_ep_ip_entry = pi_ip_entry;

        HAL_TRACE_DEBUG("PD-EP:{}: Allocating PD IP Entry: {}",
                __FUNCTION__, ipaddr2str(&(pi_ip_entry->ip_addr)));

    }

end:

    if (ret != HAL_RET_OK) {
        // Walk through ip entries
        dllist_for_each(lnode, pi_ep_list) {
            pi_ip_entry = dllist_entry(lnode, ep_ip_entry_t, ep_ip_lentry);
            if (pi_ip_entry->pd) {
                // Free PD IP entry
                g_hal_state_pd->ep_pd_ip_entry_slab()->free(pi_ip_entry->pd);
                // Unlink PD from PI
                pi_ip_entry->pd = NULL;
            }
        }
    }
    return ret;
}


// ----------------------------------------------------------------------------
// Allocate and Initialize EP L3 entries
// ----------------------------------------------------------------------------
hal_ret_t 
ep_pd_alloc_ip_entries(pd_ep_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    // dllist_ctxt_t   *lnode = NULL;
    ep_t            *pi_ep = args->ep;
    // ep_ip_entry_t   *pi_ip_entry = NULL;

    ret = ep_pd_alloc_pd_ip_entries(&(pi_ep->ip_list_head));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-EP:{}: Failed to allocated PD IP entries.",
                __FUNCTION__);
    }

    // Clean up
#if 0
    // Walk through ip entries
    dllist_for_each(lnode, &(pi_ep->ip_list_head)) {
        pi_ip_entry = (ep_ip_entry_t *)((char *)lnode - 
            offsetof(ep_ip_entry_t, ep_ip_lentry));

        pi_ip_entry->pd = (pd_ep_ip_entry_t *)g_hal_state_pd->
                          ep_pd_ip_entry_slab()->alloc();
        if (!pi_ip_entry->pd) {
            ret = HAL_RET_OOM;
            goto end;
        }

        // Link PI to PD
        pi_ip_entry->pd->pi_ep_ip_entry = pi_ip_entry;

    }
    
end:

    if (ret != HAL_RET_OK) {
        lnode = pi_ep->ip_list_head.next;

        // Walk through ip entries
        dllist_for_each(lnode, &(pi_ep->ip_list_head)) {
            pi_ip_entry = (ep_ip_entry_t *)lnode - 
                offsetof(ep_ip_entry_t, ep_ip_lentry);
            if (pi_ip_entry->pd) {
                // Free PD IP entry
                g_hal_state_pd->ep_pd_ip_entry_slab()->free(pi_ip_entry->pd);
                // Unlink PD from PI
                pi_ip_entry->pd = NULL;
            }
        }
    }
#endif
    return ret;
}

// ----------------------------------------------------------------------------
// Deleted PD IP entries
// ----------------------------------------------------------------------------
hal_ret_t
ep_pd_delete_pd_ip_entries(ep_t *pi_ep, dllist_ctxt_t *pi_ep_list)
{
    hal_ret_t       ret = HAL_RET_OK;
    dllist_ctxt_t   *lnode = NULL;
    ep_ip_entry_t   *pi_ip_entry = NULL;

    // Walk through ip entries
    dllist_for_each(lnode, pi_ep_list) {
        pi_ip_entry = dllist_entry(lnode, ep_ip_entry_t, ep_ip_lentry);
        if (pi_ip_entry->pd) {
            // Free PD IP entry
            g_hal_state_pd->ep_pd_ip_entry_slab()->free(pi_ip_entry->pd);
            // Unlink PD from PI
            pi_ip_entry->pd = NULL;
            HAL_TRACE_DEBUG("PD-EP:{}: Freeing PD IP Entry: {}",
                    __FUNCTION__, ipaddr2str(&(pi_ip_entry->ip_addr)));
        } else {
            HAL_ASSERT(0);
        }
    }

    return ret;
}


// ----------------------------------------------------------------------------
// Allocate resources for PD EP
// ----------------------------------------------------------------------------
hal_ret_t 
ep_pd_alloc_res(pd_ep_t *pd_ep)
{
    hal_ret_t            ret = HAL_RET_OK;

#if 0
    indexer::status      rs = indexer::SUCCESS;
    // Allocate lif hwid
    rs = g_hal_state_pd->lif_hwid_idxr()->alloc((uint32_t *)&pd_ep->hw_lif_id);
    if (rs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }
#endif

    return ret;
}

// ----------------------------------------------------------------------------
// De-Allocate resources for PD EP
// ----------------------------------------------------------------------------
hal_ret_t 
ep_pd_dealloc_res(pd_ep_t *pd_ep)
{
    hal_ret_t            ret = HAL_RET_OK;

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
ep_pd_program_hw(pd_ep_t *pd_ep)
{
    hal_ret_t            ret = HAL_RET_OK;


    // Program RW Table
    //ret = ep_pd_pgm_rw_tbl(pd_ep);

    // Program IPSG Table
    ret = ep_pd_pgm_ipsg_tbl(pd_ep);

    return ret;
}

// Deprecated as we are maintaining a hash table for rw table.
// Remove it ...
#if 0
hal_ret_t
ep_pd_pgm_rw_tbl(pd_ep_t *pd_ep)
{
    hal_ret_t            ret = HAL_RET_OK;
    mac_addr_t           mac_sa, mac_da;
    rewrite_actiondata   data;
    DirectMap            *rw_tbl = NULL;
    mac_addr_t           *mac;
    network_t            *nw = NULL;
    ep_t                 *pi_ep = (ep_t *)pd_ep->pi_ep;
    l2seg_t              *l2seg = NULL;

    memset(mac_sa, 0, sizeof(mac_sa));
    memset(mac_da, 0, sizeof(mac_da));
    memset(&data, 0, sizeof(data));
    
    mac = ep_get_mac_addr((ep_t *)pd_ep->pi_ep);

    rw_tbl = g_hal_state_pd->dm_table(P4TBL_ID_REWRITE);
    HAL_ASSERT_RETURN((rw_tbl != NULL), HAL_RET_ERR);

    // mac_sa = ... Get it from l2seg rmac
    // mac_da = ... Get it from EP's NH
    // Start by skipping REWRITE_NOP_ID
    for (int i = 1; i < REWRITE_MAX_ID; i++) {
        switch(i) {
            case REWRITE_REWRITE_ID:
            case REWRITE_IPV4_NAT_SRC_REWRITE_ID:
            case REWRITE_IPV4_NAT_DST_REWRITE_ID:
            case REWRITE_IPV4_NAT_SRC_UDP_REWRITE_ID:
            case REWRITE_IPV4_NAT_SRC_TCP_REWRITE_ID:
            case REWRITE_IPV4_NAT_DST_UDP_REWRITE_ID:
            case REWRITE_IPV4_NAT_DST_TCP_REWRITE_ID:
            case REWRITE_IPV4_TWICE_NAT_REWRITE_ID:
            case REWRITE_IPV4_TWICE_NAT_UDP_REWRITE_ID:
            case REWRITE_IPV4_TWICE_NAT_TCP_REWRITE_ID:
            case REWRITE_IPV6_NAT_SRC_REWRITE_ID:
            case REWRITE_IPV6_NAT_DST_REWRITE_ID:
            case REWRITE_IPV6_NAT_SRC_UDP_REWRITE_ID:
            case REWRITE_IPV6_NAT_SRC_TCP_REWRITE_ID:
            case REWRITE_IPV6_NAT_DST_UDP_REWRITE_ID:
            case REWRITE_IPV6_NAT_DST_TCP_REWRITE_ID:
            case REWRITE_IPV6_TWICE_NAT_REWRITE_ID:
            case REWRITE_IPV6_TWICE_NAT_UDP_REWRITE_ID:
            case REWRITE_IPV6_TWICE_NAT_TCP_REWRITE_ID:
                l2seg = find_l2seg_by_handle(pi_ep->l2seg_handle);
                HAL_ASSERT_RETURN(l2seg != NULL, HAL_RET_L2SEG_NOT_FOUND);
                nw = ep_pd_get_nw(pi_ep, l2seg);
                HAL_ASSERT_RETURN(nw != NULL, HAL_RET_NETWORK_NOT_FOUND);
                if (nw) {
                    memcpy(data.rewrite_action_u.rewrite_rewrite.mac_sa, nw->rmac_addr, 6);
                    memrev(data.rewrite_action_u.rewrite_rewrite.mac_sa, 6);
                } 
                memcpy(data.rewrite_action_u.rewrite_rewrite.mac_da, *mac, 6);
                memrev(data.rewrite_action_u.rewrite_rewrite.mac_da, 6);
                break;
            default:
                HAL_ASSERT(0);
        }
        data.actionid = i;
        ret = rw_tbl->insert(&data, &(pd_ep->rw_tbl_idx[i]));
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("PD-EP::{}: Unable to program for EP: {}:{}", 
                    __FUNCTION__, ep_get_l2segid((ep_t *)pd_ep->pi_ep), 
                    ether_ntoa((struct ether_addr*)*mac));
        } else {
            HAL_TRACE_DEBUG("PD-EP::{}: Programmed for EP: {}:{}", 
                    __FUNCTION__, ep_get_l2segid((ep_t *)pd_ep->pi_ep), 
                    ether_ntoa((struct ether_addr*)*mac));
        }
    }

    return ret;
}
#endif


hal_ret_t
ep_pd_pgm_ipsg_tbl_ip_entries(ep_t *pi_ep, dllist_ctxt_t *pi_ep_list)
{
    hal_ret_t           ret = HAL_RET_OK;
    dllist_ctxt_t       *lnode = NULL;
    ep_ip_entry_t       *pi_ip_entry = NULL;
    pd_ep_ip_entry_t    *pd_ip_entry = NULL;

    // Walk through ip entries
    dllist_for_each(lnode, pi_ep_list) {
        pi_ip_entry = dllist_entry(lnode, ep_ip_entry_t, ep_ip_lentry);
        pd_ip_entry = pi_ip_entry->pd;

        ret = ep_pd_pgm_ipsg_tble_per_ip(pi_ep->pd, pd_ip_entry);
        if (ret != HAL_RET_OK) {
            goto end;
        }
    }

end:
    return ret;
}


// ----------------------------------------------------------------------------
// Program IPSG table for every IP entry
// ----------------------------------------------------------------------------
hal_ret_t 
ep_pd_pgm_ipsg_tbl (pd_ep_t *pd_ep)
{
    hal_ret_t           ret = HAL_RET_OK;
    // dllist_ctxt_t       *lnode = NULL;
    ep_t                *pi_ep = (ep_t *)pd_ep->pi_ep;
    // ep_ip_entry_t       *pi_ip_entry = NULL;
    // pd_ep_ip_entry_t    *pd_ip_entry = NULL;

    ret = ep_pd_pgm_ipsg_tbl_ip_entries(pi_ep, &(pi_ep->ip_list_head));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-EP:{}: Failed to program IPSG entries", 
                __FUNCTION__);
        goto end;
    }

    // TODO: Clean up
#if 0
    // Walk through ip entries
    dllist_for_each(lnode, &(pi_ep->ip_list_head)) {
        pi_ip_entry = (ep_ip_entry_t *)((char *)lnode - 
            offsetof(ep_ip_entry_t, ep_ip_lentry));
        pd_ip_entry = pi_ip_entry->pd;

        ret = ep_pd_pgm_ipsg_tble_per_ip(pd_ep, pd_ip_entry);
        if (ret != HAL_RET_OK) {
            goto end;
        }
    }
#endif

end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram IPSG table for IP entries
// ----------------------------------------------------------------------------
hal_ret_t
ep_pd_depgm_ipsg_tbl_ip_entries(ep_t *pi_ep, dllist_ctxt_t *pi_ep_list)
{
    hal_ret_t           ret = HAL_RET_OK;
    dllist_ctxt_t       *lnode = NULL;
    ep_ip_entry_t       *pi_ip_entry = NULL;
    pd_ep_ip_entry_t    *pd_ip_entry = NULL;

    // Walk through ip entries
    dllist_for_each(lnode, pi_ep_list) {
        /*
        pi_ip_entry = (ep_ip_entry_t *)((char *)lnode - 
            offsetof(ep_ip_entry_t, ep_ip_lentry));
            */
        pi_ip_entry = dllist_entry(lnode, ep_ip_entry_t, ep_ip_lentry);
        pd_ip_entry = pi_ip_entry->pd;

        ret = ep_pd_depgm_ipsg_tble_per_ip(pd_ip_entry);
        if (ret != HAL_RET_OK) {
            goto end;
        }
    }

end:
    return ret;
}


// ----------------------------------------------------------------------------
// DeProgram IPSG table for IP entry
// ----------------------------------------------------------------------------
hal_ret_t 
ep_pd_depgm_ipsg_tble_per_ip(pd_ep_ip_entry_t *pd_ip_entry)
{
    hal_ret_t           ret = HAL_RET_OK;
    ipsg_swkey_t        key;
    ipsg_swkey_mask     key_mask;
    ipsg_actiondata     data;
    Tcam                *ipsg_tbl = NULL;
    ep_ip_entry_t       *pi_ep_ip_entry = (ep_ip_entry_t *)pd_ip_entry->pi_ep_ip_entry;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    ipsg_tbl = g_hal_state_pd->tcam_table(P4TBL_ID_IPSG);
    HAL_ASSERT_RETURN((ipsg_tbl != NULL), HAL_RET_ERR);

    ret = ipsg_tbl->remove(pd_ip_entry->ipsg_tbl_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-EP::{}: Unable to deprogram IPSG for: {}",
                __FUNCTION__, ipaddr2str(&(pi_ep_ip_entry->ip_addr)));
        goto end;
    } else {
        HAL_TRACE_DEBUG("PD-EP::{}: DeProgrammed IPSG at: {} ",
                __FUNCTION__, pd_ip_entry->ipsg_tbl_idx);
    }

end:
    return ret;
}


// ----------------------------------------------------------------------------
// Program IPSG table for IP entry
// ----------------------------------------------------------------------------
hal_ret_t 
ep_pd_pgm_ipsg_tble_per_ip(pd_ep_t *pd_ep, pd_ep_ip_entry_t *pd_ip_entry)
{
    hal_ret_t           ret = HAL_RET_OK;
    ipsg_swkey_t        key;
    ipsg_swkey_mask     key_mask;
    ipsg_actiondata     data;
    ep_t                *pi_ep = (ep_t *)pd_ep->pi_ep;
    ep_ip_entry_t       *pi_ip_entry = 
                          (ep_ip_entry_t *)pd_ip_entry->pi_ep_ip_entry;
    l2seg_t             *l2seg = NULL;
    if_t                *pi_if = NULL;
    // uint64_t            mac_int = 0;
    mac_addr_t          *mac = NULL;
    Tcam                *ipsg_tbl = NULL;
    
    
    pi_if = find_if_by_handle(pi_ep->if_handle);
    if (pi_if->if_type == intf::IF_TYPE_TUNNEL)
        return HAL_RET_OK;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    ipsg_tbl = g_hal_state_pd->tcam_table(P4TBL_ID_IPSG);
    HAL_ASSERT_RETURN((ipsg_tbl != NULL), HAL_RET_ERR);

    key.entry_inactive_ipsg = 0;
    l2seg = find_l2seg_by_handle(pi_ep->l2seg_handle);
    HAL_ASSERT_RETURN(l2seg != NULL, HAL_RET_L2SEG_NOT_FOUND);
    key.flow_lkp_metadata_lkp_vrf = ((pd_l2seg_t *)(l2seg->pd))->l2seg_ten_hw_id;
    memcpy(key.flow_lkp_metadata_lkp_src, 
           pi_ip_entry->ip_addr.addr.v6_addr.addr8,
           IP6_ADDR8_LEN);
    if (pi_ip_entry->ip_addr.af == IP_AF_IPV6) {
        memrev(key.flow_lkp_metadata_lkp_src, sizeof(key.flow_lkp_metadata_lkp_src));
    }
    key.flow_lkp_metadata_lkp_type = 
        (pi_ip_entry->ip_addr.af == IP_AF_IPV4) ? FLOW_KEY_LOOKUP_TYPE_IPV4 :
        FLOW_KEY_LOOKUP_TYPE_IPV6;

    key_mask.entry_inactive_ipsg_mask = 0x1;
    key_mask.flow_lkp_metadata_lkp_vrf_mask = 
        ~(key_mask.flow_lkp_metadata_lkp_vrf_mask & 0);
    key_mask.flow_lkp_metadata_lkp_type_mask = 
        ~(key_mask.flow_lkp_metadata_lkp_type_mask & 0);
    memset(key_mask.flow_lkp_metadata_lkp_src_mask, ~0, 
            sizeof(key_mask.flow_lkp_metadata_lkp_src_mask));

    HAL_ASSERT_RETURN(l2seg != NULL, HAL_RET_IF_NOT_FOUND);
    data.actionid = IPSG_IPSG_HIT_ID;
    // data.ipsg_action_u.ipsg_ipsg_hit.src_lport = if_get_lport_id(pi_if);
    data.ipsg_action_u.ipsg_ipsg_hit.src_lif = if_get_hw_lif_id(pi_if);
    mac = ep_get_mac_addr(pi_ep);
    // mac_int = MAC_TO_UINT64(*mac); // TODO: Cleanup May be you dont need this ?
    memcpy(data.ipsg_action_u.ipsg_ipsg_hit.mac, *mac, 6);
    memrev(data.ipsg_action_u.ipsg_ipsg_hit.mac, 6);
    if_l2seg_get_encap(pi_if, l2seg, &data.ipsg_action_u.ipsg_ipsg_hit.vlan_valid,
            &data.ipsg_action_u.ipsg_ipsg_hit.vlan_id);

     ret = ipsg_tbl->insert(&key, &key_mask, &data,
                                         &(pd_ip_entry->ipsg_tbl_idx));
     if (ret != HAL_RET_OK) {
         HAL_TRACE_ERR("PD-EP::{}: Unable to program IPSG for: {}",
                 __FUNCTION__, ipaddr2str(&(pi_ip_entry->ip_addr)));
         goto end;
     } else {
         HAL_TRACE_DEBUG("PD-EP::{}: Programmed IPSG for: at: {} "
                         "(vrf:{}, ip:{}) => "
                         "act_id:{}, lif:{}, vlan_v:{}, vlan_vid:{}, mac:{}",
                         __FUNCTION__,
                         pd_ip_entry->ipsg_tbl_idx,
                         key.flow_lkp_metadata_lkp_vrf,
                         ipaddr2str(&(pi_ip_entry->ip_addr)),
                         data.actionid, 
                         data.ipsg_action_u.ipsg_ipsg_hit.src_lif,
                         data.ipsg_action_u.ipsg_ipsg_hit.vlan_valid,
                         data.ipsg_action_u.ipsg_ipsg_hit.vlan_id,
                         macaddr2str(*mac));
     }

end:
    return ret;
}



// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
ep_link_pi_pd(pd_ep_t *pd_ep, ep_t *pi_ep)
{
    pd_ep->pi_ep = pi_ep;
    ep_set_pd_ep(pi_ep, pd_ep);
}

// ----------------------------------------------------------------------------
// De-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
ep_delink_pi_pd(pd_ep_t *pd_ep, ep_t *pi_ep)
{
    pd_ep->pi_ep = NULL;
    ep_set_pd_ep(pi_ep, NULL);
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_make_clone(ep_t *ten, ep_t *clone)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_ep_t         *pd_ep_clone = NULL;

    pd_ep_clone = ep_pd_alloc_init();
    if (pd_ep_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_ep_clone, ten->pd, sizeof(pd_ep_t));

    ep_link_pi_pd(pd_ep_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_mem_free(pd_ep_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_ep_t    *ep_pd;

    ep_pd = (pd_ep_t *)args->ep->pd;
    ep_pd_mem_free(ep_pd);

    return ret;
}

// ----------------------------------------------------------------------------
// Gets hw lif id from the interface
// ----------------------------------------------------------------------------
uint32_t
ep_pd_get_hw_lif_id(ep_t *pi_ep) 
{
    if_t            *pi_if;
    lif_t           *pi_lif;
    pd_lif_t        *pd_lif;
    pd_uplinkif_t   *pd_upif;
    pd_uplinkpc_t   *pd_uppc;
    intf::IfType    if_type;

    pi_if = ep_find_if_by_handle(pi_ep);
    HAL_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            pi_lif = if_get_lif(pi_if);
            HAL_ASSERT(pi_lif != NULL);

            pd_lif = (pd_lif_t *)lif_get_pd_lif(pi_lif);
            HAL_ASSERT(pi_lif != NULL);

            return pd_lif->hw_lif_id;
            break;
        case intf::IF_TYPE_UPLINK:
            pd_upif = (pd_uplinkif_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_upif != NULL);

            return pd_upif->hw_lif_id;
            break;
        case intf::IF_TYPE_UPLINK_PC:
            pd_uppc = (pd_uplinkpc_t *)if_get_pd_if(pi_if);
            HAL_ASSERT(pd_uppc != NULL);

            return pd_uppc->hw_lif_id;
            break;
        case intf::IF_TYPE_TUNNEL:
            return (if_get_hw_lif_id(pi_if));
            break;
        default:
            HAL_ASSERT(0);
    }

    return 0;
}

// ----------------------------------------------------------------------------
// Gets interface type
// ----------------------------------------------------------------------------
intf::IfType
ep_pd_get_if_type(ep_t *pi_ep) 
{
    if_t            *pi_if;

    pi_if = ep_find_if_by_handle(pi_ep);
    HAL_ASSERT(pi_if != NULL);

    return intf_get_if_type(pi_if);
}

uint32_t 
ep_pd_get_rw_tbl_idx_from_pi_ep(ep_t *pi_ep, rewrite_actions_en rw_act)
{
    pd_ep_t *pd_ep = NULL;

    pd_ep = (pd_ep_t *)ep_get_pd_ep(pi_ep);

    return ep_pd_get_rw_tbl_idx(pd_ep, rw_act);
}

// ----------------------------------------------------------------------------
// Input: Dest PD EP and Rewrite Action
// Retuns: RW idx in flow table
// ----------------------------------------------------------------------------
uint32_t
ep_pd_get_rw_tbl_idx(pd_ep_t *pd_ep, rewrite_actions_en rw_act)
{
    HAL_ASSERT(rw_act < REWRITE_MAX_ID);

    return pd_ep->rw_tbl_idx[rw_act];
}

uint32_t
ep_pd_get_tnnl_rw_tbl_idx_from_pi_ep(ep_t *pi_ep, 
                                     tunnel_rewrite_actions_en tnnl_rw_act) 
{
    pd_ep_t *pd_ep = NULL;

    pd_ep = (pd_ep_t *)ep_get_pd_ep(pi_ep);

    return ep_pd_get_tnnl_rw_tbl_idx(pd_ep, tnnl_rw_act);
}

// ----------------------------------------------------------------------------
// Input: Dest EP and Tunnel Rewrite Action
// Retuns: Tunnel RW idx in flow table
// ----------------------------------------------------------------------------
uint32_t
ep_pd_get_tnnl_rw_tbl_idx(pd_ep_t *pd_ep, 
                          tunnel_rewrite_actions_en tnnl_rw_act) {
    HAL_ASSERT(tnnl_rw_act < TUNNEL_REWRITE_MAX_ID);

    if (tnnl_rw_act == TUNNEL_REWRITE_ENCAP_VLAN_ID) {
        return g_hal_state_pd->tnnl_rwr_tbl_encap_vlan_idx();
    } else if (tnnl_rw_act == TUNNEL_REWRITE_ENCAP_VXLAN_ID) {
        if_t *tunnel_if = ep_find_if_by_handle((ep_t *)pd_ep->pi_ep);
        return (pd_tunnelif_get_rw_idx((pd_tunnelif_t *)tunnel_if->pd_if));
    }

    return 0;
}

network_t *
ep_pd_get_nw(ep_t *pi_ep, l2seg_t *l2seg) 
{
    network_t       *nw = NULL;
    dllist_ctxt_t   *lnode = NULL, *nw_lnode = NULL;
    ep_ip_entry_t   *pi_ip_entry = NULL;

    // Get the first IP
    if (dllist_empty(&pi_ep->ip_list_head)) {
        goto end;
    } else {
        lnode = pi_ep->ip_list_head.next;
        pi_ip_entry = (ep_ip_entry_t *)((char *)lnode - offsetof(ep_ip_entry_t, ep_ip_lentry));

        dllist_for_each(nw_lnode, &(l2seg->nw_list_head)) {
            nw = (network_t *)((char *)nw_lnode -
                    offsetof(network_t, l2seg_nw_lentry));
            // Check if ip is in prefix
            if (hal::ip_addr_in_ip_pfx(&pi_ip_entry->ip_addr, &nw->nw_key.ip_pfx)) {
                return nw;
            }
        }
    }

end:
    return nw;
}

}    // namespace pd
}    // namespace hal
