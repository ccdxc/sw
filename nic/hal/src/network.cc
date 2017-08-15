#include <base.h>
#include <hal.hpp>
#include <hal_lock.hpp>
#include <hal_state.hpp>
#include <utils.hpp>
#include <network_svc.hpp>
#include <network.hpp>
#include <pd_api.hpp>

namespace hal {

void *
network_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((network_t *)entry)->nw_key);
}

uint32_t
network_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(network_key_t)) % ht_size;
}

bool
network_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(network_key_t))) {
        return true;
    }
    return false;
}

void *
network_get_handle_key_func(void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((network_t *)entry)->hal_handle);
}

uint32_t
network_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
network_compare_handle_key_func (void *key1, void *key2)
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
validate_network_create (NetworkSpec& spec, NetworkResponse *rsp)
{
    // key-handle field must be set
    if (!spec.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    if (!spec.has_meta() ||
        spec.meta().tenant_id() == HAL_TENANT_ID_INVALID) {
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
network_create (NetworkSpec& spec, NetworkResponse *rsp)
{
    hal_ret_t           ret = HAL_RET_OK;
    network_t           *nw = NULL;
    tenant_id_t         tid;
    tenant_t            *tenant = NULL;

    HAL_TRACE_DEBUG("--------------------- API Start ------------------------");
    HAL_TRACE_DEBUG("PI-Network:{}: Network Create ", __FUNCTION__);


    auto kh = spec.key_or_handle();
    auto nw_pfx = kh.ip_prefix();

    // validate the request message
    ret = validate_network_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // fetch the tenant information
    tid = spec.meta().tenant_id();
    tenant = find_tenant_by_id(tid);
    if (tenant == NULL) {
        ret = HAL_RET_INVALID_ARG;
        rsp->set_api_status(types::API_STATUS_TENANT_NOT_FOUND);
        goto end;
    }
    // instantiate a network
    nw = network_alloc_init();
    if (nw == NULL) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("PI-Network:{}: Out of Memory. Err: {}", 
                      ret);
        goto end;
    }

    nw->nw_key.tenant_id = tid;
    MAC_UINT64_TO_ADDR(nw->rmac_addr, spec.rmac());
    ip_pfx_spec_to_pfx_spec(&nw->nw_key.ip_pfx, nw_pfx);

    HAL_TRACE_DEBUG("Rmac: {}", macaddr2str(nw->rmac_addr));

    // nw->gw_ep_handle = 
    nw->hal_handle = hal_alloc_handle();

    // add this network object to our db
    ret = add_network_to_db(nw);
    HAL_ASSERT(ret == HAL_RET_OK);

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_status()->set_nw_handle(nw->hal_handle);

end:

    
    if (ret != HAL_RET_OK && nw != NULL) {
        network_free(nw);
    }
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

//------------------------------------------------------------------------------
// process a tenant update request
//------------------------------------------------------------------------------
hal_ret_t
network_update (NetworkSpec& spec, NetworkResponse *rsp)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a tenant get request
//------------------------------------------------------------------------------
hal_ret_t
network_get (NetworkGetRequest& req, NetworkGetResponseMsg *rsp)
{
    network_key_t         nw_key = { 0 };
    ip_prefix_t           ip_pfx = { 0 };
    network_t             *nw;
    NetworkGetResponse    *response;

    response = rsp->add_response();
    if (!req.has_meta() ||
        req.meta().tenant_id() == HAL_TENANT_ID_INVALID) {
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (req.has_key_or_handle()) {
        auto kh = req.key_or_handle();
        if (kh.key_or_handle_case() == NetworkKeyHandle::kIpPrefix) {
            auto nw_pfx = kh.ip_prefix();

            nw_key.tenant_id = req.meta().tenant_id();
            ip_pfx_spec_to_pfx_spec(&ip_pfx, nw_pfx);

            nw = find_network_by_key(nw_key.tenant_id, &ip_pfx);

        } else if (kh.key_or_handle_case() ==
                       NetworkKeyHandle::kNwHandle) {
            nw = find_network_by_handle(kh.nw_handle());
        } else {
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            return HAL_RET_INVALID_ARG;
        }
    } else {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    if (nw == NULL) {
        rsp->set_api_status(types::API_STATUS_ENDPOINT_NOT_FOUND);
        return HAL_RET_EP_NOT_FOUND;
    }

    // fill config spec of this tenant
    response->mutable_spec()->mutable_meta()->set_tenant_id(nw->nw_key.tenant_id);

    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a tenant delete request
//------------------------------------------------------------------------------
hal_ret_t
network_delete (NetworkDeleteRequest& req, NetworkDeleteResponseMsg *rsp)
{
    return HAL_RET_OK;
}

}    // namespace hal
