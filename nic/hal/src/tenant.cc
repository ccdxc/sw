// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include <base.h>
#include <hal.hpp>
#include <hal_lock.hpp>
#include <hal_state.hpp>
#include <tenant_svc.hpp>
#include <tenant.hpp>
#include <pd_api.hpp>

namespace hal {

static inline hal_ret_t tenant_delete_cb(void *obj);

static inline void *
tenant_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((tenant_t *)entry)->tenant_id);
}

#if 0
static inline uint32_t
tenant_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(tenant_id_t)) % ht_size;
}

bool
tenant_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tenant_id_t *)key1 == *(tenant_id_t *)key2) {
        return true;
    }
    return false;
}
#endif

void *
tenant_id_get_key_func (void *entry)
{
    hal_handle_ht_entry_t    *ht_e;

    HAL_ASSERT(entry != NULL);
    ht_e = (hal_handle_ht_entry_t *)entry;
    return tenant_get_key_func(ht_e->handle->get_any_obj());
}

uint32_t
tenant_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return utils::hash_algo::fnv_hash(key, sizeof(tenant_id_t)) % ht_size;
}

bool
tenant_id_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(tenant_id_t *)key1 == *(tenant_id_t *)key2) {
        return true;
    }
    return false;
}

void *
tenant_get_handle_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((tenant_t *)entry)->hal_handle);
}

uint32_t
tenant_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
tenant_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
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
    hal_ret_t                ret;
    hal_handle_ht_entry_t    *entry;
    hal_handle               *hndl;

    // allocate an entry to establish mapping from tenant id to its handle
    entry =
        (hal_handle_ht_entry_t *)g_hal_state->hal_handle_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }
    hndl = reinterpret_cast<hal_handle *>(handle);

    // add mapping from tenant id to its handle
    entry->handle = hndl;
    ret = g_hal_state->tenant_id_ht()->insert_with_key(&tenant->tenant_id,
                                                       entry, &entry->ht_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add tenant id to handle mapping, "
                      "err : {}", ret);
        g_hal_state->hal_handle_ht_entry_slab()->free(entry);
    }

    // finally add the object to its handle
    hndl = reinterpret_cast<hal_handle *>(handle);
    ret = hndl->add_obj(tenant);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add tenant cfg to cfg db, err : {}", ret);
        g_hal_state->hal_handle_ht_entry_slab()->free(entry);
        return ret;
    }
    // establish the association between tenant and handle now
    tenant->hal_handle = handle;

    return ret;
}

