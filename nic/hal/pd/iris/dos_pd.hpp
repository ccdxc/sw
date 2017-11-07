#ifndef __HAL_PD_DOS_HPP__
#define __HAL_PD_DOS_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/common/pd_api.hpp"
#include "nic/include/endpoint_api.hpp"

namespace hal {
namespace pd {

#define HAL_MAX_HW_DOS_PROFILES        256

typedef uint32_t    dos_policy_hw_id_t;

typedef enum pd_ddos_policer_type_e_ {
    DDOS_POLICER_TYPE_SRC_VF = 0,
    DDOS_POLICER_TYPE_SRC_DST,
    DDOS_POLICER_TYPE_SERVICE,
} pd_ddos_policer_type_e;

// security profile pd state
struct pd_dos_policy_s {
    // PI DoS policy
    void                  *pi_dos_policy;
    // DDoS src vf table and policer hw idx
    int                   ddos_src_vf_hw_id;
    int                   ddos_src_vf_pol_hw_id;
    // DDoS src dst table and policer hw idx
    int                   ddos_src_dst_hw_id;
    int                   ddos_src_dst_pol_hw_id;
    // DDoS service table and policer hw idx
    int                   ddos_service_hw_id;
    int                   ddos_service_pol_hw_id;
} __PACK__;

// allocate a dos pd instance
static inline pd_dos_policy_t *dos_pd_alloc (void)
{
    pd_dos_policy_t    *dos_pd;

    dos_pd = (pd_dos_policy_t *)g_hal_state_pd->dos_pd_slab()->alloc();
    if (dos_pd == NULL) {
        return NULL;
    }

    return dos_pd;
}

// initialize a dos pd instance
static inline pd_dos_policy_t *
dos_pd_init (pd_dos_policy_t *dos_pd)
{
    if (!dos_pd) {
        return NULL;
    }
    dos_pd->pi_dos_policy = NULL;
    dos_pd->ddos_src_vf_hw_id = -1;
    dos_pd->ddos_src_vf_pol_hw_id = -1;
    dos_pd->ddos_src_dst_hw_id = -1;
    dos_pd->ddos_src_dst_pol_hw_id = -1;
    dos_pd->ddos_service_hw_id = -1;
    dos_pd->ddos_service_pol_hw_id = -1;

    return dos_pd;
}

// allocate and initialize a dos pd instance
static inline pd_dos_policy_t *
dos_pd_alloc_init (void)
{
    return dos_pd_init(dos_pd_alloc());
}

// free dos pd instance
static inline hal_ret_t
dos_pd_free (pd_dos_policy_t *dos_pd)
{
    g_hal_state_pd->dos_pd_slab()->free(dos_pd);
    return HAL_RET_OK;
}

// free dos pd instance. Just freeing as it will be used during
// update to just memory free.
static inline hal_ret_t
dos_pd_mem_free (pd_dos_policy_t *dos_pd)
{
    g_hal_state_pd->dos_pd_slab()->free(dos_pd);
    return HAL_RET_OK;
}

hal_ret_t dos_pd_alloc_res(pd_dos_policy_t *pd_nw);
hal_ret_t dos_pd_dealloc_res(pd_dos_policy_t *pd_nw);
hal_ret_t dos_pd_cleanup(pd_dos_policy_t *dos_pd);
hal_ret_t pd_dos_free (pd_dos_policy_t *pd_nw);
void dos_link_pi_pd(pd_dos_policy_t *pd_nw, dos_policy_t *pi_nw);
void dos_delink_pi_pd(pd_dos_policy_t *pd_nw, dos_policy_t  *pi_nw);
hal_ret_t dos_pd_program_hw(pd_dos_policy_t *pd_nw, bool create);
hal_ret_t dos_pd_deprogram_hw (pd_dos_policy_t *dos_pd);


}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_DOS_HPP__

