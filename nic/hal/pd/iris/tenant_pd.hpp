#ifndef __HAL_PD_TENANT_HPP__
#define __HAL_PD_TENANT_HPP__

#include <base.h>
#include <ht.hpp>
#include <pd.hpp>
#include <l2seg_pd.hpp>
#include <hal_state_pd.hpp>

using hal::utils::ht_ctxt_t;

namespace hal {
namespace pd {

#define HAL_MAX_HW_VRFS         256
#define HAL_PD_L2SEG_MASK       0xFFF
#define HAL_PD_TENANT_MASK      0xF000
#define HAL_PD_TENANT_SHIFT     12

typedef uint32_t    tenant_hw_id_t;

// tenant pd state
struct pd_tenant_s {
    void               *tenant;              // PI tenant

    // operational state of tenant pd
    tenant_hw_id_t     ten_hw_id;            // hw id for this VRF
    indexer            *l2seg_hw_id_idxr_;   // indexer for l2segs in this ten
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
    tenant_pd->ten_hw_id = INVALID_INDEXER_INDEX;

    tenant_pd->l2seg_hw_id_idxr_ = 
        new hal::utils::indexer(HAL_MAX_HW_L2SEGMENTS);
    HAL_ASSERT_RETURN((tenant_pd->l2seg_hw_id_idxr_ != NULL), NULL);

    // Prevention of usage of 0
    tenant_pd->l2seg_hw_id_idxr_->alloc_withid(0);

    return tenant_pd;
}

// allocate and initialize a tenant pd instance
static inline pd_tenant_t *
tenant_pd_alloc_init (void)
{
    return tenant_pd_init(tenant_pd_alloc());
}

// freeing tenant pd
static inline hal_ret_t
tenant_pd_free (pd_tenant_t *ten)
{
    ten->l2seg_hw_id_idxr_ ? delete ten->l2seg_hw_id_idxr_ : HAL_NOP;
    g_hal_state_pd->tenant_slab()->free(ten);
    return HAL_RET_OK;
}

// freeing tenant pd memory
static inline hal_ret_t
tenant_pd_mem_free (pd_tenant_t *ten)
{
    g_hal_state_pd->tenant_slab()->free(ten);
    return HAL_RET_OK;
}

static inline indexer *
tenant_pd_l2seg_hw_id_indexer(pd_tenant_t *tenant_pd) 
{
    return tenant_pd->l2seg_hw_id_idxr_;

}

hal_ret_t tenant_pd_alloc_res(pd_tenant_t *pd_ten);
hal_ret_t tenant_pd_dealloc_res(pd_tenant_t *tenant_pd);
hal_ret_t tenant_pd_cleanup(pd_tenant_t *tenant_pd);
void link_pi_pd(pd_tenant_t *pd_ten, tenant_t *pi_ten);
void delink_pi_pd(pd_tenant_t *pd_ten, tenant_t *pi_ten);

hal_ret_t tenant_pd_alloc_l2seg_hw_id(pd_tenant_t *tenant_pd, 
                                      uint32_t *l2seg_hw_id);
hal_ret_t tenant_pd_free_l2seg_hw_id(pd_tenant_t *tenant_pd, 
                                     uint32_t l2seg_hw_id);

}   // namespace pd
}   // namespace hal

#endif    // __HAL_PD_TENANT_HPP__
