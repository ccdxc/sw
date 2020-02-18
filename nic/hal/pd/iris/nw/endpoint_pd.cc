#include <netinet/ether.h>
#include "nic/sdk/include/sdk/lock.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/endpoint_api.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/hal/pd/iris/nw/endpoint_pd.hpp"
#include "nic/hal/pd/iris/lif/lif_pd.hpp"
#include "nic/hal/pd/iris/nw/uplinkif_pd.hpp"
#include "nic/hal/pd/iris/nw/uplinkpc_pd.hpp"
#include "nic/hal/plugins/cfg/nw/l2segment.hpp"
#include "nic/hal/plugins/cfg/nw/nw.hpp"
#include "nic/hal/pd/iris/nw/l2seg_pd.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/hal/pd/iris/nw/if_pd.hpp"
#include "nic/hal/pd/iris/p4pd_defaults.hpp"

using namespace hal;

namespace hal {
namespace pd {

hal_ret_t ep_pd_depgm_registered_mac(pd_ep_t *pd_ep);

#define reg_mac data.action_u.registered_macs_registered_macs

// ----------------------------------------------------------------------------
// EP Create
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_create (pd_func_args_t *pd_func_args)
{
    hal_ret_t            ret = HAL_RET_OK;
    pd_ep_create_args_t *args = pd_func_args->pd_ep_create;
    pd_ep_t             *pd_ep;
    mac_addr_t           *mac;

    mac = ep_get_mac_addr(args->ep);

    HAL_TRACE_DEBUG("creating pd state for ep: {}",
                    ep_l2_key_to_str(args->ep));

    // Create ep PD
    pd_ep = ep_pd_alloc_init();
    if (pd_ep == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    ep_link_pi_pd(pd_ep, args->ep);

    // Create EP L3 entry PDs
    ret = ep_pd_alloc_ip_entries(args->ep);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to alloc. ip entries: ep:{}",
                      ep_l2_key_to_str(args->ep));
        goto end;
    }

    // Allocate Resources
    ret = ep_pd_alloc_res(pd_ep);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("Unable to alloc. resources for EP: {}:{}",
                      ep_get_l2segid(args->ep),
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
pd_ep_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_ep_update_args_t     *pd_ep_upd_args = pd_func_args->pd_ep_update;
    pd_ep_t                 *ep_pd = (pd_ep_t *)pd_ep_upd_args->ep->pd;;
    pd_ep_if_update_args_t  if_upd_args = {0};

    HAL_TRACE_DEBUG("updating pd state for ep:{}",
                    ep_l2_key_to_str(pd_ep_upd_args->ep));

    if (pd_ep_upd_args->iplist_change) {
        ret = pd_ep_upd_iplist_change(pd_ep_upd_args);
    }

    if (pd_ep_upd_args->if_change) {
        if_upd_args.if_change = pd_ep_upd_args->if_change;
        if_upd_args.new_if = pd_ep_upd_args->new_if;
        ret = pd_ep_pgm_registered_mac(ep_pd, &if_upd_args, NULL, TABLE_OPER_UPDATE);
    }

    return ret;
}

// ----------------------------------------------------------------------------
// EP Update for IF update
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_if_update (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_ep_if_update_args_t  *if_upd_args = pd_func_args->pd_ep_if_update;
    pd_ep_t                 *ep_pd = (pd_ep_t *)if_upd_args->ep->pd;;
    ep_t                    *ep = (ep_t *)ep_pd->pi_ep;
    nwsec_profile_t         *nwsec_profile;

    nwsec_profile = ep_get_pi_nwsec(ep);
    if (if_upd_args->lif_change) {
        if(nwsec_profile &&
           nwsec_profile->ipsg_en) {
            ret = ep_pd_pgm_ipsg_tbl(ep_pd,
                                     false,
                                     if_upd_args,
                                     TABLE_OPER_UPDATE);
        }
        if (if_upd_args->new_lif != NULL) {
            // Update reg_mac entry
            if (ep_pd->reg_mac_tbl_idx == INVALID_INDEXER_INDEX) { 
                ret = pd_ep_pgm_registered_mac(ep_pd,
                                               if_upd_args, NULL,
                                               TABLE_OPER_INSERT);
            } else {
                ret = pd_ep_pgm_registered_mac(ep_pd,
                                               if_upd_args, NULL,
                                               TABLE_OPER_UPDATE);
            }
        } else {
            ret = ep_pd_depgm_registered_mac(ep_pd);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("PD-EP: Failed to depgm registered_mac, ret:{}", ret);
            }
        }
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD Endpoint Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_ep_delete (pd_func_args_t *pd_func_args)
{
    hal_ret_t    ret = HAL_RET_OK;
    pd_ep_delete_args_t *args = pd_func_args->pd_ep_delete;
    pd_ep_t      *ep_pd;

    SDK_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->ep != NULL), HAL_RET_INVALID_ARG);
    SDK_ASSERT_RETURN((args->ep->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_VERBOSE("deleting pd state for ep {}",
                    ep_l2_key_to_str(args->ep));

    ep_pd = (pd_ep_t *)args->ep->pd;

    // deprogram hw
    ret = ep_pd_depgm_ipsg_tbl_ip_entries(args->ep,
                                          &(args->ep->ip_list_head));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-EP: Failed to depgm IPSG, ret:{}", ret);
        goto end;
    }

    if (ep_pd->reg_mac_tbl_idx != INVALID_INDEXER_INDEX) {
        ret = ep_pd_depgm_registered_mac(ep_pd);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("PD-EP: Failed to depgm registered_mac, ret:{}", ret);
            goto end;
        }
    }

