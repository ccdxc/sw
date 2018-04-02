#ifndef __HAL_PD_NWSEC_HPP__
#define __HAL_PD_NWSEC_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/pd_api.hpp"

namespace hal {
namespace pd {

#define HAL_MAX_HW_NWSEC_PROFILES        256

typedef uint32_t    nwsec_profile_hw_id_t;

// security profile pd state
struct pd_nwsec_profile_s {
    void                    *nwsec_profile;    // PI security profile

    nwsec_profile_hw_id_t   nwsec_hw_id;       // hw id for this segment
} __PACK__;

// allocate a nwsec pd instance
static inline pd_nwsec_profile_t *
nwsec_pd_alloc (void)
{
    pd_nwsec_profile_t    *nwsec_pd;

    nwsec_pd = (pd_nwsec_profile_t *)g_hal_state_pd->nwsec_pd_slab()->alloc();
    if (nwsec_pd == NULL) {
        return NULL;
    }

    return nwsec_pd;
}

// initialize a nwsec pd instance
static inline pd_nwsec_profile_t *
nwsec_pd_init (pd_nwsec_profile_t *nwsec_pd)
{
    if (!nwsec_pd) {
        return NULL;
    }
    nwsec_pd->nwsec_profile = NULL;
    nwsec_pd->nwsec_hw_id = 0;

    return nwsec_pd;
}

// allocate and initialize a nwsec pd instance
static inline pd_nwsec_profile_t *
nwsec_pd_alloc_init (void)
{
    return nwsec_pd_init(nwsec_pd_alloc());
}

// free nwsec pd instance
static inline hal_ret_t
nwsec_pd_free (pd_nwsec_profile_t *nwsec_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_SECURITY_PROFILE_PD, nwsec_pd);
    return HAL_RET_OK;
}

// free nwsec pd instance. Just freeing as it will be used during
// update to just memory free.
static inline hal_ret_t
nwsec_pd_mem_free (pd_nwsec_profile_t *nwsec_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_SECURITY_PROFILE_PD, nwsec_pd);
    return HAL_RET_OK;
}

hal_ret_t nwsec_pd_alloc_res(pd_nwsec_profile_t *pd_nw);
hal_ret_t nwsec_pd_dealloc_res(pd_nwsec_profile_t *pd_nw);
hal_ret_t nwsec_pd_cleanup(pd_nwsec_profile_t *nwsec_pd);
hal_ret_t nwsec_pd_program_hw(pd_nwsec_profile_t *pd_nw, bool create);
hal_ret_t pd_nwsec_free (pd_nwsec_profile_t *pd_nw);
void nwsec_link_pi_pd(pd_nwsec_profile_t *pd_nw, nwsec_profile_t *pi_nw);
void nwsec_delink_pi_pd(pd_nwsec_profile_t *pd_nw, nwsec_profile_t  *pi_nw);
hal_ret_t nwsec_pd_pgm_l4_profile_table (pd_nwsec_profile_t *pd_nw, bool create);
hal_ret_t nwsec_pd_depgm_l4_prof_tbl (pd_nwsec_profile_t *nwsec_pd);
hal_ret_t nwsec_pd_deprogram_hw (pd_nwsec_profile_t *nwsec_pd);


}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_NWSEC_HPP__