//------------------------------------------------------------------------------
// delete a tenant from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
tenant_del_from_db (hal_handle_t handle, tenant_t *tenant,
                        hal_cfg_del_cb_t del_cb)
{
    hal_ret_t     ret;
    hal_handle    *hndl;

    hndl = reinterpret_cast<hal_handle *>(handle);
    ret = hndl->del_obj(tenant, del_cb);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to del tenant, id {}, err : {}",
                      tenant->tenant_id, ret);
        return ret;
    }

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
        HAL_TRACE_ERR("Tenant id and handle not set in request");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() != TenantKeyHandle::kTenantId) {
        // key-handle field set, but tenant id not provided
        HAL_TRACE_ERR("Tenant id not set in request");
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // check if tenant id is in the valid range
    if (kh.tenant_id() == HAL_TENANT_ID_INVALID) {
        HAL_TRACE_ERR("Tenant id {} invalid in the request",
                      HAL_TENANT_ID_INVALID);
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a tenant create request
// TODO: if tenant exists, treat this as modify
//------------------------------------------------------------------------------
hal_ret_t
tenant_create (TenantSpec& spec, TenantResponse *rsp)
{
    hal_ret_t               ret;
    tenant_t                *tenant = NULL;
    pd::pd_tenant_args_t    pd_tenant_args;
    nwsec_profile_t         *sec_prof;
    hal_handle_t            hal_handle = HAL_HANDLE_INVALID;

    HAL_TRACE_DEBUG("Creating tenant with id {}",
                    spec.key_or_handle().tenant_id());

    // validate the request message
    ret = validate_tenant_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Tenant request validation failed, ret : {}", ret);
        return ret;
    }

    // check if tenant exists already, and reject if one is found
    if (tenant_lookup_by_id(spec.key_or_handle().tenant_id())) {
        HAL_TRACE_ERR("Failed to create a tenant, tenant {} exists already",
                      spec.key_or_handle().tenant_id());
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        return HAL_RET_ENTRY_EXISTS;
    }

    // instantiate a PI tenant object
    tenant = tenant_alloc_init();
    if (tenant == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("Failed to create teanant, err : {}", ret);
        return HAL_RET_OOM;
    }
    tenant->tenant_id = spec.key_or_handle().tenant_id();
    tenant->nwsec_profile_handle = spec.security_profile_handle();
    if (tenant->nwsec_profile_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_DEBUG("Using default security profile");
        sec_prof = NULL;
    } else {
        sec_prof = nwsec_profile_lookup_by_handle(tenant->nwsec_profile_handle);
        if (sec_prof == NULL) {
            HAL_TRACE_ERR("Failed to create tenant, security profile with "
                          "handle {} not found", tenant->nwsec_profile_handle);
            rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_NOT_FOUND);
            tenant_free(tenant);
            return HAL_RET_SECURITY_PROFILE_NOT_FOUND;
        }
    }
    hal_handle = hal_handle_alloc();

    // add this tenant to our db
    ret = tenant_add_to_db(tenant, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add tenant {} to db, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_CFG_DB_ERR);
        goto error;
    }

    // allocate all PD resources and finish programming, if any
    pd::pd_tenant_args_init(&pd_tenant_args);
    pd_tenant_args.tenant = tenant;
    pd_tenant_args.nwsec_profile = sec_prof;
    ret = pd::pd_tenant_create(&pd_tenant_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to create tenant pd, err : {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto error;
    }

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_tenant_status()->set_tenant_handle(hal_handle);
    return HAL_RET_OK;

error:

    if (ret != HAL_RET_OK) {
        if (tenant->hal_handle != HAL_HANDLE_INVALID) {
            // (handle, object) are associated, we need to break the association
            tenant_del_from_db(tenant->hal_handle, tenant, tenant_delete_cb);
        } else {
            hal_handle_free(hal_handle);
            tenant_free(tenant);
        }
    }

    return ret;
}

#if 0
//------------------------------------------------------------------------------
// process a tenant create request
// TODO: if tenant exists, treat this as modify
//------------------------------------------------------------------------------
hal_ret_t
tenant_create (TenantSpec& spec, TenantResponse *rsp)
{
    hal_ret_t               ret;
    tenant_t                *tenant = NULL;
    pd::pd_tenant_args_t    pd_tenant_args;
    nwsec_profile_t         *sec_prof;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-Tenant:{}: Tenant Create for id {}", __FUNCTION__, 
                    spec.key_or_handle().tenant_id());
    // validate the request message
    ret = validate_tenant_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        return ret;
    }

    // check if tenant exists already, and reject if one is found
    if (tenant_lookup_by_id(spec.key_or_handle().tenant_id())) {
        rsp->set_api_status(types::API_STATUS_EXISTS_ALREADY);
        return HAL_RET_ENTRY_EXISTS;
    }

    // instantiate a PI tenant object
    tenant = tenant_alloc_init();
    if (tenant == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("PI-Tenant:{}: Out of Memory. Err: {}", 
                      ret);
        goto end;
    }
    tenant->tenant_id = spec.key_or_handle().tenant_id();
    tenant->nwsec_profile_handle = spec.security_profile_handle();
    if (tenant->nwsec_profile_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_DEBUG("Using default security profile");
        sec_prof = NULL;
    } else {
        sec_prof = nwsec_profile_lookup_by_handle(tenant->nwsec_profile_handle);
        if (sec_prof == NULL) {
            HAL_TRACE_ERR("Security profile with handle not found {}",
                          tenant->nwsec_profile_handle);
            ret = HAL_RET_SECURITY_PROFILE_NOT_FOUND;
            rsp->set_api_status(types::API_STATUS_NWSEC_PROFILE_NOT_FOUND);
            HAL_TRACE_ERR("PI-Tenant:{}: Sec. Profile not found. Err: {}", 
                          ret);
            goto end;
        }
    }
    tenant->hal_handle = hal_alloc_handle();

    // allocate all PD resources and finish programming, if any
    pd::pd_tenant_args_init(&pd_tenant_args);
    pd_tenant_args.tenant = tenant;
    pd_tenant_args.nwsec_profile = sec_prof;
    ret = pd::pd_tenant_create(&pd_tenant_args);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PI-Tenant:{}: PD Programming fail Err: {}", ret);
        rsp->set_api_status(types::API_STATUS_HW_PROG_ERR);
        goto end;
    }

    // add this tenant to our db
    ret = add_tenant_to_db(tenant);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_tenant_status()->set_tenant_handle(tenant->hal_handle);

