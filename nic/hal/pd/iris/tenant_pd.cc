#include <hal_lock.hpp>
#include <pd_api.hpp>
#include <tenant_pd.hpp>

namespace hal {
namespace pd {

//-----------------------------------------------------------------------------
// PD Tenant Create
//-----------------------------------------------------------------------------
hal_ret_t
pd_tenant_create (pd_tenant_args_t *args)
{
    hal_ret_t               ret;
    pd_tenant_t             *tenant_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-tenant:{}:creating pd state for tenant {}",
                    __FUNCTION__, args->tenant->tenant_id);

    // allocate PD tenant state
    tenant_pd = tenant_pd_alloc_init();
    if (tenant_pd == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(tenant_pd, args->tenant);

    // allocate resources
    ret = tenant_pd_alloc_res(tenant_pd);

end:
    if (ret != HAL_RET_OK) {
        tenant_pd_cleanup(tenant_pd);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// PD Tenant Update
//-----------------------------------------------------------------------------
hal_ret_t
pd_tenant_update (pd_tenant_args_t *args)
{
    // Nothing to do for now
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// PD Tenant Delete
//-----------------------------------------------------------------------------
hal_ret_t
pd_tenant_delete (pd_tenant_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_tenant_t    *tenant_pd;

    HAL_ASSERT_RETURN((args != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->tenant != NULL), HAL_RET_INVALID_ARG);
    HAL_ASSERT_RETURN((args->tenant->pd != NULL), HAL_RET_INVALID_ARG);
    HAL_TRACE_DEBUG("pd-tenant:{}:Deleting pd state for tenant {}",
                    __FUNCTION__, args->tenant->tenant_id);
    tenant_pd = (pd_tenant_t *)args->tenant->pd;

    ret = tenant_pd_cleanup(tenant_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-tenant:{}:failed pd tenant delete",
                      __FUNCTION__);
    }

    return ret;
}

//-----------------------------------------------------------------------------
// Allocate resources. 
//-----------------------------------------------------------------------------
hal_ret_t
tenant_pd_alloc_res(pd_tenant_t *tenant_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;

    // allocate h/w id for this tenant
    rs = g_hal_state_pd->tenant_hwid_idxr()->
                         alloc((uint32_t *)&tenant_pd->ten_hw_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("pd-tenant:{}:failed to alloc ten_hw_id err: {}", 
                      __FUNCTION__, rs);
        tenant_pd->ten_hw_id = INVALID_INDEXER_INDEX;
        ret = HAL_RET_NO_RESOURCE;
        goto end;
    }

    HAL_TRACE_DEBUG("pd-tenant:{}:allocated ten_hw_id: {}", 
                    __FUNCTION__, tenant_pd->ten_hw_id);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// De-Allocate resources. 
//-----------------------------------------------------------------------------
hal_ret_t
tenant_pd_dealloc_res(pd_tenant_t *tenant_pd)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs;

    if (tenant_pd->ten_hw_id != INVALID_INDEXER_INDEX) {
        rs = g_hal_state_pd->tenant_hwid_idxr()->free(tenant_pd->ten_hw_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("pd-tenant:{}:failed to free ten_hw_id err: {}", 
                          __FUNCTION__, tenant_pd->ten_hw_id);
            ret = HAL_RET_INVALID_OP;
            goto end;
        }

        HAL_TRACE_DEBUG("pd-tenant:{}:freed ten_hw_id: {}", 
                        __FUNCTION__, tenant_pd->ten_hw_id);
    }

end:
    return ret;
}

//-----------------------------------------------------------------------------
// PD Tenant Cleanup
//  - Release all resources
//  - Delink PI <-> PD
//  - Free PD Tenant
//  Note:
//      - Just free up whatever PD has. 
//      - Dont use this inplace of delete. Delete may result in giving callbacks
//        to others.
//-----------------------------------------------------------------------------
hal_ret_t
tenant_pd_cleanup(pd_tenant_t *tenant_pd)
{
    hal_ret_t       ret = HAL_RET_OK;

    if (!tenant_pd) {
        // Nothing to do
        goto end;
    }

    // Check if l2segs have been removed before tenant cleanup
    if (!tenant_pd->l2seg_hw_id_idxr_->usage()) {
        HAL_TRACE_ERR("pd-tenant:{}:l2seg idxr still in use", __FUNCTION__);
        ret = HAL_RET_INVALID_OP;
        goto end;
    }

    // Releasing resources
    ret = tenant_pd_dealloc_res(tenant_pd);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pd-tenant:{}: unable to dealloc res for tenant: {}", 
                      __FUNCTION__, 
                      ((tenant_t *)(tenant_pd->tenant))->tenant_id);
        goto end;
    }

    // Delinking PI<->PD
    delink_pi_pd(tenant_pd, (tenant_t *)tenant_pd->tenant);

    // Freeing PD
    tenant_pd_free(tenant_pd);
end:
    return ret;
}

//-----------------------------------------------------------------------------
// Allocate l2seg hwid per tenant
//-----------------------------------------------------------------------------
hal_ret_t
tenant_pd_alloc_l2seg_hw_id(pd_tenant_t *tenant_pd, uint32_t *l2seg_hw_id)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs = indexer::SUCCESS;