    // free up the resource and memory
    ret = ep_pd_cleanup(ep_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("failed pd ep delete");
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// EP Get
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_get (pd_func_args_t *pd_func_args)
{
    hal_ret_t               ret = HAL_RET_OK;
    pd_ep_get_args_t *args = pd_func_args->pd_ep_get;
    ep_t                    *ep = args->ep;
    pd_ep_t                 *ep_pd = (pd_ep_t *)ep->pd;
    EndpointGetResponse     *rsp = args->rsp;
    dllist_ctxt_t           *lnode = NULL;
    ep_ip_entry_t           *pi_ip_entry = NULL;
    int                     i;

    auto ep_info = rsp->mutable_status()->mutable_epd_status();

    ep_info->set_reg_mac_tbl_idx(ep_pd->reg_mac_tbl_idx);

    for (i = 0; i < REWRITE_MAX_ID; i++) {
        if (ep_pd->rw_tbl_idx[i]) {
            ep_info->add_rw_tbl_idx(ep_pd->rw_tbl_idx[i]);
        }
    }

    lnode = ep->ip_list_head.next;
    i = 0;
    dllist_for_each(lnode, &(ep->ip_list_head)) {
        pi_ip_entry = (ep_ip_entry_t *)((char *)lnode -
                offsetof(ep_ip_entry_t, ep_ip_lentry));
        EndpointIpAddress *endpoint_ip_addr = rsp->mutable_status()->mutable_ip_address(i);
        endpoint_ip_addr->set_ipsg_tbl_idx(pi_ip_entry->pd->ipsg_tbl_idx);
        i ++;
    }

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
        HAL_TRACE_ERR("unable to dealloc res for ep: {}",
                      (ep_l2_key_to_str((ep_t *)(ep_pd->pi_ep))));
        goto end;
    }

    // Free up IPs PD State
    ret = ep_pd_delete_pd_ip_entries((ep_t *)ep_pd->pi_ep,
                                     &((ep_t *)(ep_pd->pi_ep))->ip_list_head);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(" failed to free pd ip entries. ret:{}", ret);
        goto end;
    }


    // Delinking PI<->PD
    ep_delink_pi_pd(ep_pd, (ep_t *)ep_pd->pi_ep);

    // Freeing PD
    ep_pd_free(ep_pd);
end:
    return ret;
}

hal_ret_t
ep_pd_restore_data (pd_ep_restore_args_t *args)
{
    ep_t *ep        = args->ep;
    pd_ep_t *ep_pd  = ep->pd;

    auto ep_status = args->ep_status->epd_status();

    ep_pd->reg_mac_tbl_idx = ep_status.reg_mac_tbl_idx();

    for (int i = 0; i < ep_status.rw_tbl_idx_size(); i++) {
        ep_pd->rw_tbl_idx[i] = ep_status.rw_tbl_idx(i);
    }

    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// EP Restore
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_restore (pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;;
    pd_ep_restore_args_t *args = pd_func_args->pd_ep_restore;
    pd_ep_t             *pd_ep;
    mac_addr_t          *mac;

    mac = ep_get_mac_addr(args->ep);

    HAL_TRACE_DEBUG("creating pd state for ep: {}",
                    ep_l2_key_to_str(args->ep));

    // Create ep PD
    pd_ep = ep_pd_alloc_init();
    if (pd_ep == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    ep_link_pi_pd(pd_ep, args->ep);

    // Restore data
    ret = ep_pd_restore_data(args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Unable to restore data for EP: {}:{}",
                      ep_get_l2segid(args->ep),
                ether_ntoa((struct ether_addr*)*mac));
        goto end;
    }

    // Create EP L3 entry PDs
    ret = ep_pd_alloc_ip_entries(args->ep);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("unable to alloc. ip entries: ep:{}",
                      ep_l2_key_to_str(args->ep));
        goto end;
    }

    // Program HW
    ret = ep_pd_program_hw(pd_ep, true);

end:
    if (ret != HAL_RET_OK) {
        // unlink_pi_pd(pd_ep, args->ep);
        // ep_pd_free(pd_ep);
        ep_pd_cleanup(pd_ep);
    }

    return ret;
}


// ----------------------------------------------------------------------------
// EP Update: Handling ip list change
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_upd_iplist_change (pd_ep_update_args_t *pd_ep_upd_args)
{
    hal_ret_t       ret = HAL_RET_OK;
    ep_t            *ep = pd_ep_upd_args->ep;
    nwsec_profile_t *nwsec_profile;

    HAL_TRACE_DEBUG("ip-list change: ");

    // Allocated PD State for new IP entries
    ret = ep_pd_alloc_pd_ip_entries(pd_ep_upd_args->add_iplist);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(" failed to alloced pd ip entries "
                      "for new ip. ret:{}", ret);
        goto end;
    }

