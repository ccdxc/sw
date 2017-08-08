#ifndef __HAL_PD_NWSEC_HPP__
#define __HAL_PD_NWSEC_HPP__

#include <base.h>
#include <pd.hpp>
#include <pd_api.hpp>

namespace hal {
namespace pd {

#define HAL_MAX_HW_NWSEC_PROFILES        256

typedef uint32_t    nwsec_profile_hw_id_t;

// security profile pd state
struct pd_nwsec_profile_s {
    void                    *nwsec_profile;    // PI security profile

    nwsec_profile_hw_id_t   nwsec_hw_id;       // hw id for this segment
} __PACK__;

hal_ret_t pd_nwsec_profile_create (pd_nwsec_profile_args_t *args);
pd_nwsec_profile_t *pd_nwsec_alloc_init(void);
pd_nwsec_profile_t *pd_nwsec_alloc (void);
pd_nwsec_profile_t *pd_nwsec_init (pd_nwsec_profile_t *pd_nw);
hal_ret_t pd_nwsec_alloc_res(pd_nwsec_profile_t *pd_nw);
hal_ret_t pd_nwsec_program_hw(pd_nwsec_profile_t *pd_nw);
hal_ret_t pd_nwsec_free (pd_nwsec_profile_t *pd_nw);
void link_pi_pd(pd_nwsec_profile_t *pd_nw, nwsec_profile_t *pi_nw);
void unlink_pi_pd(pd_nwsec_profile_t *pd_nw, nwsec_profile_t  *pi_nw);
hal_ret_t p4pd_program_l4_profile_table (pd_nwsec_profile_t *pd_nw);


}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_NWSEC_HPP__

