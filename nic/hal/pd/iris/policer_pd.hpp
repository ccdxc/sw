#ifndef __HAL_POLICER_PD_HPP__
#define __HAL_POLICER_PD_HPP__

#include <base.h>
#include <pd.hpp>
#include <pd_api.hpp>


namespace hal {
namespace pd {

#define HAL_INVALID_HW_POLICER_ID 0xffffffff

struct pd_policer_s {
    uint32_t    hw_policer_id;
    uint32_t    dummy;

    // pi ptr
    void        *pi_policer;
} __PACK__;


pd_policer_t *policer_pd_alloc ();
pd_policer_t *policer_pd_init (pd_policer_t *policer);
pd_policer_t *policer_pd_alloc_init ();
hal_ret_t policer_pd_free (pd_policer_t *policer);
hal_ret_t policer_pd_alloc_res(pd_policer_t *pd_policer);
void policer_pd_dealloc_res (pd_policer_t *pd_policer);
hal_ret_t policer_pd_program_hw(pd_policer_t *pd_policer);
void link_pi_pd(pd_policer_t *pd_policer, policer_t *pi_policer);
void unlink_pi_pd(pd_policer_t *pd_policer, policer_t *pi_policer);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_POLICER_PD_HPP__