    // Program IPSG entries for new IPs
    nwsec_profile = ep_get_pi_nwsec(ep);
    if (nwsec_profile && nwsec_profile->ipsg_en) {
        ret = ep_pd_pgm_ipsg_tbl_ip_entries(pd_ep_upd_args->ep,
                                            pd_ep_upd_args->add_iplist,
                                            false,
                                            NULL,
                                            TABLE_OPER_INSERT);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(" failed to pgm IPSG"
                          "for new ip. ret:{}", ret);
            goto end;
        }

        // Deprogram IPSG entries for deleted IPs
        ret = ep_pd_depgm_ipsg_tbl_ip_entries(pd_ep_upd_args->ep,
                                              pd_ep_upd_args->del_iplist);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR(" failed to depgm IPSG"
                          "for deleted ip. ret:{}", ret);
            goto end;
        }
    }

    // free up delete IPs PD state
    ret = ep_pd_delete_pd_ip_entries(pd_ep_upd_args->ep,
                                     pd_ep_upd_args->del_iplist);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR(" failed to free pd ip entries. ret:{}", ret);
        goto end;
    }


end:
    return ret;
}

// ----------------------------------------------------------------------------
// Allocate PD IP entries of a list of PI entries
// ----------------------------------------------------------------------------
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
        pi_ip_entry->pd->ipsg_tbl_idx = INVALID_INDEXER_INDEX;

        // Link PI to PD
        pi_ip_entry->pd->pi_ep_ip_entry = pi_ip_entry;

        HAL_TRACE_DEBUG("Allocating PD IP Entry: {}",
                        ipaddr2str(&(pi_ip_entry->ip_addr)));

    }

