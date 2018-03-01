#ifndef __HAL_PD_VRF_HPP__
#define __HAL_PD_VRF_HPP__

#include "nic/include/base.h"
#include "sdk/ht.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/gft/gft_state.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_VRFS         256
#define HAL_PD_L2SEG_MASK       0xFFF
#define HAL_PD_VRF_MASK         0xF000
#define HAL_PD_VRF_SHIFT        12

typedef uint32_t    vrf_hw_id_t;

// vrf pd state
struct pd_vrf_s {
    void               *vrf;                 // PI vrf

} __PACK__;

// allocate a vrf pd instance
static inline pd_vrf_t *
vrf_pd_alloc (void)
{
    pd_vrf_t    *vrf_pd;

    vrf_pd = (pd_vrf_t *)g_hal_state_pd->vrf_pd_slab()->alloc();
    if (vrf_pd == NULL) {
        return NULL;
    }

    return vrf_pd;
}

// initialize a vrf pd instance
static inline pd_vrf_t *
vrf_pd_init (pd_vrf_t *vrf_pd)
{
    if (!vrf_pd) {
        return NULL;
    }
    vrf_pd->vrf = NULL;
    return vrf_pd;
}

// allocate and initialize a vrf pd instance
static inline pd_vrf_t *
vrf_pd_alloc_init (void)
{
    return vrf_pd_init(vrf_pd_alloc());
}

// freeing vrf pd
static inline hal_ret_t
vrf_pd_free (pd_vrf_t *ten)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_VRF_PD, ten);
    return HAL_RET_OK;
}

// freeing vrf pd memory
static inline hal_ret_t
vrf_pd_mem_free (pd_vrf_t *ten)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_VRF_PD, ten);
    return HAL_RET_OK;
}

hal_ret_t vrf_pd_alloc_res(pd_vrf_t *pd_ten);
hal_ret_t vrf_pd_dealloc_res(pd_vrf_t *vrf_pd);
hal_ret_t vrf_pd_cleanup(pd_vrf_t *vrf_pd);
void link_pi_pd(pd_vrf_t *pd_ten, vrf_t *pi_ten);
void delink_pi_pd(pd_vrf_t *pd_ten, vrf_t *pi_ten);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_VRF_HPP__
