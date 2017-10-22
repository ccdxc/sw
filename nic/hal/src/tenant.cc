// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/tenant.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/utils.hpp"
#include "nic/hal/src/if_utils.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// hash table tenant_id => entry
//  - Get key from entry
// ----------------------------------------------------------------------------
void *
tenant_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    tenant_t                    *tenant = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    tenant = (tenant_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(tenant->tenant_id);
}

// ----------------------------------------------------------------------------
// hash table tenant_id => entry - compute hash
// ----------------------------------------------------------------------------
uint32_t
tenant_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return utils::hash_algo::fnv_hash(key, sizeof(tenant_id_t)) % ht_size;
}

// ----------------------------------------------------------------------------
// hash table tenant_id => entry - compare function
// ----------------------------------------------------------------------------
bool
tenant_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tenant_id_t *)key1 == *(tenant_id_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert a tenant to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
tenant_add_to_db (tenant_t *tenant, hal_handle_t handle)
{
    hal_ret_t                   ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-tenant:{}:adding to tenant id hash table", 
                    __FUNCTION__);
    // allocate an entry to establish mapping from tenant id to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from tenant id to its handle
    entry->handle_id = handle;
    ret = g_hal_state->tenant_id_ht()->insert_with_key(&tenant->tenant_id,
                                                       entry, &entry->ht_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to add tenant id to handle mapping, "
                      "err : {}", __FUNCTION__, ret);
        g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);
    }

    // TODO: Check if this is the right place
    tenant->hal_handle = handle;

    return ret;
}

//------------------------------------------------------------------------------
// delete a tenant from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
tenant_del_from_db (tenant_t *tenant)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-tenant:{}:removing from tenant id hash table", __FUNCTION__);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->tenant_id_ht()->
        remove(&tenant->tenant_id);

    // free up
    g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate an incoming tenant create request
