#ifndef __HAL_ENDPOINT_PD_HPP__
#define __HAL_ENDPOINT_PD_HPP__

#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"
#include "nic/hal/pd/utils/acl_tcam/acl_tcam.hpp"

using hal::pd::utils::acl_tcam_entry_handle_t;

namespace hal {
namespace pd {

struct pd_ep_s {
    uint32_t                rw_tbl_idx[REWRITE_MAX_ID];
    uint32_t                reg_mac_tbl_idx;        // Only in classic mode
    acl_tcam_entry_handle_t ep_quiesce_nacl_hdl;    // EP Quiesce NACL Handle

    // pi ptr
    void                    *pi_ep;
} __PACK__;

struct pd_ep_ip_entry_s {
    uint32_t    ipsg_tbl_idx;

    // pi ptr
    void        *pi_ep_ip_entry;
} __PACK__;

// allocate EP Instance
static inline pd_ep_t *
ep_pd_alloc (void)
{
    pd_ep_t    *ep;

    ep = (pd_ep_t *)g_hal_state_pd->ep_pd_slab()->alloc();
    if (ep == NULL) {
        return NULL;
    }

    return ep;
}

// initialize EP PD instance
static inline pd_ep_t *
ep_pd_init (pd_ep_t *ep)
{
    // Nothing to do currently
    if (!ep) {
        return NULL;
    }

    // Set here if you want to initialize any fields
    ep->reg_mac_tbl_idx = INVALID_INDEXER_INDEX;

    return ep;
}

// allocate and Initialize EP PD Instance
static inline pd_ep_t *
ep_pd_alloc_init(void)
{
    return ep_pd_init(ep_pd_alloc());
}

// freeing EP PD
static inline hal_ret_t
ep_pd_free (pd_ep_t *ep)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_EP_PD, ep);
    return HAL_RET_OK;
}


// freeing EP PD memory
static inline hal_ret_t
ep_pd_mem_free (pd_ep_t *ep)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_EP_PD, ep);
    return HAL_RET_OK;
}

// allocate EP IP Instance
static inline pd_ep_ip_entry_t *
ep_pd_ip_alloc (void)
{
    pd_ep_ip_entry_t    *ep_ip;

    ep_ip = (pd_ep_ip_entry_t *)g_hal_state_pd->ep_pd_ip_entry_slab()->alloc();
    if (ep_ip == NULL) {
        return NULL;
    }

    return ep_ip;
}

// initialize EP IP instance
static inline pd_ep_ip_entry_t *
ep_pd_ip_init (pd_ep_ip_entry_t *ep_ip)
{
    // Nothing to do currently
    if (!ep_ip) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return ep_ip;
}

// allocate and Initialize EP PD Instance
static inline pd_ep_ip_entry_t *
ep_pd_ip_alloc_init(void)
{
    return ep_pd_ip_init(ep_pd_ip_alloc());
}

// freeing EP IP
static inline hal_ret_t
ep_pd_ip_free (pd_ep_ip_entry_t *ep_ip)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_EP_IP_ENTRY_PD, ep_ip);
    return HAL_RET_OK;
}


hal_ret_t ep_pd_alloc_res(pd_ep_t *up_ep);
hal_ret_t ep_pd_dealloc_res(pd_ep_t *up_ep);
hal_ret_t ep_pd_cleanup(pd_ep_t *ep_pd);


hal_ret_t ep_pd_alloc_ip_entries(ep_t *ep);
hal_ret_t ep_pd_program_hw(pd_ep_t *up_ep, bool is_upgrade = false);
void ep_link_pi_pd(pd_ep_t *pd_ep, ep_t *pi_ep);
void ep_delink_pi_pd(pd_ep_t *pd_ep, ep_t *pi_up_ep);
// hal_ret_t ep_pd_pgm_rw_tbl(pd_ep_t *pd_ep);
hal_ret_t ep_pd_pgm_ipsg_tbl(pd_ep_t *pd_ep,
                             bool is_upgrade,
                             pd_ep_if_update_args_t *if_args,
                             table_oper_t oper);
hal_ret_t ep_pd_pgm_ipsg_tble_per_ip(pd_ep_t *pd_ep,
                                     pd_ep_ip_entry_t *pd_ip_entry,
                                     bool is_upgrade,
                                     pd_ep_if_update_args_t *if_args,
                                     table_oper_t oper);
uint32_t ep_pd_get_hw_lif_id(ep_t *pi_ep);
intf::IfType ep_pd_get_if_type(ep_t *pi_ep);
uint32_t ep_pd_get_rw_tbl_idx_from_pi_ep(ep_t *pi_ep,
                                         rewrite_actions_en rw_act);
uint32_t
ep_pd_get_tnnl_rw_tbl_idx_from_pi_ep(ep_t *pi_ep,
                                     tunnel_rewrite_actions_en tnnl_rw_act);
uint32_t
ep_pd_get_tnnl_rw_tbl_idx(pd_ep_t *pd_ep,
                          tunnel_rewrite_actions_en tnnl_rw_act);
uint32_t ep_pd_get_rw_tbl_idx(pd_ep_t *pd_ep, rewrite_actions_en rw_act);
hal_ret_t pd_ep_upd_iplist_change (pd_ep_update_args_t *pd_ep_upd_args);
hal_ret_t ep_pd_alloc_pd_ip_entries (dllist_ctxt_t *pi_ep_list);
hal_ret_t ep_pd_pgm_ipsg_tbl_ip_entries(ep_t *pi_ep,
                                        dllist_ctxt_t *pi_ep_list,
                                        bool is_upgrade,
                                        pd_ep_if_update_args_t *if_args,
                                        table_oper_t oper);
hal_ret_t pd_ep_pgm_registered_mac(pd_ep_t *pd_ep, table_oper_t oper);
hal_ret_t ep_pd_depgm_registered_mac(pd_ep_t *pd_ep);

hal_ret_t ep_pd_depgm_ipsg_tbl_ip_entries(ep_t *pi_ep,
                                          dllist_ctxt_t *pi_ep_list);
hal_ret_t ep_pd_depgm_ipsg_tble_per_ip(pd_ep_ip_entry_t *pd_ip_entry);
hal_ret_t ep_pd_delete_pd_ip_entries(ep_t *pi_ep, dllist_ctxt_t *pi_ep_list);
}   // namespace pd
}   // namespace hal
#endif    // __HAL_ENDPOINT_PD_HPP__

