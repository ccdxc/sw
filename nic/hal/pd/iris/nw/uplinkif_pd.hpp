#ifndef __HAL_UPLINKIF_PD_HPP__
#define __HAL_UPLINKIF_PD_HPP__

#include "nic/include/base.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"

namespace hal {
namespace pd {

struct pd_uplinkif_s {
    uint32_t    hw_lif_id;          // hw lif id
    uint32_t    up_ifpc_id;         // Unique for Uplinkif & PC. Used in l2seg
    uint32_t    upif_lport_id;      // lport

    void        *pi_if;             // pi back-reference
} __PACK__;

// ----------------------------------------------------------------------------
// Allocate Uplink IF Instance
// ----------------------------------------------------------------------------
static inline pd_uplinkif_t *
uplinkif_pd_alloc (void)
{
    pd_uplinkif_t    *upif;

    upif = (pd_uplinkif_t *)g_hal_state_pd->uplinkif_pd_slab()->alloc();
    if (upif == NULL) {
        return NULL;
    }
    return upif;
}

// ----------------------------------------------------------------------------
// Initialize Uplink IF PD instance
// ----------------------------------------------------------------------------
static inline pd_uplinkif_t *
uplinkif_pd_init (pd_uplinkif_t *upif)
{
    // Nothing to do currently
    if (!upif) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return upif;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize Uplinkif PD Instance
// ----------------------------------------------------------------------------
static inline pd_uplinkif_t *
uplinkif_pd_alloc_init(void)
{
    return uplinkif_pd_init(uplinkif_pd_alloc());
}

// ----------------------------------------------------------------------------
// Freeing UPLINKIF PD
// ----------------------------------------------------------------------------
static inline hal_ret_t
uplinkif_pd_free (pd_uplinkif_t *upif)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_UPLINKIF_PD, upif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Freeing UPLINKIF PD memory
// ----------------------------------------------------------------------------
static inline hal_ret_t
uplinkif_pd_mem_free (pd_uplinkif_t *upif)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_UPLINKIF_PD, upif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
static inline void
uplinkif_link_pi_pd(pd_uplinkif_t *pd_upif, if_t *pi_if)
{
    pd_upif->pi_if = pi_if;
    if_set_pd_if(pi_if, pd_upif);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
static inline void
uplinkif_delink_pi_pd(pd_uplinkif_t *pd_upif, if_t *pi_if)
{
    pd_upif->pi_if = NULL;
    if_set_pd_if(pi_if, NULL);
}


hal_ret_t uplinkif_pd_alloc_res(pd_uplinkif_t *up_if);
hal_ret_t uplinkif_pd_dealloc_res(pd_uplinkif_t *up_if);
hal_ret_t uplinkif_pd_program_hw(pd_uplinkif_t *up_if, bool is_upgrade = false);
hal_ret_t uplinkif_pd_pgm_tm_register(pd_uplinkif_t *up_if);
hal_ret_t uplinkif_pd_pgm_output_mapping_tbl(pd_uplinkif_t *up_if);
hal_ret_t uplinkif_pd_cleanup(pd_uplinkif_t *upif_pd);
hal_ret_t uplinkif_pd_depgm_output_mapping_tbl (pd_uplinkif_t *pd_upif);
hal_ret_t uplinkif_pd_depgm_tm_register(pd_uplinkif_t *pd_upif);
hal_ret_t uplinkif_pd_deprogram_hw (pd_uplinkif_t *pd_upif);
uint32_t pd_uplinkif_pdif_label(pd_uplinkif_t *upif);
uint32_t pd_uplinkif_if_label(if_t *hal_if);

hal_ret_t pd_uplinkif_create(pd_if_create_args_t *args);
hal_ret_t pd_uplinkif_update(pd_if_update_args_t *args);
hal_ret_t pd_uplinkif_delete(pd_if_delete_args_t *args);
hal_ret_t pd_uplinkif_get(pd_if_get_args_t *args);
hal_ret_t pd_uplinkif_make_clone(pd_if_make_clone_args_t *args);
hal_ret_t pd_uplinkif_mem_free(pd_if_mem_free_args_t *args);
hal_ret_t pd_uplinkif_restore(pd_if_restore_args_t *args);

}   // namespace pd
}   // namespace hal
#endif    // __HAL_UPLINKIF_PD_HPP__