// TODO:
// 1. check if tenant exists
// 2. validate L4 profile existence if that handle is valid
//------------------------------------------------------------------------------
static hal_ret_t
validate_tenant_create (TenantSpec& spec, TenantResponse *rsp)
{
    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-tenant:{}:tenant id and handle not set in request",
                      __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() != TenantKeyHandle::kTenantId) {
        // key-handle field set, but tenant id not provided
        HAL_TRACE_ERR("pi-tenant:{}:tenant id not set in request", __FUNCTION__);
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // check if tenant id is in the valid range
    if (kh.tenant_id() == HAL_TENANT_ID_INVALID) {
        HAL_TRACE_ERR("pi-tenant:{}:tenant id {} invalid in the request", __FUNCTION__,
                      HAL_TENANT_ID_INVALID);
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}


//------------------------------------------------------------------------------
// PD Call to allocate PD resources and HW programming
//------------------------------------------------------------------------------
hal_ret_t
tenant_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_tenant_args_t        pd_tenant_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    tenant_t                    *tenant = NULL;
    tenant_create_app_ctxt_t    *app_ctxt = NULL; 

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-tenant:{}: invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (tenant_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    tenant = (tenant_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-tenant:{}:create add CB {}",
                    __FUNCTION__, tenant->tenant_id);

    // PD Call to allocate PD resources and HW programming
    pd::pd_tenant_args_init(&pd_tenant_args);
    pd_tenant_args.tenant = tenant;
    pd_tenant_args.nwsec_profile = app_ctxt->sec_prof;
    ret = pd::pd_tenant_create(&pd_tenant_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to create tenant pd, err : {}", 
                __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// 1. Update PI DBs as tenant_create_add_cb() was a success
//      a. Add to tenant id hash table
//------------------------------------------------------------------------------
hal_ret_t
tenant_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    tenant_t                    *tenant = NULL;
    hal_handle_t                hal_handle = 0;
    tenant_create_app_ctxt_t    *app_ctxt = NULL; 

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-tenant:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (tenant_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    tenant = (tenant_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-tenant:{}:create commit CB {}",
                    __FUNCTION__, tenant->tenant_id);

    // 1. a. Add to tenant id hash table
    ret = tenant_add_to_db(tenant, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to add tenant {} to db, err : {}", 
                __FUNCTION__, tenant->tenant_id, ret);
        goto end;
    }

    // Add tenant to nwsec profile
    if (tenant->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        ret = nwsec_prof_add_tenant(app_ctxt->sec_prof, tenant);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-tenant:{}:failed to add rel. from nwsec. prof",
                          __FUNCTION__);
            goto end;
        }
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// tenant_create_add_cb was a failure
// 1. call delete to PD
//      a. Deprogram HW
//      b. Clean up resources
//      c. Free PD object
// 2. Remove object from hal_handle id based hash table in infra
// 3. Free PI tenant 
//------------------------------------------------------------------------------
hal_ret_t
tenant_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_tenant_args_t        pd_tenant_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    tenant_t                    *tenant = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-tenant:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    tenant = (tenant_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-tenant:{}:create abort CB {}",
                    __FUNCTION__, tenant->tenant_id);

    // 1. delete call to PD
    if (tenant->pd) {
        pd::pd_tenant_args_init(&pd_tenant_args);
        pd_tenant_args.tenant = tenant;
        ret = pd::pd_tenant_delete(&pd_tenant_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-tenant:{}:failed to delete tenant pd, err : {}", 
                          __FUNCTION__, ret);
        }
    }

    // 2. remove object from hal_handle id based hash table in infra
    hal_handle_free(hal_handle);

    // 3. Free PI tenant
    tenant_free(tenant);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
tenant_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
tenant_prepare_rsp (TenantResponse *rsp, hal_ret_t ret, hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_tenant_status()->set_tenant_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a tenant create request
// TODO: if tenant exists, treat this as modify
//------------------------------------------------------------------------------
hal_ret_t
tenant_create (TenantSpec& spec, TenantResponse *rsp)
{
    hal_ret_t                   ret;
    tenant_t                    *tenant = NULL;
    nwsec_profile_t             *sec_prof;
    hal_handle_t                hal_handle = HAL_HANDLE_INVALID;
    tenant_create_app_ctxt_t    app_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    cfg_op_ctxt_t               cfg_ctxt = { 0 };

    hal_api_trace(" API Begin: tenant create ");
    HAL_TRACE_DEBUG("pi-tenant:{}:creating tenant with id {}",
                    __FUNCTION__, spec.key_or_handle().tenant_id());

    // validate the request message
    ret = validate_tenant_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:validation failed, ret : {}", 
                __FUNCTION__, ret);
        goto end;
    }

    // check if tenant exists already, and reject if one is found
    if (tenant_lookup_by_id(spec.key_or_handle().tenant_id())) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to create a tenant, "
                      "tenant {} exists already", __FUNCTION__, 
                      spec.key_or_handle().tenant_id());
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    // instantiate a PI tenant object
    tenant = tenant_alloc_init();
    if (tenant == NULL) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to create teanant, err : {}", ret);
        ret = HAL_RET_OOM;
        goto end;
    }
    tenant->tenant_id = spec.key_or_handle().tenant_id();
    tenant->nwsec_profile_handle = spec.security_profile_handle();
    if (tenant->nwsec_profile_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_DEBUG("pi-tenant:{}: No nwsec prof passed, "
                        "using default security profile", __FUNCTION__);
        sec_prof = NULL;
    } else {
        sec_prof = find_nwsec_profile_by_handle(tenant->nwsec_profile_handle);
        if (sec_prof == NULL) {
            HAL_TRACE_ERR("pi-tenant:{}:Failed to create tenant, "
                          "security profile with handle {} not found", 
                          __FUNCTION__, tenant->nwsec_profile_handle);
            rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_NOT_FOUND);
            tenant_free(tenant);
            ret = HAL_RET_SECURITY_PROFILE_NOT_FOUND;
            goto end;
        }
    }

    // allocate hal handle id
    hal_handle = hal_handle_alloc(HAL_OBJ_ID_TENANT);
    if (hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-tenant:{}: failed to alloc handle {}", 
                      __FUNCTION__, tenant->tenant_id);
        rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
        tenant_free(tenant);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    // form ctxt and call infra add
    app_ctxt.sec_prof = sec_prof;
    dhl_entry.handle = hal_handle;
    dhl_entry.obj = tenant;
    cfg_ctxt.app_ctxt = &app_ctxt;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(hal_handle, &cfg_ctxt, 
                             tenant_create_add_cb,
                             tenant_create_commit_cb,
                             tenant_create_abort_cb, 
                             tenant_create_cleanup_cb);


