#ifndef __HAL_UPLINKPC_PD_HPP__
#define __HAL_UPLINKPC_PD_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/iris/nw/if_pd_utils.hpp"

namespace hal {
namespace pd {

struct pd_uplinkpc_s {
    uint32_t    hw_lif_id;      // hw lif id coming from tm
    uint32_t    up_ifpc_id;     // Unique for Uplinkif & PC. Used in l2seg
    uint32_t    uppc_lport_id;  // lport

    // pi ptr
    void        *pi_if;
} __PACK__;


// ----------------------------------------------------------------------------
// Allocate Uplink IF Instance
// ----------------------------------------------------------------------------
static inline pd_uplinkpc_t *
uplinkpc_pd_alloc (void)
{
    pd_uplinkpc_t    *upif;

    upif = (pd_uplinkpc_t *)g_hal_state_pd->uplinkpc_pd_slab()->alloc();
    if (upif == NULL) {
        return NULL;
    }
    return upif;
}

// ----------------------------------------------------------------------------
// Initialize Uplink IF PD instance
// ----------------------------------------------------------------------------
static inline pd_uplinkpc_t *
uplinkpc_pd_init (pd_uplinkpc_t *upif)
{
    // Nothing to do currently
    if (!upif) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return upif;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize uplinkpc PD Instance
// ----------------------------------------------------------------------------
static inline pd_uplinkpc_t *
uplinkpc_pd_alloc_init(void)
{
    return uplinkpc_pd_init(uplinkpc_pd_alloc());
}

// ----------------------------------------------------------------------------
// Freeing uplinkpc PD
// ----------------------------------------------------------------------------
static inline hal_ret_t
uplinkpc_pd_free (pd_uplinkpc_t *upif)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_UPLINKPC_PD, upif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Freeing uplinkpc PD memory
// ----------------------------------------------------------------------------
static inline hal_ret_t
uplinkpc_pd_mem_free (pd_uplinkpc_t *upif)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_UPLINKPC_PD, upif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
static inline void
uplinkpc_link_pi_pd(pd_uplinkpc_t *pd_upif, if_t *pi_if)
{
    pd_upif->pi_if = pi_if;
    if_set_pd_if(pi_if, pd_upif);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
static inline void
uplinkpc_delink_pi_pd(pd_uplinkpc_t *pd_upif, if_t *pi_if)
{
    pd_upif->pi_if = NULL;
    if_set_pd_if(pi_if, NULL);
}

hal_ret_t uplinkpc_pd_alloc_res(pd_uplinkpc_t *up_if);
hal_ret_t uplinkpc_pd_program_hw(pd_uplinkpc_t *up_if, bool is_upgrade = false);
hal_ret_t uplinkpc_pd_pgm_tm_register(pd_uplinkpc_t *up_if, bool add);
hal_ret_t uplinkpc_pd_pgm_tm_register_per_upif(pd_uplinkpc_t *pd_uppcif,
                                                 pd_uplinkif_t *pd_upif,
                                                 bool add);
hal_ret_t uplinkpc_pd_upd_tm_register (pd_if_update_args_t *args);
hal_ret_t
uplinkpc_pd_pgm_output_mapping_tbl(pd_uplinkpc_t *pd_uppcif,
                                   block_list *mbr_list,
                                   table_oper_t oper);
hal_ret_t uplinkpc_pd_cleanup(pd_uplinkpc_t *upif_pd);
hal_ret_t uplinkpc_pd_deprogram_hw (pd_uplinkpc_t *pd_upif);
hal_ret_t uplinkpc_pd_depgm_output_mapping_tbl (pd_uplinkpc_t *pd_upif);

hal_ret_t pd_uplinkpc_create(pd_if_create_args_t *args);
hal_ret_t pd_uplinkpc_update(pd_if_update_args_t *args);
hal_ret_t pd_uplinkpc_delete(pd_if_delete_args_t *args);
hal_ret_t pd_uplinkpc_get (pd_if_get_args_t *args);
hal_ret_t pd_uplinkpc_make_clone(pd_if_make_clone_args_t *args);
hal_ret_t pd_uplinkpc_mem_free(pd_if_mem_free_args_t *args);
hal_ret_t pd_uplinkpc_restore(pd_if_restore_args_t *args);

hal_ret_t pd_uplinkpc_restore (pd_if_restore_args_t *args);

}   // namespace pd
}   // namespace hal
#endif    // __HAL_UPLINKPC_PD_HPP__