end:

    if (ret != HAL_RET_OK) {
        // Walk through ip entries
        dllist_for_each(lnode, pi_ep_list) {
            pi_ip_entry = dllist_entry(lnode, ep_ip_entry_t, ep_ip_lentry);
            if (pi_ip_entry->pd) {
                // Free PD IP entry
                hal::pd::delay_delete_to_slab(HAL_SLAB_EP_IP_ENTRY_PD,
                                              pi_ip_entry->pd);
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
ep_pd_alloc_ip_entries(ep_t *pi_ep)
{
    hal_ret_t       ret = HAL_RET_OK;

    ret = ep_pd_alloc_pd_ip_entries(&(pi_ep->ip_list_head));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to allocated PD IP entries.");
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
            hal::pd::delay_delete_to_slab(HAL_SLAB_EP_IP_ENTRY_PD,
                                          pi_ip_entry->pd);
            // Unlink PD from PI
            pi_ip_entry->pd = NULL;
            HAL_TRACE_DEBUG("Freeing PD IP Entry: {}",
                            ipaddr2str(&(pi_ip_entry->ip_addr)));
        } else {
            SDK_ASSERT(0);
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
ep_pd_program_hw(pd_ep_t *pd_ep, bool is_upgrade)
{
    hal_ret_t           ret = HAL_RET_OK;
    ep_t                *pi_ep = (ep_t *)pd_ep->pi_ep;
    // l2seg_t             *l2seg = NULL;
    nwsec_profile_t     *nwsec_profile;

    // Program IPSG Table
    nwsec_profile = ep_get_pi_nwsec(pi_ep);
    if (nwsec_profile && nwsec_profile->ipsg_en) {
        ret = ep_pd_pgm_ipsg_tbl(pd_ep,
                                 is_upgrade,
                                 NULL,
                                 TABLE_OPER_INSERT);
    }

    ret = pd_ep_pgm_registered_mac(pd_ep, NULL, NULL, TABLE_OPER_INSERT);

    return ret;
}

hal_ret_t
ep_pd_pgm_ipsg_tbl_ip_entries(ep_t *pi_ep, dllist_ctxt_t *pi_ep_list,
                              bool is_upgrade,
                              pd_ep_if_update_args_t *if_args,
                              table_oper_t oper)
{
    hal_ret_t           ret = HAL_RET_OK;
    dllist_ctxt_t       *lnode = NULL;
    ep_ip_entry_t       *pi_ip_entry = NULL;
    pd_ep_ip_entry_t    *pd_ip_entry = NULL;

    // Walk through ip entries
    dllist_for_each(lnode, pi_ep_list) {
        pi_ip_entry = dllist_entry(lnode, ep_ip_entry_t, ep_ip_lentry);
        pd_ip_entry = pi_ip_entry->pd;

        ret = ep_pd_pgm_ipsg_tble_per_ip(pi_ep->pd, pd_ip_entry,
                                         is_upgrade,
                                         if_args,
                                         oper);
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
ep_pd_pgm_ipsg_tbl (pd_ep_t *pd_ep, bool is_upgrade,
                    pd_ep_if_update_args_t *if_args,
                    table_oper_t oper)
{
    hal_ret_t           ret = HAL_RET_OK;
    // dllist_ctxt_t       *lnode = NULL;
    ep_t                *pi_ep = (ep_t *)pd_ep->pi_ep;
    // ep_ip_entry_t       *pi_ip_entry = NULL;
    // pd_ep_ip_entry_t    *pd_ip_entry = NULL;

    ret = ep_pd_pgm_ipsg_tbl_ip_entries(pi_ep, &(pi_ep->ip_list_head),
                                        is_upgrade,
                                        if_args,
                                        oper);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to program IPSG entries");
        goto end;
    }

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
    sdk_ret_t           sdk_ret;
    ipsg_swkey_t        key;
    ipsg_swkey_mask     key_mask;
    ipsg_actiondata_t     data;
    tcam                *ipsg_tbl = NULL;
    ep_ip_entry_t       *pi_ep_ip_entry = (ep_ip_entry_t *)pd_ip_entry->pi_ep_ip_entry;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    ipsg_tbl = g_hal_state_pd->tcam_table(P4TBL_ID_IPSG);
    SDK_ASSERT_RETURN((ipsg_tbl != NULL), HAL_RET_ERR);
    HAL_TRACE_DEBUG("Delete the ipsg_tbl at index {}", pd_ip_entry->ipsg_tbl_idx);

    if (pd_ip_entry->ipsg_tbl_idx != INVALID_INDEXER_INDEX) {
        sdk_ret = ipsg_tbl->remove(pd_ip_entry->ipsg_tbl_idx);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to deprogram IPSG for: {}",
                          ipaddr2str(&(pi_ep_ip_entry->ip_addr)));
            goto end;
        } else {
            HAL_TRACE_DEBUG("DeProgrammed IPSG at: {} ",
                            pd_ip_entry->ipsg_tbl_idx);
        }
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Deprogram IPSG table for every IP entry
// ----------------------------------------------------------------------------
hal_ret_t
ep_pd_depgm_ipsg_tbl (pd_ep_t *pd_ep)
{
    hal_ret_t           ret = HAL_RET_OK;
    ep_t                *pi_ep = (ep_t *)pd_ep->pi_ep;

    ret = ep_pd_depgm_ipsg_tbl_ip_entries(pi_ep, &(pi_ep->ip_list_head));
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to deprogram IPSG entries, ret:{}", ret);
        goto end;
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Program IPSG table for IP entry
// ----------------------------------------------------------------------------
hal_ret_t
ep_pd_pgm_ipsg_tble_per_ip(pd_ep_t *pd_ep,
                           pd_ep_ip_entry_t *pd_ip_entry,
                           bool is_upgrade,
                           pd_ep_if_update_args_t *if_args,
                           table_oper_t oper)
{
    hal_ret_t           ret = HAL_RET_OK;
    sdk_ret_t           sdk_ret;
    ipsg_swkey_t        key;
    ipsg_swkey_mask     key_mask;
    ipsg_actiondata_t     data;
    ep_t                *pi_ep = (ep_t *)pd_ep->pi_ep;
    ep_ip_entry_t       *pi_ip_entry =
                          (ep_ip_entry_t *)pd_ip_entry->pi_ep_ip_entry;
    l2seg_t             *l2seg = NULL;
    if_t                *pi_if = NULL;
    // uint64_t            mac_int = 0;
    mac_addr_t          *mac = NULL;
    tcam                *ipsg_tbl = NULL;


    pi_if = find_if_by_handle(pi_ep->if_handle);
    if (pi_if->if_type == intf::IF_TYPE_TUNNEL)
        return HAL_RET_OK;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    ipsg_tbl = g_hal_state_pd->tcam_table(P4TBL_ID_IPSG);
    SDK_ASSERT_RETURN((ipsg_tbl != NULL), HAL_RET_ERR);

    key.entry_inactive_ipsg = 0;
    l2seg = l2seg_lookup_by_handle(pi_ep->l2seg_handle);
    SDK_ASSERT_RETURN(l2seg != NULL, HAL_RET_L2SEG_NOT_FOUND);
    key.flow_lkp_metadata_lkp_vrf = ((pd_l2seg_t *)(l2seg->pd))->l2seg_fl_lkup_id;
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

    SDK_ASSERT_RETURN(l2seg != NULL, HAL_RET_IF_NOT_FOUND);
    data.action_id = IPSG_IPSG_HIT_ID;
    // data.action_u.ipsg_ipsg_hit.src_lport = if_get_lport_id(pi_if);
    if (if_args && if_args->lif_change) {
        if (if_args->new_lif) {
            uint32_t src_lif = 0;
            ret = pd_lif_get_hw_lif_id(if_args->new_lif, &src_lif);
            data.action_u.ipsg_ipsg_hit.src_lif = src_lif;
            HAL_TRACE_DEBUG("Lif change on Enic. IPSG src_lif: {}",
                            data.action_u.ipsg_ipsg_hit.src_lif);
        } else {
            HAL_TRACE_DEBUG("Lif removal on Enic. IPSG src_lif: {}",
                            data.action_u.ipsg_ipsg_hit.src_lif);
        }
    } else {
        data.action_u.ipsg_ipsg_hit.src_lif = if_get_hw_lif_id(pi_if);
    }
    mac = ep_get_mac_addr(pi_ep);
    // mac_int = MAC_TO_UINT64(*mac); // TODO: Cleanup May be you dont need this ?
    memcpy(data.action_u.ipsg_ipsg_hit.mac, *mac, 6);
    memrev(data.action_u.ipsg_ipsg_hit.mac, 6);
    if_l2seg_get_encap(pi_if, l2seg, &data.action_u.ipsg_ipsg_hit.vlan_valid,
            &data.action_u.ipsg_ipsg_hit.vlan_id);

    if (oper == TABLE_OPER_INSERT) {
        if (is_upgrade) {
            sdk_ret = ipsg_tbl->insert_withid(&key, &key_mask, &data,
                                              pd_ip_entry->ipsg_tbl_idx);
        } else {
            sdk_ret = ipsg_tbl->insert(&key, &key_mask, &data,
                                       &(pd_ip_entry->ipsg_tbl_idx));
        }
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to program IPSG for: {}",
                          ipaddr2str(&(pi_ip_entry->ip_addr)));
            goto end;
        } else {
            HAL_TRACE_DEBUG("Programmed IPSG for: at: {} "
                            "(vrf:{}, ip:{}) => "
                            "act_id:{}, lif:{}, vlan_v:{}, vlan_vid:{}, mac:{}",
                            pd_ip_entry->ipsg_tbl_idx,
                            key.flow_lkp_metadata_lkp_vrf,
                            ipaddr2str(&(pi_ip_entry->ip_addr)),
                            data.action_id,
                            data.action_u.ipsg_ipsg_hit.src_lif,
                            data.action_u.ipsg_ipsg_hit.vlan_valid,
                            data.action_u.ipsg_ipsg_hit.vlan_id,
                            macaddr2str(*mac));
        }
    } else {
        sdk_ret = ipsg_tbl->update(pd_ip_entry->ipsg_tbl_idx, &data);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Unable to update IPSG at: {}", pd_ip_entry->ipsg_tbl_idx);
            goto end;
        } else {
            HAL_TRACE_ERR("Updated IPSG at: {}", pd_ip_entry->ipsg_tbl_idx);
        }
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// EP Pgm/Depgm IPSG entries
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_ipsg_change (pd_func_args_t *pd_func_args)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd_ep_ipsg_change_args_t    *args = pd_func_args->pd_ep_ipsg_change;
    ep_t                        *pi_ep = args->ep;
    pd_ep_t                     *pd_ep = (pd_ep_t *)pi_ep->pd;

    if (args->pgm) {
        ret = ep_pd_pgm_ipsg_tbl(pd_ep, false, NULL, TABLE_OPER_INSERT);
    } else {
        ret = ep_pd_depgm_ipsg_tbl(pd_ep);
    }

    return ret;
}

hal_ret_t
pd_ep_reg_mac_info (l2seg_t *ep_l2seg, l2seg_t *cl_l2seg, l2seg_t *hp_l2seg, 
                    if_t *ep_if, if_t *uplink_if, lif_t *enic_lif,
                    registered_macs_swkey_t &key,
                    registered_macs_otcam_swkey_mask_t &key_mask,
                    registered_macs_actiondata_t &data)
{
    hal_ret_t ret = HAL_RET_OK;
    p4_replication_data_t repl_data = {};
    pd_l2seg_t *cl_l2seg_pd, *hp_l2seg_pd;

    cl_l2seg_pd = cl_l2seg ? (pd_l2seg_t *)cl_l2seg->pd : NULL;
    hp_l2seg_pd = hp_l2seg ? (pd_l2seg_t *)hp_l2seg->pd : NULL;
    if (cl_l2seg && hp_l2seg) {
        key.flow_lkp_metadata_lkp_reg_mac_vrf = hp_l2seg_pd->l2seg_fl_lkup_id;
        reg_mac.multicast_en = 0;
        reg_mac.dst_if_label = pd_uplinkif_if_label(uplink_if);
        reg_mac.flow_learn = (ep_l2seg == hp_l2seg) ? 1 : 0;
        if (ep_l2seg == cl_l2seg) {
            // Pkts from Uplink destined to mgmt EPs, will have profile as 0, which will have all knobs off.
            reg_mac.l4_profile_en = 1;
            reg_mac.l4_profile_idx = L4_PROF_DEFAULT_ENTRY;
        }
    } else if (hp_l2seg) {
        key.flow_lkp_metadata_lkp_reg_mac_vrf = hp_l2seg_pd->l2seg_fl_lkup_id;
        reg_mac.multicast_en = 0;
        reg_mac.dst_if_label = pd_uplinkif_if_label(uplink_if);
        reg_mac.flow_learn = 1;
    } else {
        key.flow_lkp_metadata_lkp_reg_mac_vrf = cl_l2seg_pd->l2seg_fl_lkup_id;
        reg_mac.multicast_en = 0;
        reg_mac.dst_if_label = pd_uplinkif_if_label(uplink_if);
        reg_mac.flow_learn = 0;
    }

    ret = if_l2seg_get_multicast_rewrite_data(ep_if, ep_l2seg, 
                                              enic_lif, &repl_data);
    if (repl_data.tunnel_rewrite_index != 0) {
        reg_mac.tunnel_rewrite_en = 1;
        reg_mac.tunnel_rewrite_index = repl_data.tunnel_rewrite_index;
    }

    return ret;
}

hal_ret_t
pd_ep_pgm_registered_mac(pd_ep_t *pd_ep, 
                         pd_ep_if_update_args_t *if_args,
                         l2seg_t *attached_l2seg,
                         table_oper_t oper)
{
    hal_ret_t                           ret = HAL_RET_OK;
    sdk_ret_t                           sdk_ret;
    registered_macs_swkey_t             key;
    registered_macs_otcam_swkey_mask_t  key_mask, *key_mask_p = NULL; 
    registered_macs_actiondata_t        data;
    sdk_hash                            *reg_mac_tbl = NULL;
    ep_t                                *pi_ep = (ep_t *)pd_ep->pi_ep;
    l2seg_t                             *l2seg = NULL;
    mac_addr_t                          *mac = NULL;
    if_t                                *pi_if = NULL, *uplink_if = NULL;
    uint32_t                            hash_idx = INVALID_INDEXER_INDEX;
    bool                                direct_to_otcam = false;
    lif_t                               *lif = NULL;
    l2seg_t                             *cl_l2seg = NULL, *hp_l2seg = NULL;

    memset(&key, 0, sizeof(key));
    memset(&key_mask, 0, sizeof(key_mask));
    memset(&data, 0, sizeof(data));

    reg_mac_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_REGISTERED_MACS);
    SDK_ASSERT_RETURN((reg_mac_tbl != NULL), HAL_RET_ERR);

    l2seg = l2seg_lookup_by_handle(pi_ep->l2seg_handle);
    if (if_args && if_args->if_change) {
        pi_if = if_args->new_if;
    } else {
        pi_if = find_if_by_handle(pi_ep->if_handle);
    }

    if (hal::intf_get_if_type(pi_if) == intf::IF_TYPE_ENIC) {
        if (if_args && if_args->lif_change) {
            lif = if_args->new_lif;
            uplink_if = lif_get_pinned_uplink(if_args->new_lif);
        } else {
            lif = if_get_lif(pi_if);
            if_enicif_get_pinned_if(pi_if, &uplink_if);
            if (!uplink_if) {
                if (lif && (lif->type == types::LIF_TYPE_MNIC_INTERNAL_MANAGEMENT ||
                            lif->type == types::LIF_TYPE_HOST_MANAGEMENT)) {
                    // No uplink for internal mgmt lif. 
                } else {
                    HAL_TRACE_DEBUG("No EP => Lif mapping. Skipping reg_mac programming");
                    goto end;
                }
            }
        }
        if (l2seg_is_cust(l2seg)) {
            hp_l2seg = l2seg;
            cl_l2seg = l2seg_pd_get_shared_mgmt_l2seg(l2seg, uplink_if);
        } else {
            cl_l2seg = l2seg;
            if (attached_l2seg) {
                hp_l2seg = attached_l2seg;
            } else {
                hp_l2seg = l2seg_pd_get_shared_mgmt_l2seg(l2seg, uplink_if);
            }
        }
        pd_ep_reg_mac_info(l2seg, cl_l2seg, hp_l2seg, pi_if, uplink_if, lif, 
                           key, key_mask, data);
    } else {
        if (l2seg_is_cust(l2seg)) {
            hp_l2seg = l2seg;
        } else {
            cl_l2seg = l2seg;
        }
        // only in gtests as there are no EPs on uplinks
        pd_ep_reg_mac_info(l2seg, cl_l2seg, hp_l2seg, pi_if, uplink_if, lif, 
                           key, key_mask, data);
    }

#if 0
    // lkp_vrf
    SDK_ASSERT_RETURN(l2seg != NULL, HAL_RET_L2SEG_NOT_FOUND);
    key.flow_lkp_metadata_lkp_classic_vrf = 
        ((pd_l2seg_t *)(l2seg->pd))->l2seg_fl_lkup_id;
    if (l2seg_is_shared_mgmt_attached(l2seg) && l2seg_is_cust(l2seg)) {
        data.action_u.registered_macs_registered_macs.nic_mode = NIC_MODE_SMART;
        direct_to_otcam = true;
        memset(&key_mask, ~0, sizeof(key_mask));
        memset(&key_mask.flow_lkp_metadata_lkp_classic_vrf_mask, 0, 
               sizeof(key_mask.flow_lkp_metadata_lkp_classic_vrf_mask));
        key_mask_p = &key_mask;
    } else {
        data.action_u.registered_macs_registered_macs.nic_mode = NIC_MODE_CLASSIC;
    }
#endif

    // lkp_mac
    mac = ep_get_mac_addr(pi_ep);
    memcpy(key.flow_lkp_metadata_lkp_dstMacAddr, *mac, 6);
    memrev(key.flow_lkp_metadata_lkp_dstMacAddr, 6);

    // dst_lport
    data.action_id = REGISTERED_MACS_REGISTERED_MACS_ID;
    data.action_u.registered_macs_registered_macs.dst_lport =
        if_get_lport_id(pi_if);

    if (oper == TABLE_OPER_INSERT) {
        // Insert
        sdk_ret = reg_mac_tbl->insert(&key, &data, &hash_idx, key_mask_p, 
                                      direct_to_otcam);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to program for ep:{}, ret: {}",
                          ep_l2_key_to_str(pi_ep), ret);
            if (ret == HAL_RET_ENTRY_EXISTS) {
                // If hash lib returns entry exists, return hw prog error. Otherwise
                // entry exists means that vrf was already created.
                ret = HAL_RET_HW_PROG_ERR;
            }
            goto end;
        } else {
            HAL_TRACE_DEBUG("programmed for ep:{} at hash_idx:{}",
                            ep_l2_key_to_str(pi_ep), hash_idx);
        }

        pd_ep->reg_mac_tbl_idx = hash_idx;
    } else {
        hash_idx = pd_ep->reg_mac_tbl_idx;
        // Update
        sdk_ret = reg_mac_tbl->update(hash_idx, &data);
        ret = hal_sdk_ret_to_hal_ret(sdk_ret);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("unable to reprogram for ep:{} at: {}",
                          ep_l2_key_to_str(pi_ep), hash_idx);
            goto end;
        } else {
            HAL_TRACE_DEBUG("reprogrammed for ep:{} at: {}",
                            ep_l2_key_to_str(pi_ep), hash_idx);
        }
    }
end:
    return ret;
}

