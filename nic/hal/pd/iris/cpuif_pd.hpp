#ifndef __HAL_PD_CPUIF_HPP__
#define __HAL_PD_CPUIF_HPP__

#include "nic/include/base.h"
#include "nic/include/pd.hpp"

namespace hal {
namespace pd {

struct pd_cpuif_s {
    uint32_t    cpu_lport_id;   // lport for cpu

    void        *pi_if;         // pi ptr
} __PACK__;

// ----------------------------------------------------------------------------
// Allocate CPU IF Instance
// ----------------------------------------------------------------------------
static pd_cpuif_t *
pd_cpuif_alloc (void)
{
    pd_cpuif_t    *cpuif;

    cpuif = (pd_cpuif_t *)g_hal_state_pd->cpuif_pd_slab()->alloc();
    if (cpuif == NULL) {
        return NULL;
    }
    return cpuif;
}

// ----------------------------------------------------------------------------
// Initialize CPU IF PD instance
// ----------------------------------------------------------------------------
static pd_cpuif_t *
pd_cpuif_init (pd_cpuif_t *cpuif)
{
    // Nothing to do currently
    if (!cpuif) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return cpuif;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize CPU If PD Instance
// ----------------------------------------------------------------------------
static pd_cpuif_t *
pd_cpuif_alloc_init(void)
{
    return pd_cpuif_init(pd_cpuif_alloc());
}

// ----------------------------------------------------------------------------
// Freeing CPU IF PD
// ----------------------------------------------------------------------------
static hal_ret_t
pd_cpuif_free (pd_cpuif_t *cpuif)
{
    g_hal_state_pd->cpuif_pd_slab()->free(cpuif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Freeing CPU IF PD memory
// ----------------------------------------------------------------------------
static hal_ret_t
pd_cpuif_pd_mem_free (pd_cpuif_t *cpuif)
{
    g_hal_state_pd->cpuif_pd_slab()->free(cpuif);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
static void 
cpuif_link_pi_pd(pd_cpuif_t *pd_cpuif, if_t *pi_if)
{
    pd_cpuif->pi_if = pi_if;
    if_set_pd_if(pi_if, pd_cpuif);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
static void 
cpuif_delink_pi_pd(pd_cpuif_t *pd_cpuif, if_t *pi_if)
{
    pd_cpuif->pi_if = NULL;
    if_set_pd_if(pi_if, NULL);
}


hal_ret_t pd_cpuif_alloc_res(pd_cpuif_t *pd_cpuif);
hal_ret_t pd_cpuif_program_hw(pd_cpuif_t *pd_cpuif);
hal_ret_t pd_cpuif_pd_pgm_output_mapping_tbl(pd_cpuif_t *pd_cpuif);
hal_ret_t pd_cpuif_cleanup(pd_cpuif_t *upif_pd);
hal_ret_t pd_cpuif_deprogram_hw (pd_cpuif_t *pd_upif);
hal_ret_t pd_cpuif_pd_depgm_output_mapping_tbl(pd_cpuif_t *pd_cpuif);

pd_lif_t *pd_cpuif_get_pd_lif(pd_cpuif_t *pd_cpuif);

hal_ret_t pd_cpuif_create(pd_if_args_t *args);
hal_ret_t pd_cpuif_update(pd_if_args_t *args);
hal_ret_t pd_cpuif_delete(pd_if_args_t *args);
hal_ret_t pd_cpuif_make_clone(if_t *hal_if, if_t *clone);
hal_ret_t pd_cpuif_mem_free(pd_if_args_t *args);

}   // namespace pd
}   // namespace hal
#endif    // __HAL_PD_CPUIF_HPP__