end:
    if (ret != HAL_RET_OK) {
        if (tenant) {
            // if there is an error, if will be freed in abort CB
            tenant = NULL;
        }
    }

    tenant_prepare_rsp(rsp, ret, 
                      tenant ? tenant->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: tenant create ");
    return ret;
}


//------------------------------------------------------------------------------
// handling nwsec update
//------------------------------------------------------------------------------
hal_ret_t
tenant_handle_nwsec_update (tenant_t *tenant, nwsec_profile_t *nwsec_prof)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    hal_handle_id_list_entry_t  *entry = NULL;
    l2seg_t                     *l2seg = NULL;

    if (tenant == NULL) {
        return ret;
    }

    HAL_TRACE_DEBUG("pi-tenant:{}:tenant_id: {}", 
                    __FUNCTION__, tenant->tenant_id);
    // Walk L2 segs
    dllist_for_each(lnode, &tenant->l2seg_list_head) {
        entry = dllist_entry(lnode, hal_handle_id_list_entry_t, dllist_ctxt);
        l2seg = find_l2seg_by_handle(entry->handle_id);
        if (!l2seg) {
            HAL_TRACE_ERR("pi-tenant:{}:unable to find l2seg with handle:{}",
                          __FUNCTION__, entry->handle_id);
            continue;
        }
        l2seg_handle_nwsec_update(l2seg, nwsec_prof);
    }

    return ret;
}


