#include <hal_lock.hpp>
#include <pd_api.hpp>
#include <tenant_pd.hpp>

namespace hal {
namespace pd {

void *
tenant_pd_get_hw_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((pd_tenant_t *)entry)->hw_id);
}

uint32_t
tenant_pd_compute_hw_hash_func (void *key, uint32_t ht_size)
{
    return hal::utils::hash_algo::fnv_hash(key, sizeof(tenant_hw_id_t)) % ht_size;
}

bool
tenant_pd_compare_hw_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tenant_hw_id_t *)key1 == *(tenant_hw_id_t *)key2) {
        return true;
    }
    return false;
}

hal_ret_t
pd_tenant_create (pd_tenant_args_t *args)
{
    hal_ret_t               ret;
    indexer::status         rs;
    pd_tenant_t             *tenant_pd;

    HAL_TRACE_DEBUG("Creating pd state for tenant");

    // allocate PD tenant state
    tenant_pd = tenant_pd_alloc_init();
    if (tenant_pd == NULL) {
        return HAL_RET_OOM;
    }
    tenant_pd->tenant = args->tenant;

    // allocate h/w id for this tenant
    rs = g_hal_state_pd->tenant_hwid_idxr()->alloc((uint32_t *)&tenant_pd->hw_id);
    if (rs != indexer::SUCCESS) {
        g_hal_state_pd->tenant_slab()->free(tenant_pd);
        return HAL_RET_NO_RESOURCE;
    }

    HAL_TRACE_DEBUG("PD-Tenant:{}: Allocated ten_hw_id:{}", 
                    __FUNCTION__, tenant_pd->hw_id);

    // add to db
    ret = add_tenant_pd_to_db(tenant_pd);
    if (ret != HAL_RET_OK) {
        goto cleanup;
    }
    args->tenant->pd = tenant_pd;

    return HAL_RET_OK;

cleanup:

    if (tenant_pd) {
        g_hal_state_pd->tenant_hwid_idxr()->free(tenant_pd->hw_id);
        tenant_pd_free(tenant_pd);
    }
    return ret;
}

}    // namespace pd
}    // namespace hal
