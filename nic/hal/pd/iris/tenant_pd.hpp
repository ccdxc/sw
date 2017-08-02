#ifndef __HAL_PD_TENANT_HPP__
#define __HAL_PD_TENANT_HPP__

#include <base.h>
#include <ht.hpp>
#include <pd.hpp>
#include <hal_state_pd.hpp>

using hal::utils::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_VRFS     256

typedef uint8_t    tenant_hw_id_t;

// tenant pd state
struct pd_tenant_s {
    void               *tenant;              // PI tenant

    // operational state of tenant pd
    tenant_hw_id_t     hw_id;                // hw id for this VRF

    // meta data maintained for tenant pd
    ht_ctxt_t          hw_ht_ctxt;           // h/w id based hash table ctxt
} __PACK__;

// allocate a tenant pd instance
static inline pd_tenant_t *
tenant_pd_alloc (void)
{
    pd_tenant_t    *tenant_pd;

    tenant_pd = (pd_tenant_t *)g_hal_state_pd->tenant_slab()->alloc();
    if (tenant_pd == NULL) {
        return NULL;
    }

    return tenant_pd;
}

// initialize a tenant pd instance
static inline pd_tenant_t *
tenant_pd_init (pd_tenant_t *tenant_pd)
{
    if (!tenant_pd) {
        return NULL;
    }
    tenant_pd->tenant = NULL;

    // initialize meta information
    tenant_pd->hw_ht_ctxt.reset();

    return tenant_pd;
}

// allocate and initialize a tenant pd instance
static inline pd_tenant_t *
tenant_pd_alloc_init (void)
{
    return tenant_pd_init(tenant_pd_alloc());
}

// free tenant pd instance
static inline hal_ret_t
tenant_pd_free (pd_tenant_t *tenant_pd)
{
    g_hal_state_pd->tenant_slab()->free(tenant_pd);
    return HAL_RET_OK;
}

// insert tenant pd state in all meta data structures
static inline hal_ret_t
add_tenant_pd_to_db (pd_tenant_t *tenant_pd)
{
    g_hal_state_pd->tenant_hwid_ht()->insert(tenant_pd, &tenant_pd->hw_ht_ctxt);
    return HAL_RET_OK;
}

// find a tenant pd instance given its hw id
static inline pd_tenant_t *
find_tenant_by_hwid (tenant_hw_id_t hwid)
{
    return (pd_tenant_t *)g_hal_state_pd->tenant_hwid_ht()->lookup(&hwid);
}

extern void *tenant_pd_get_hw_key_func(void *entry);
extern uint32_t tenant_pd_compute_hw_hash_func(void *key, uint32_t ht_size);
extern bool tenant_pd_compare_hw_key_func(void *key1, void *key2);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_TENANT_HPP__
