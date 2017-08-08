#ifndef __HAL_UPLINKIF_PD_HPP__
#define __HAL_UPLINKIF_PD_HPP__

#include <base.h>
#include <pd.hpp>

namespace hal {
namespace pd {

struct pd_uplinkif_s {
    uint32_t    hw_lif_id;
    uint32_t    up_ifpc_id;

    // pi ptr
    void        *pi_if;
} __PACK__;

hal_ret_t pd_uplinkif_create(pd_if_args_t *args);
pd_uplinkif_t *uplinkif_pd_alloc();
pd_uplinkif_t *uplinkif_pd_init(pd_uplinkif_t *up_if);
pd_uplinkif_t *uplinkif_pd_alloc_init();
hal_ret_t uplinkif_pd_free(pd_uplinkif_t *up_if);
hal_ret_t uplinkif_pd_alloc_res(pd_uplinkif_t *up_if);
hal_ret_t uplinkif_pd_program_hw(pd_uplinkif_t *up_if);
hal_ret_t uplinkif_pd_pgm_tm_register(pd_uplinkif_t *up_if);
hal_ret_t uplinkif_pd_pgm_output_mapping_tbl(pd_uplinkif_t *up_if);
void link_pi_pd(pd_uplinkif_t *pd_upif, if_t *pi_if);
void unlink_pi_pd(pd_uplinkif_t *pd_up_if, if_t *pi_up_if);

}   // namespace pd
}   // namespace hal
#endif    // __HAL_UPLINKIF_PD_HPP__