end:

    
    if (ret != HAL_RET_OK && tenant != NULL) {
        tenant_free(tenant);
    }
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}
#endif

//------------------------------------------------------------------------------
// process a tenant update request
//------------------------------------------------------------------------------
hal_ret_t
tenant_update (TenantSpec& spec, TenantResponse *rsp)
{
    return HAL_RET_OK;
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

#if 0
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
#endif

//------------------------------------------------------------------------------
// tenant delete callback function, this will be invoked when no other thread is
// using this object once tenant delete API is called
// NOTE: hal handle allocated for this object shouldn't be explicitly deleted
//------------------------------------------------------------------------------
static inline hal_ret_t
tenant_delete_cb (void *obj)
{
    hal_ret_t                ret;
    tenant_t                 *tenant = (tenant_t *)obj;
    hal_handle_ht_entry_t    *entry;
    pd::pd_tenant_args_t     pd_tenant_args;

    if (tenant == NULL) {
        HAL_TRACE_ERR("Invalid tenant object passed for deletion");
        return HAL_RET_INVALID_ARG;
    }
    HAL_TRACE_DEBUG("Started Tenant {} deletion", tenant->tenant_id);

    // delete this tenant from all the meta data structures
    entry = (hal_handle_ht_entry_t *)
                g_hal_state->tenant_id_ht()->remove(&tenant->tenant_id);
    if (entry == NULL) {
        HAL_TRACE_ERR("Failed to find tenant tenant-id HT, tenant id {}",
                      tenant->tenant_id);
        // still go ahead and cleanup whatever we can !!!
    } else {
        g_hal_state->hal_handle_ht_entry_slab()->free(entry);
    }

    // do the actual cleanup for this tenant object now
    if (tenant->pd) {
        pd::pd_tenant_args_init(&pd_tenant_args);
        pd_tenant_args.tenant = tenant;
        ret = pd::pd_tenant_delete(&pd_tenant_args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_ERR("Failed to delete tenant pd, err : {}", ret);
        }
    }
    tenant_free(tenant);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a tenant delete request
//------------------------------------------------------------------------------
hal_ret_t
tenant_delete (TenantDeleteRequest& req, TenantDeleteResponseMsg *rsp)
{
    tenant_t     *tenant;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        rsp->add_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    // lookup the tenant object
    auto kh = req.key_or_handle();
    if (kh.key_or_handle_case() == TenantKeyHandle::kTenantId) {
        tenant = tenant_lookup_by_id(kh.tenant_id());
    } else if (kh.key_or_handle_case() == TenantKeyHandle::kTenantHandle) {
        tenant = tenant_lookup_by_handle(kh.tenant_handle());
    } else {
        rsp->add_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    // check if we found the tenant of interest
    if (tenant == NULL) {
        HAL_TRACE_ERR("Failed to delete tenant, id {}, handle {}",
                      kh.tenant_id(), kh.tenant_handle());
        rsp->add_api_status(types::API_STATUS_TENANT_NOT_FOUND);
        return HAL_RET_TENANT_NOT_FOUND;
    }
    HAL_TRACE_DEBUG("Deleting tenant {} ...", tenant->tenant_id);
    tenant_del_from_db(tenant->hal_handle, tenant, tenant_delete_cb);
    HAL_TRACE_DEBUG("Initiated tenant {} delete ...", tenant->tenant_id);

    rsp->add_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

}    // namespace hal
