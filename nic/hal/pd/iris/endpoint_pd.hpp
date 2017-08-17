#ifndef __HAL_ENDPOINT_PD_HPP__
#define __HAL_ENDPOINT_PD_HPP__

#include <base.h>
#include <pd.hpp>

namespace hal {
namespace pd {

struct pd_ep_s {
    uint32_t    rw_tbl_idx[REWRITE_MAX_ID];

    // pi ptr
    void        *pi_ep;
} __PACK__;

struct pd_ep_ip_entry_s {
    uint32_t    ipsg_tbl_idx;

    // pi ptr
    void        *pi_ep_ip_entry;
} __PACK__;


hal_ret_t pd_ep_create(pd_ep_args_t *args);
pd_ep_t *ep_pd_alloc();
pd_ep_t *ep_pd_init(pd_ep_t *up_ep);
pd_ep_t *ep_pd_alloc_init();
hal_ret_t ep_pd_free(pd_ep_t *up_ep);
hal_ret_t ep_pd_alloc_res(pd_ep_t *up_ep);
hal_ret_t ep_pd_alloc_ip_entries(pd_ep_args_t *args);
hal_ret_t ep_pd_program_hw(pd_ep_t *up_ep);
void link_pi_pd(pd_ep_t *pd_ep, ep_t *pi_ep);
void unlink_pi_pd(pd_ep_t *pd_ep, ep_t *pi_up_ep);
hal_ret_t ep_pd_pgm_rw_tbl(pd_ep_t *pd_ep);
hal_ret_t ep_pd_pgm_ipsg_tbl(pd_ep_t *pd_ep);
hal_ret_t ep_pd_pgm_ipsg_tble_per_ip(pd_ep_t *pd_ep, 
                                     pd_ep_ip_entry_t *pd_ip_entry);
uint32_t ep_pd_get_hw_lif_id(ep_t *pi_ep);
intf::IfType ep_pd_get_if_type(ep_t *pi_ep);
uint32_t ep_pd_get_rw_tbl_idx_from_pi_ep(ep_t *pi_ep, 
                                         rewrite_actions_en rw_act);
uint32_t
ep_pd_get_tnnl_rw_tbl_idx(pd_ep_t *pd_ep, 
                          tunnel_rewrite_actions_en tnnl_rw_act);
uint32_t ep_pd_get_rw_tbl_idx(pd_ep_t *pd_ep, rewrite_actions_en rw_act);
network_t *ep_pd_get_nw(ep_t *pi_ep, l2seg_t *l2seg);
}   // namespace pd
}   // namespace hal
#endif    // __HAL_ENDPOINT_PD_HPP__