//------------------------------------------------------------------------------
// validate tenant update request
//------------------------------------------------------------------------------
hal_ret_t
validate_tenant_update (TenantSpec& spec, TenantResponse *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-tenant:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// check if nwsec profile changed for tenant
//------------------------------------------------------------------------------
hal_ret_t
tenant_nwsec_update (TenantSpec& spec, tenant_t *tenant, bool *nwsec_change,
                     hal_handle_t *new_nwsec_handle)
{
    *nwsec_change = false;
    if (tenant->nwsec_profile_handle != spec.security_profile_handle()) {
        HAL_TRACE_DEBUG("pi-tenant:{}:nwSec profile updated", __FUNCTION__);
        *nwsec_change = true;
        *new_nwsec_handle = spec.security_profile_handle();
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// This is the first call back infra does for update.
// 1. PD Call to update PD
// 2. Update Other objects to update with new nwsec profile
//------------------------------------------------------------------------------
hal_ret_t
tenant_update_upd_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_tenant_args_t        pd_tenant_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    tenant_t                    *tenant = NULL;
    tenant_update_app_ctxt_t    *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-tenant{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (tenant_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    tenant = (tenant_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-tenant:{}:update upd CB {}",
                    __FUNCTION__, tenant->tenant_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_tenant_args_init(&pd_tenant_args);
    pd_tenant_args.tenant = tenant;
    pd_tenant_args.nwsec_profile = app_ctxt->nwsec_prof;
    ret = pd::pd_tenant_update(&pd_tenant_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to update tenant pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Pass the tenant(old/new is fine) and new nwsec profile
    if (app_ctxt->nwsec_prof_change) {
        // Triggers reprogramming of input properties table for enicifs and uplinks
        tenant_handle_nwsec_update(tenant, app_ctxt->nwsec_prof);
    }

end:
    return ret;
}


//------------------------------------------------------------------------------
// Make a clone
// - Both PI and PD objects cloned. 
//------------------------------------------------------------------------------
hal_ret_t
tenant_make_clone (tenant_t *ten, tenant_t **ten_clone)
{
    *ten_clone = tenant_alloc_init();
    memcpy(*ten_clone, ten, sizeof(tenant_t));

    // After clone always reset lists
    dllist_reset(&(*ten_clone)->l2seg_list_head);
    dllist_reset(&(*ten_clone)->ep_list_head);
    dllist_reset(&(*ten_clone)->session_list_head);

    pd::pd_tenant_make_clone(ten, *ten_clone);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// After all hw programming is done
//  1. Free original PI & PD tenant.
// Note: Infra make clone as original by replacing original pointer by clone.
//------------------------------------------------------------------------------
hal_ret_t
tenant_update_commit_cb(cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_tenant_args_t        pd_tenant_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    tenant_t                    *tenant = NULL, *tenant_clone = NULL;
    tenant_update_app_ctxt_t    *app_ctxt = NULL;
    nwsec_profile_t             *nwsec_prof = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-tenant{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    app_ctxt = (tenant_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    tenant = (tenant_t *)dhl_entry->obj;
    tenant_clone = (tenant_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-tenant:{}:update commit CB {}",
                    __FUNCTION__, tenant->tenant_id);

    // move lists to clone
    dllist_move(&tenant_clone->l2seg_list_head, &tenant->l2seg_list_head);

    // update clone with new attrs
    if (app_ctxt->nwsec_prof_change) {
        tenant_clone->nwsec_profile_handle = app_ctxt->nwsec_profile_handle;

        if (tenant->nwsec_profile_handle != HAL_HANDLE_INVALID) {
            // detach from old nwsec prof
            nwsec_prof = find_nwsec_profile_by_handle(
                         tenant->nwsec_profile_handle);
            if (nwsec_prof == NULL) {
                HAL_TRACE_ERR("pi-tenant:{}:unable to find nwsec prof "
                        "with hdl:{}",
                        __FUNCTION__, tenant->nwsec_profile_handle);
                goto end;
            }
            ret = nwsec_prof_del_tenant(nwsec_prof, tenant);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pi-tenant:{}:unable to detach old nwsec prof",
                        __FUNCTION__);
                goto end;
            }
        }

        if (tenant_clone->nwsec_profile_handle != HAL_HANDLE_INVALID) {
            // attach to new nwsec prof
            ret = nwsec_prof_add_tenant(app_ctxt->nwsec_prof, tenant_clone);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_ERR("pi-tenant:{}:unable to attach new nwsec prof",
                        __FUNCTION__);
                goto end;
            }
        }
    }

    // Free PD
    pd::pd_tenant_args_init(&pd_tenant_args);
    pd_tenant_args.tenant = tenant;
    ret = pd::pd_tenant_mem_free(&pd_tenant_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to delete tenant pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    tenant_free(tenant);
end:
    return ret;
}

//------------------------------------------------------------------------------
// Update didnt go through.
//  1. Kill the clones
//------------------------------------------------------------------------------
hal_ret_t
tenant_update_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_tenant_args_t        pd_tenant_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    tenant_t                    *tenant = NULL;
    // tenant_update_app_ctxt_t    *app_ctxt = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-tenant{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (tenant_update_app_ctxt_t *)cfg_ctxt->app_ctxt;

    tenant = (tenant_t *)dhl_entry->cloned_obj;

    HAL_TRACE_DEBUG("pi-tenant:{}:update commit CB {}",
                    __FUNCTION__, tenant->tenant_id);

    // Free PD
    pd::pd_tenant_args_init(&pd_tenant_args);
    pd_tenant_args.tenant = tenant;
    ret = pd::pd_tenant_mem_free(&pd_tenant_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to delete tenant pd, err : {}",
                      __FUNCTION__, ret);
    }

    // Free PI
    tenant_free(tenant);
end:

    return ret;
}

hal_ret_t
tenant_update_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a tenant update request
//------------------------------------------------------------------------------
hal_ret_t
tenant_update (TenantSpec& spec, TenantResponse *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    tenant_t                    *tenant = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    const TenantKeyHandle       &kh = spec.key_or_handle();
    tenant_update_app_ctxt_t    app_ctxt = { 0 };

    hal_api_trace(" API Begin: tenant update");

    // validate the request message
    ret = validate_tenant_update(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:tenant delete validation failed, ret : {}", 
                      __FUNCTION__, ret);
        goto end;
    }

    tenant = tenant_lookup_key_or_handle(kh);
    if (tenant == NULL) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to find tenant, id {}, handle {}",
                      __FUNCTION__, kh.tenant_id(), kh.tenant_handle());
        ret = HAL_RET_TENANT_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("pi-tenant:{}:update tenant {}", __FUNCTION__, 
                    tenant->tenant_id);

    ret = tenant_nwsec_update(spec, tenant, &app_ctxt.nwsec_prof_change, 
                              &app_ctxt.nwsec_profile_handle);

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to check if nwsec is updated.", 
                      __FUNCTION__);
        goto end;
    }

    if (!app_ctxt.nwsec_prof_change) {
        HAL_TRACE_ERR("pi-tenant:{}:no change in tenant update: noop", __FUNCTION__);
        // Its a no-op. We can just return HAL_RET_OK
        // ret = HAL_RET_INVALID_OP;
        goto end;
    }

    if (app_ctxt.nwsec_profile_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_DEBUG("pi-tenant:{}: No nwsec prof passed, "
                        "using default security profile", __FUNCTION__);
        app_ctxt.nwsec_prof = NULL;
    } else {
        app_ctxt.nwsec_prof = find_nwsec_profile_by_handle(app_ctxt.nwsec_profile_handle);
        if (app_ctxt.nwsec_prof == NULL) {
            HAL_TRACE_ERR("pi-tenant:{}:security profile with handle {} not found", 
                    __FUNCTION__, 
                    app_ctxt.nwsec_profile_handle);
            ret = HAL_RET_SECURITY_PROFILE_NOT_FOUND;
            goto end;
        } else {
            HAL_TRACE_DEBUG("pi-tenant:{}:new nwsec profile id: {}", __FUNCTION__, 
                    app_ctxt.nwsec_prof->profile_id);
        }
    }

    tenant_make_clone(tenant, (tenant_t **)&dhl_entry.cloned_obj);

    // form ctxt and call infra update object
    dhl_entry.handle = tenant->hal_handle;
    dhl_entry.obj = tenant;
    cfg_ctxt.app_ctxt = &app_ctxt;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_upd_obj(tenant->hal_handle, &cfg_ctxt, 
                             tenant_update_upd_cb,
                             tenant_update_commit_cb,
                             tenant_update_abort_cb, 
                             tenant_update_cleanup_cb);

end:
    tenant_prepare_rsp(rsp, ret, 
                       tenant ? tenant->hal_handle : HAL_HANDLE_INVALID);
    hal_api_trace(" API End: tenant update ");
    return ret;
}

//------------------------------------------------------------------------------
// process a tenant get request
//------------------------------------------------------------------------------
hal_ret_t
tenant_get (TenantGetRequest& req, TenantGetResponse *rsp)
{
    tenant_t     *tenant;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = req.key_or_handle();
    if (kh.key_or_handle_case() == TenantKeyHandle::kTenantId) {
        tenant = tenant_lookup_by_id(kh.tenant_id());
    } else if (kh.key_or_handle_case() == TenantKeyHandle::kTenantHandle) {
        tenant = tenant_lookup_by_handle(kh.tenant_handle());
    } else {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    if (tenant == NULL) {
        rsp->set_api_status(types::API_STATUS_TENANT_NOT_FOUND);
        return HAL_RET_TENANT_NOT_FOUND;
    }

    // fill config spec of this tenant
    rsp->mutable_spec()->mutable_meta()->set_tenant_id(tenant->tenant_id);
    rsp->mutable_spec()->mutable_key_or_handle()->set_tenant_id(tenant->tenant_id);
    rsp->mutable_spec()->set_security_profile_handle(tenant->nwsec_profile_handle);

    // fill operational state of this tenant
    rsp->mutable_status()->set_tenant_handle(tenant->hal_handle);

    // fill stats of this tenant
    rsp->mutable_stats()->set_num_l2_segments(tenant->num_l2seg);
    rsp->mutable_stats()->set_num_security_groups(tenant->num_sg);
    rsp->mutable_stats()->set_num_l4lb_services(tenant->num_l4lb_svc);
    rsp->mutable_stats()->set_num_endpoints(tenant->num_ep);

    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate tenant delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_tenant_delete_req (TenantDeleteRequest& req, TenantDeleteResponseMsg *rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-tenant:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// Lookup tenant from key or handle
//------------------------------------------------------------------------------
tenant_t *
tenant_lookup_key_or_handle (const TenantKeyHandle& kh)
{
    tenant_t     *tenant = NULL;

    if (kh.key_or_handle_case() == TenantKeyHandle::kTenantId) {
        tenant = tenant_lookup_by_id(kh.tenant_id());
    } else if (kh.key_or_handle_case() == TenantKeyHandle::kTenantHandle) {
        tenant = tenant_lookup_by_handle(kh.tenant_handle());
    }

    return tenant;
}

//------------------------------------------------------------------------------
// 1. PD Call to delete PD and free up resources and deprogram HW
//------------------------------------------------------------------------------
hal_ret_t
tenant_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    pd::pd_tenant_args_t        pd_tenant_args = { 0 };
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    tenant_t                    *tenant = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-tenant:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // TODO: Check the dependency ref count for the tenant. 
    //       If its non zero, fail the delete.


    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    tenant = (tenant_t *)dhl_entry->obj;

    HAL_TRACE_DEBUG("pi-tenant:{}:delete del CB {}",
                    __FUNCTION__, tenant->tenant_id);

    // 1. PD Call to allocate PD resources and HW programming
    pd::pd_tenant_args_init(&pd_tenant_args);
    pd_tenant_args.tenant = tenant;
    ret = pd::pd_tenant_delete(&pd_tenant_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to delete tenant pd, err : {}", 
                      __FUNCTION__, ret);
    }

end:
    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as tenant_delete_del_cb() was a succcess
//      a. Delete from tenant id hash table
//      b. Remove object from handle id based hash table
//      c. Free PI tenant
//------------------------------------------------------------------------------
hal_ret_t
tenant_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    tenant_t                    *tenant = NULL;
    hal_handle_t                hal_handle = 0;
    nwsec_profile_t             *sec_prof = NULL;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-tenant:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    tenant = (tenant_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-tenant:{}:delete commit CB {}",
                    __FUNCTION__, tenant->tenant_id);

    // Remove tenant references from other objects
    if (tenant->nwsec_profile_handle != HAL_HANDLE_INVALID) {
        sec_prof = find_nwsec_profile_by_handle(tenant->nwsec_profile_handle);
        ret = nwsec_prof_del_tenant(sec_prof, tenant);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("pi-tenant:{}:failed to del rel. from nwsec. prof",
                          __FUNCTION__);
            goto end;
        }

    }

    // a. Remove from tenant id hash table
    ret = tenant_del_from_db(tenant);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to del tenant {} from db, err : {}", 
                      __FUNCTION__, tenant->tenant_id, ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI tenant
    tenant_free(tenant);

end:
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
tenant_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
tenant_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// validate tenant delete request
//------------------------------------------------------------------------------
hal_ret_t
validate_tenant_delete (tenant_t *tenant)
{
    hal_ret_t   ret = HAL_RET_OK;

    // check for no presence of l2segs
    if (dllist_count(&tenant->l2seg_list_head)) {
        ret = HAL_RET_OBJECT_IN_USE;
        HAL_TRACE_ERR("pi-tenant:{}:l2segs still referring:", __FUNCTION__);
        hal_print_handles_list(&tenant->l2seg_list_head);
    }

    return ret;
}

//------------------------------------------------------------------------------
// process a tenant delete request
//------------------------------------------------------------------------------
hal_ret_t
tenant_delete (TenantDeleteRequest& req, TenantDeleteResponseMsg *rsp)
{
    hal_ret_t                   ret = HAL_RET_OK;
    tenant_t                    *tenant = NULL;
    cfg_op_ctxt_t               cfg_ctxt = { 0 };
    dhl_entry_t                 dhl_entry = { 0 };
    const TenantKeyHandle       &kh = req.key_or_handle();

    hal_api_trace(" API Begin: tenant delete ");

    // validate the request message
    ret = validate_tenant_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:tenant delete validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

    tenant = tenant_lookup_key_or_handle(kh);
    if (tenant == NULL) {
        HAL_TRACE_ERR("pi-tenant:{}:failed to find tenant, id {}, handle {}",
                      __FUNCTION__, kh.tenant_id(), kh.tenant_handle());
        ret = HAL_RET_TENANT_NOT_FOUND;
        goto end;
    }

    HAL_TRACE_DEBUG("pi-tenant:{}:deleting tenant {}", 
                    __FUNCTION__, tenant->tenant_id);

    // validate if there no objects referring this sec. profile
    ret = validate_tenant_delete(tenant);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-tenant:{}:tenant delete validation failed, "
                      "ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

    // form ctxt and call infra add
    dhl_entry.handle = tenant->hal_handle;
    dhl_entry.obj = tenant;
    cfg_ctxt.app_ctxt = NULL;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(tenant->hal_handle, &cfg_ctxt, 
                             tenant_delete_del_cb,
                             tenant_delete_commit_cb,
                             tenant_delete_abort_cb, 
                             tenant_delete_cleanup_cb);

end:
    rsp->add_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: tenant delete ");
    return ret;
}


//-----------------------------------------------------------------------------
// Adds l2seg into tenant list
//-----------------------------------------------------------------------------
hal_ret_t
tenant_add_l2seg (tenant_t *tenant, l2seg_t *l2seg)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_list_entry_t  *entry = NULL;

    if (tenant == NULL || l2seg == NULL) {
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // Allocate the entry
    entry = (hal_handle_id_list_entry_t *)g_hal_state->
        hal_handle_id_list_entry_slab()->alloc();
    if (entry == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }
    entry->handle_id = l2seg->hal_handle;

    tenant_lock(tenant, __FILENAME__, __LINE__, __func__);      // lock
    // Insert into the list
    utils::dllist_add(&tenant->l2seg_list_head, &entry->dllist_ctxt);
    tenant_unlock(tenant, __FILENAME__, __LINE__, __func__);    // unlock

end:
    HAL_TRACE_DEBUG("pi-tenant:{}:add tenant => l2seg, {} => {}, ret:{}",
                    __FUNCTION__, tenant->tenant_id, l2seg->seg_id, ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove l2seg from tenant list
//-----------------------------------------------------------------------------
hal_ret_t
tenant_del_l2seg (tenant_t *tenant, l2seg_t *l2seg)
{
    hal_ret_t                   ret = HAL_RET_IF_NOT_FOUND;
    hal_handle_id_list_entry_t  *entry = NULL;
    dllist_ctxt_t               *curr, *next;

    tenant_lock(tenant, __FILENAME__, __LINE__, __func__);      // lock
    dllist_for_each_safe(curr, next, &tenant->l2seg_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == l2seg->hal_handle) {
            // Remove from list
            utils::dllist_del(&entry->dllist_ctxt);
            // Free the entry
            g_hal_state->hal_handle_id_list_entry_slab()->free(entry);
            ret = HAL_RET_OK;
        }
    }
    tenant_unlock(tenant, __FILENAME__, __LINE__, __func__);    // unlock

    HAL_TRACE_DEBUG("pi-tenant:{}:del tenant =/=> l2seg, {} =/=> {}, ret:{}",
                    __FUNCTION__, tenant->tenant_id, l2seg->seg_id, ret);
    return ret;
}

}    // namespace hal
