#include <base.h>
#include <hal.hpp>
#include <hal_lock.hpp>
#include <hal_state.hpp>
#include <tenant_svc.hpp>
#include <tenant.hpp>
#include <pd_api.hpp>

namespace hal {

void *
tenant_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((tenant_t *)entry)->tenant_id);
}

uint32_t
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
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() != TenantKeyHandle::kTenantId) {
        // key-handle field set, but tenant id not provided
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // check if tenant id is in the valid range
    if (kh.tenant_id() == HAL_TENANT_ID_INVALID) {
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
    tenant_t                *tenant;
    pd::pd_tenant_args_t    pd_tenant_args;
    nwsec_profile_t         *sec_prof;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-Tenant:{}: Tenant Create for id {}", __FUNCTION__, 
                    spec.key_or_handle().tenant_id());
    // validate the request message
    ret = validate_tenant_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // instantiate a tenant
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
        sec_prof = find_nwsec_profile_by_handle(tenant->nwsec_profile_handle);
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
        tenant = find_tenant_by_id(kh.tenant_id());
    } else if (kh.key_or_handle_case() == TenantKeyHandle::kTenantHandle) {
        tenant = find_tenant_by_handle(kh.tenant_handle());
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
// process a tenant delete request
//------------------------------------------------------------------------------
hal_ret_t
tenant_delete (TenantDeleteRequest& req, TenantDeleteResponseMsg *rsp)
{
    return HAL_RET_OK;
}

}    // namespace hal