    if (!tenant_pd || !l2seg_hw_id) {
        ret =  HAL_RET_INVALID_ARG;
        goto end;
    }

    rs = tenant_pd->l2seg_hw_id_idxr_->alloc(l2seg_hw_id);
    if (rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("pd-tenant:{}:failed to alloc l2seg_hw_id err: {}", 
                      __FUNCTION__, rs);
        *l2seg_hw_id = INVALID_INDEXER_INDEX;
        ret = HAL_RET_NO_RESOURCE;
        goto end;
    }

    HAL_TRACE_DEBUG("pd-tenant:{}:allocated l2seg_hw_id: {} for tenant: {}", 
                    __FUNCTION__, *l2seg_hw_id, 
                    ((tenant_t *)(tenant_pd->tenant))->tenant_id);

end:
    return ret;
}

//-----------------------------------------------------------------------------
// Free l2seg hwid per tenant
//-----------------------------------------------------------------------------
hal_ret_t
tenant_pd_free_l2seg_hw_id(pd_tenant_t *tenant_pd, uint32_t l2seg_hw_id)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     rs = indexer::SUCCESS;

    if (!tenant_pd) {
        ret =  HAL_RET_INVALID_ARG;
        goto end;
    }

    if (tenant_pd->ten_hw_id != INVALID_INDEXER_INDEX) { 
        rs = tenant_pd->l2seg_hw_id_idxr_->free(l2seg_hw_id);
        if (rs != indexer::SUCCESS) {
            HAL_TRACE_ERR("pd-tenant:{}:Failed to free l2seg_hw_id:{} "
                    "err: {}", __FUNCTION__,
                    l2seg_hw_id, rs);
            ret = HAL_RET_NO_RESOURCE;
            goto end;
        }
        HAL_TRACE_DEBUG("pd-tenant:{}:freed l2seg_hw_id: {} for tenant: {}", 
                        __FUNCTION__, l2seg_hw_id, 
                        ((tenant_t *)(tenant_pd->tenant))->tenant_id);
    }

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_tenant_t *pd_ten, tenant_t *pi_ten)
{
    pd_ten->tenant = pi_ten;
    pi_ten->pd = pd_ten;
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
delink_pi_pd(pd_tenant_t *pd_ten, tenant_t *pi_ten)
{
    if (pd_ten) {
        pd_ten->tenant = NULL;
    }
    if (pi_ten) {
        pi_ten->pd = NULL;
    }
}

// ----------------------------------------------------------------------------
// Makes a clone
// ----------------------------------------------------------------------------
hal_ret_t
pd_tenant_make_clone(tenant_t *ten, tenant_t *clone)
{
    hal_ret_t           ret = HAL_RET_OK;
    pd_tenant_t         *pd_ten_clone = NULL;

    pd_ten_clone = tenant_pd_alloc_init();
    if (pd_ten_clone == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    memcpy(pd_ten_clone, ten->pd, sizeof(pd_tenant_t));

    link_pi_pd(pd_ten_clone, clone);

end:
    return ret;
}

// ----------------------------------------------------------------------------
// Frees PD memory without indexer free.
// ----------------------------------------------------------------------------
hal_ret_t
pd_tenant_mem_free(pd_tenant_args_t *args)
{
    hal_ret_t      ret = HAL_RET_OK;
    pd_tenant_t    *tenant_pd;

    tenant_pd = (pd_tenant_t *)args->tenant->pd;
    tenant_pd_mem_free(tenant_pd);

    return ret;
}



}    // namespace pd
}    // namespace hal