// ----------------------------------------------------------------------------
// DeProgram registered mac
// ----------------------------------------------------------------------------
hal_ret_t
ep_pd_depgm_registered_mac(pd_ep_t *pd_ep)
{
    hal_ret_t                   ret = HAL_RET_OK;
    sdk_ret_t                   sdk_ret;
    registered_macs_swkey_t     key;
    registered_macs_actiondata_t  data;
    sdk_hash                    *reg_mac_tbl = NULL;

    if (pd_ep->reg_mac_tbl_idx == INVALID_INDEXER_INDEX) {
        HAL_TRACE_DEBUG("Skipping as it was never programmed");
        goto end;
    }

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    reg_mac_tbl = g_hal_state_pd->hash_tcam_table(P4TBL_ID_REGISTERED_MACS);
    SDK_ASSERT_RETURN((reg_mac_tbl != NULL), HAL_RET_ERR);

    sdk_ret = reg_mac_tbl->remove(pd_ep->reg_mac_tbl_idx);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("classic: Unable to deprogram for: {}. ret:{}",
                      pd_ep->reg_mac_tbl_idx, ret);
        goto end;
    } else {
        // Getting trace from hash lib. This is redundant.
        HAL_TRACE_VERBOSE("classic: DeProgrammed at: {} ",
                          pd_ep->reg_mac_tbl_idx);
    }
    pd_ep->reg_mac_tbl_idx = INVALID_INDEXER_INDEX;

