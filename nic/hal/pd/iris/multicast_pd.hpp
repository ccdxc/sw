// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __HAL_PD_MULTICAST_HPP__
#define __HAL_PD_MULTICAST_HPP__

#include "nic/include/base.h"
#include "sdk/ht.hpp"
#include "nic/include/pd.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/pd_api.hpp"

using sdk::lib::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_MC_ENTRIES           2048

// mc_entry pd state
struct pd_mc_entry_s {
    void                *mc_entry;      // PI mc entry

    // operational state of mc_entry pd
    uint32_t    reg_mac_tbl_idx;        // Only in classic mode

} __PACK__;

// allocate a mc_entry pd instance
static inline pd_mc_entry_t *
mc_entry_pd_alloc (void)
{
    pd_mc_entry_t    *mc_entry_pd;

    mc_entry_pd = (pd_mc_entry_t *)g_hal_state_pd->mc_entry_slab()->alloc();
    if (mc_entry_pd == NULL) {
        return NULL;
    }

    return mc_entry_pd;
}

// initialize a mc_entry pd instance
static inline pd_mc_entry_t *
mc_entry_pd_init (pd_mc_entry_t *mc_entry_pd)
{
    if (!mc_entry_pd) {
        return NULL;
    }
    mc_entry_pd->mc_entry = NULL;

    mc_entry_pd->reg_mac_tbl_idx = INVALID_INDEXER_INDEX;

    return mc_entry_pd;
}

// allocate and initialize a mc_entry pd instance
static inline pd_mc_entry_t *
mc_entry_pd_alloc_init (void)
{
    return mc_entry_pd_init(mc_entry_pd_alloc());
}

// free mc_entry pd instance
static inline hal_ret_t
mc_entry_pd_free (pd_mc_entry_t *mc_entry_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_MC_ENTRY_PD, mc_entry_pd);
    return HAL_RET_OK;
}

// free mc_entry pd instance. Just freeing as it will be used during
// update to just memory free.
static inline hal_ret_t
mc_entry_pd_mem_free (pd_mc_entry_t *mc_entry_pd)
{
    hal::pd::delay_delete_to_slab(HAL_SLAB_MC_ENTRY_PD, mc_entry_pd);
    return HAL_RET_OK;
}

void mc_entry_link_pi_pd(pd_mc_entry_t *pd_mc_entry, mc_entry_t *pi_mc_entry);
void mc_entry_delink_pi_pd(pd_mc_entry_t *pd_mc_entry, mc_entry_t *pi_mc_entry);

hal_ret_t mc_entry_pd_cleanup(pd_mc_entry_t *mc_entry_pd);
hal_ret_t mc_entry_pd_program_hw(pd_mc_entry_t *mc_entry_pd);
hal_ret_t mc_entry_pd_deprogram_hw (pd_mc_entry_t *mc_entry_pd);
hal_ret_t pd_mc_entry_pgm_registered_mac(pd_mc_entry_t *mc_entry_pd);
hal_ret_t pd_mc_entry_depgm_registered_mac(pd_mc_entry_t *mc_entry_pd);


}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_MULTICAST_HPP__
