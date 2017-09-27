#ifndef __HAL_L4LB_PD_HPP__
#define __HAL_L4LB_PD_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"

namespace hal {
namespace pd {

struct pd_l4lb_s {
    uint32_t    rw_tbl_idx[REWRITE_MAX_ID];

    // pi ptr
    void        *pi_l4lb;
} __PACK__;

hal_ret_t pd_l4lb_create(pd_l4lb_args_t *args);
pd_l4lb_t *l4lb_pd_alloc();
pd_l4lb_t *l4lb_pd_init(pd_l4lb_t *up_l4lb);
pd_l4lb_t *l4lb_pd_alloc_init();
hal_ret_t l4lb_pd_free(pd_l4lb_t *up_l4lb);
hal_ret_t l4lb_pd_alloc_res(pd_l4lb_t *up_l4lb);
hal_ret_t l4lb_pd_alloc_ip_entries(pd_l4lb_args_t *args);
hal_ret_t l4lb_pd_program_hw(pd_l4lb_t *up_l4lb);
void link_pi_pd(pd_l4lb_t *pd_l4lb, l4lb_service_entry_t *pi_l4lb);
void unlink_pi_pd(pd_l4lb_t *pd_l4lb, l4lb_service_entry_t *pi_up_l4lb);
hal_ret_t l4lb_pd_pgm_rw_tbl(pd_l4lb_t *pd_l4lb);

uint32_t l4lb_pd_get_rw_tbl_idx_from_pi_l4lb(l4lb_service_entry_t *pi_l4lb, 
                                         rewrite_actions_en rw_act);
uint32_t l4lb_pd_get_rw_tbl_idx(pd_l4lb_t *pd_l4lb, rewrite_actions_en rw_act);
}   // namespace pd
}   // namespace hal
#endif    // __HAL_L4LB_PD_HPP__