end:
    return ret;
}

// ----------------------------------------------------------------------------
// EP Quiesce entry add/delete
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_quiesce (pd_func_args_t *pd_func_args)
{
    hal_ret_t              ret = HAL_RET_OK;
    pd_ep_quiesce_args_t  *args = pd_func_args->pd_ep_quiesce;
    ep_t                  *pi_ep = args->ep;
    pd_ep_t               *pd_ep = (pd_ep_t *)pi_ep->pd;
    acl_tcam              *acl_tbl = g_hal_state_pd->acl_table();
    nacl_swkey_t           key;
    nacl_swkey_mask_t      mask;
    nacl_actiondata_t      data;
    if_t                  *pi_if = NULL;

    if (!pd_ep) {
        HAL_TRACE_ERR("EP Quiesce NACL removing error. pd_ep not found");
        return HAL_RET_ERR;
    }

    SDK_ASSERT_RETURN((acl_tbl != NULL), HAL_RET_ERR);

    if (args->entry_add) {
        pi_if = find_if_by_handle(pi_ep->if_handle);

        // Install destination NACL - Drop any traffic destined towards this local EP
        memset(&key, 0, sizeof(key));
        memset(&mask, 0, sizeof(mask));
        memset(&data, 0, sizeof(data));
        // Key
        key.entry_inactive_nacl              = 0;
        mask.entry_inactive_nacl_mask        = 0x1;
        key.control_metadata_from_cpu        = 0;
        mask.control_metadata_from_cpu_mask  = 0x1;
        memcpy(key.ethernet_dstAddr, pi_ep->l2_key.mac_addr, sizeof(mac_addr_t));
        memset(mask.ethernet_dstAddr_mask, 0xFF, sizeof(mac_addr_t));
        // Data
        data.action_id                 = NACL_NACL_DENY_ID;

        ret = acl_tbl->insert(&key, &mask, &data, ACL_QUIESCE_ENTRY_PRIORITY,
                              &pd_ep->ep_quiesce_dst_nacl_hdl);
        if (ret == HAL_RET_OK) {
            HAL_TRACE_DEBUG("EP Quiesce NACL programmed at: {}.", pd_ep->ep_quiesce_dst_nacl_hdl);
        } else {
            HAL_TRACE_ERR("EP Quiesce NACL programming error. ret: {}", ret);
        }

        // Install source NACL - Drop any traffic coming from this local EP
        memset(&key, 0, sizeof(key));
        memset(&mask, 0, sizeof(mask));
        // Key
        key.entry_inactive_nacl              = 0;
        mask.entry_inactive_nacl_mask        = 0x1;
        key.control_metadata_src_lport       = if_get_lport_id(pi_if);
        mask.control_metadata_src_lport_mask = 0xFFFF;
        // Data
        data.action_id                 = NACL_NACL_DENY_ID;

        ret = acl_tbl->insert(&key, &mask, &data, ACL_QUIESCE_ENTRY_PRIORITY,
                              &pd_ep->ep_quiesce_src_nacl_hdl);
        if (ret == HAL_RET_OK) {
            HAL_TRACE_DEBUG("EP Quiesce NACL programmed at: {}. lport:{}",
                             pd_ep->ep_quiesce_src_nacl_hdl, key.control_metadata_src_lport);
        } else {
            HAL_TRACE_ERR("EP Quiesce NACL programming error. ret: {}", ret);
        }
    } else {
        HAL_TRACE_DEBUG("EP Quiesce NACL removed at: Src:{} Dst:{}.",
                        pd_ep->ep_quiesce_src_nacl_hdl, pd_ep->ep_quiesce_dst_nacl_hdl);

        ret = acl_tbl->remove(pd_ep->ep_quiesce_src_nacl_hdl);

        if (ret == HAL_RET_OK) {
            pd_ep->ep_quiesce_src_nacl_hdl = INVALID_INDEXER_INDEX;
        } else {
            HAL_TRACE_ERR("EP Quiesce Source NACL removing error. ret: {} hndl: {}",
                          ret, pd_ep->ep_quiesce_src_nacl_hdl);
        }

        ret = acl_tbl->remove(pd_ep->ep_quiesce_dst_nacl_hdl);

        if (ret == HAL_RET_OK) {
            pd_ep->ep_quiesce_dst_nacl_hdl = INVALID_INDEXER_INDEX;
        } else {
            HAL_TRACE_ERR("EP Quiesce Destination NACL removing error. ret: {} hndl: {}",
                          ret, pd_ep->ep_quiesce_dst_nacl_hdl);
        }
    }
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
pd_ep_make_clone (pd_func_args_t *pd_func_args)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_ep_make_clone_args_t *args = pd_func_args->pd_ep_make_clone;
    pd_ep_t             *pd_ep_clone = NULL;
    ep_t *ep = args->ep;
    ep_t *clone = args->clone;

    pd_ep_clone = ep_pd_alloc_init();
    if (pd_ep_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_ep_clone, ep->pd, sizeof(pd_ep_t));

    ep_link_pi_pd(pd_ep_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_ep_mem_free (pd_func_args_t *pd_func_args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_ep_mem_free_args_t *args = pd_func_args->pd_ep_mem_free;
    pd_ep_t        *ep_pd;

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
    SDK_ASSERT(pi_if != NULL);

    if_type = intf_get_if_type(pi_if);
    switch(if_type) {
        case intf::IF_TYPE_ENIC:
            pi_lif = if_get_lif(pi_if);
            SDK_ASSERT(pi_lif != NULL);

            pd_lif = (pd_lif_t *)lif_get_pd_lif(pi_lif);
            SDK_ASSERT(pi_lif != NULL);

            return pd_lif->hw_lif_id;
            break;
        case intf::IF_TYPE_UPLINK:
            pd_upif = (pd_uplinkif_t *)if_get_pd_if(pi_if);
            SDK_ASSERT(pd_upif != NULL);

            return pd_upif->hw_lif_id;
            break;
        case intf::IF_TYPE_UPLINK_PC:
            pd_uppc = (pd_uplinkpc_t *)if_get_pd_if(pi_if);
            SDK_ASSERT(pd_uppc != NULL);

            return pd_uppc->hw_lif_id;
            break;
        case intf::IF_TYPE_TUNNEL:
            return (if_get_hw_lif_id(pi_if));
            break;
        default:
            SDK_ASSERT(0);
    }

    return INVALID_INDEXER_INDEX;
}

// ----------------------------------------------------------------------------
// Gets interface type
// ----------------------------------------------------------------------------
intf::IfType
ep_pd_get_if_type(ep_t *pi_ep)
{
    if_t            *pi_if;

    pi_if = ep_find_if_by_handle(pi_ep);
    SDK_ASSERT(pi_if != NULL);

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
    SDK_ASSERT(rw_act < REWRITE_MAX_ID);

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

    SDK_ASSERT(tnnl_rw_act < TUNNEL_REWRITE_MAX_ID);
    if (tnnl_rw_act == TUNNEL_REWRITE_ENCAP_VLAN_ID) {
        return g_hal_state_pd->tnnl_rwr_tbl_encap_vlan_idx();
    } else if (tnnl_rw_act == TUNNEL_REWRITE_ENCAP_VXLAN_ID) {
        if_t *tunnel_if = ep_find_if_by_handle((ep_t *)pd_ep->pi_ep);
        return (tunnelif_get_rw_idx((pd_tunnelif_t *)tunnel_if->pd_if));
    }

    return 0;
}

}    // namespace pd
}    // namespace hal
