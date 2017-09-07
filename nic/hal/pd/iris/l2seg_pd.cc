#include <hal_lock.hpp>
#include <pd_api.hpp>
#include <l2seg_pd.hpp>
#include <pd.hpp>
#include <hal_state_pd.hpp>
#include "if_pd_utils.hpp"

namespace hal {
namespace pd {

void *
l2seg_pd_get_hw_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((pd_l2seg_t *)entry)->hw_id);
}

uint32_t
l2seg_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return hal::utils::hash_algo::fnv_hash(key, sizeof(l2seg_hw_id_t)) % ht_size;
}

bool
l2seg_pd_compare_hw_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(l2seg_hw_id_t *)key1 == *(l2seg_hw_id_t *)key2) {
        return true;
    }
    return false;
}

l2seg_t *
find_l2seg_by_hwid (l2seg_hw_id_t hwid)
{
    pd_l2seg_t *l2seg_pd = find_l2seg_pd_by_hwid(hwid);
    return l2seg_pd ? (l2seg_t*) l2seg_pd->l2seg : NULL;
}

hal_ret_t
pd_l2seg_create (pd_l2seg_args_t *args)
{
    hal_ret_t               ret;
    indexer::status         rs;
    pd_l2seg_s              *l2seg_pd;

    HAL_TRACE_DEBUG("PD-L2Seg:{}: L2Seg Create ", __FUNCTION__);

    // allocate PD l2seg state
    l2seg_pd = l2seg_pd_alloc_init();
    if (l2seg_pd == NULL) {
        return HAL_RET_OOM;
    }
    l2seg_pd->l2seg = args->l2seg;

    // allocate h/w id for this L2 segment
    rs = g_hal_state_pd->l2seg_hwid_idxr()->alloc((uint32_t *)&l2seg_pd->hw_id);
    if (rs != indexer::SUCCESS) {
        g_hal_state_pd->l2seg_slab()->free(l2seg_pd);
        return HAL_RET_NO_RESOURCE;
    }
    if(l2seg_pd->hw_id == 0) {
        // TODO: remove this hack. p4 doesn't like 0 hwid
        HAL_TRACE_DEBUG("HACK ALERT: Allocating hwid again for l2seg");
        rs = g_hal_state_pd->l2seg_hwid_idxr()->alloc((uint32_t *)&l2seg_pd->hw_id);
        if (rs != indexer::SUCCESS) {
            g_hal_state_pd->l2seg_slab()->free(l2seg_pd);
            return HAL_RET_NO_RESOURCE;
        }
    }
    HAL_TRACE_DEBUG("PD-L2Seg:{}: L2Seg hwid: {} ", __FUNCTION__, l2seg_pd->hw_id);

    // add to db
    ret = add_l2seg_pd_to_db(l2seg_pd);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }
    args->l2seg->pd = l2seg_pd;

    return HAL_RET_OK;

cleanup:

    if (l2seg_pd) {
        g_hal_state_pd->l2seg_hwid_idxr()->free(l2seg_pd->hw_id);
        l2seg_pd_free(l2seg_pd);
    }
    return ret;
}

uint32_t
pd_l2seg_get_l4_prof_idx(pd_l2seg_t *pd_l2seg)
{
    l2seg_t         *pi_l2seg = NULL;
    tenant_t        *pi_tenant = NULL;

    pi_l2seg = (l2seg_t *)pd_l2seg->l2seg;
    HAL_ASSERT_RETURN(pi_l2seg != NULL, 0);

    pi_tenant = l2seg_get_pi_tenant(pi_l2seg);
    HAL_ASSERT_RETURN(pi_tenant != NULL, 0);

    return ten_get_nwsec_prof_hw_id(pi_tenant);
}

}    // namespace pd
}    // namespace hal
