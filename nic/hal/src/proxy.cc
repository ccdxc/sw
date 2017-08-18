#include <base.h>
#include <hal.hpp>
#include <hal_lock.hpp>
#include <hal_state.hpp>
#include <proxy.hpp>
#include <proxy_svc.hpp>
#include <tenant.hpp>
#include <pd_api.hpp>

namespace hal {

void *
proxy_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((proxy_t *)entry)->type);
}

uint32_t
proxy_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(types::ProxyType)) % ht_size;
}

bool
proxy_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(types::ProxyType *)key1 == *(types::ProxyType *)key2) {
        return true;
    }
    return false;
}

void *
proxy_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((proxy_t *)entry)->hal_handle);
}

uint32_t
proxy_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
proxy_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// validate an incoming PROXY create request
// TODO:
// 1. check if PROXY exists already
//------------------------------------------------------------------------------
static hal_ret_t
validate_proxy_create (ProxySpec& spec, ProxyResponse *rsp)
{
    // must have key-handle set
    if (spec.proxy_type() == types::PROXY_TYPE_NONE) {
        rsp->set_api_status(types::API_STATUS_PROXY_TYPE_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this Proxy in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_proxy_to_db (proxy_t *proxy)
{
    g_hal_state->proxy_hal_handle_ht()->insert(proxy,
                                               &proxy->hal_handle_ht_ctxt);
    g_hal_state->proxy_type_ht()->insert(proxy, &proxy->ht_ctxt);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// API to allocate and program QID
//-----------------------------------------------------------------------------

hal_ret_t
proxy_allocate_qid(types::ProxyType type,
                   lif_id_t&        lif,
                   qtype_t&         qtype,
                   qid_t&           qid)
{
    proxy_t             *proxy = NULL;
    indexer::status     rs;

    proxy = find_proxy_by_type(type);
    if(proxy == NULL) {
        HAL_TRACE_ERR("Proxy not found for the type, {}", type);
        return HAL_RET_PROXY_NOT_FOUND;
    }

    qtype = proxy->qtype;
    
    rs = proxy->qid_idxr_->alloc((uint32_t *)&qid);
    if(rs != indexer::SUCCESS) {
        HAL_TRACE_ERR("Error in qid allocation, err: {}", rs);
        return HAL_RET_NO_RESOURCE;
    }

    HAL_TRACE_DEBUG("QID Allocator: qtype: {}, qid: {}", qtype, qid);
    return HAL_RET_OK;
}

hal_ret_t
proxy_program_qid(types::ProxyType type, lif_id_t lif, qtype_t qtype, qid_t qid)
{
    return HAL_RET_OK;
}

hal_ret_t
proxy_allocate_program_qid(types::ProxyType type,
                           lif_id_t    lif,
                           qtype_t& qtype, 
                           qid_t& qid) 
{
    hal_ret_t ret;
    ret = proxy_allocate_qid(type, lif, qtype, qid);
    if(ret!= HAL_RET_OK) {
        return ret;    
    }

    return proxy_program_qid(type, lif, qtype, qid);
}

//------------------------------------------------------------------------------
// process a Proxy create request
// TODO: if Proxy exists, treat this as modify (tenant id in the meta must
// match though)
//------------------------------------------------------------------------------
proxy_t* 
proxy_factory(types::ProxyType type, qtype_t qtype) 
{
    hal_ret_t       ret = HAL_RET_OK;
    proxy_t         * proxy = NULL;

    // instantiate Proxy
    proxy = proxy_alloc_init();
    if (proxy == NULL) {
        HAL_TRACE_ERR("Failed to allocate proxy service");
        return NULL;
    }

    proxy->type = type;
    proxy->qtype = qtype;
    proxy->hal_handle = hal_alloc_handle();
    
    // Instantiate QID indexer 
    proxy->qid_idxr_ = new hal::utils::indexer(HAL_MAX_QID);
    HAL_ASSERT(NULL != proxy->qid_idxr_);
    
    // TODO: Allocate  LIF for this proxy

    // add this Proxy to our db
    ret = add_proxy_to_db(proxy);
    HAL_ASSERT(ret == HAL_RET_OK);

    return proxy;
}

hal_ret_t
proxy_create (ProxySpec& spec, ProxyResponse *rsp)
{
    hal_ret_t              ret = HAL_RET_OK;
    proxy_t                *proxy;

    // validate the request message
    ret = validate_proxy_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        // api_status already set, just return
        HAL_TRACE_ERR("Proxy validate failure, err : {}", ret);
        return ret;
    }

    // instantiate Proxy
    proxy = proxy_factory(spec.proxy_type(), spec.proxy_qtype());
    if (ret != HAL_RET_OK) {
        rsp->set_api_status(types::API_STATUS_OUT_OF_MEM);
        return HAL_RET_OOM;
    }

    // prepare the response
    rsp->set_api_status(types::API_STATUS_OK);
    rsp->mutable_proxy_status()->set_proxy_handle(proxy->hal_handle);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// Initialize default proxy services
//------------------------------------------------------------------------------
hal_ret_t
hal_init_def_proxy_services(void) 
{
    HAL_ASSERT(NULL != proxy_factory(types::PROXY_TYPE_TCP, 0));
    HAL_TRACE_DEBUG("Initialized TCP Proxy Service...");
    HAL_ASSERT(NULL != proxy_factory(types::PROXY_TYPE_TLS, 0));   
    HAL_TRACE_DEBUG("Initialized TLS Proxy Service...");
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a Proxy update request
//------------------------------------------------------------------------------
hal_ret_t
proxy_update (ProxySpec& spec, ProxyResponse *rsp)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a Proxy get request
//------------------------------------------------------------------------------
hal_ret_t
proxy_get (ProxyGetRequest& req, ProxyGetResponse *rsp)
{
#if 0
    hal_ret_t              ret = HAL_RET_OK; 
    proxy_t                rproxy;
    proxy_t*               proxy;
    pd::pd_proxy_args_t    pd_proxy_args;

    if (!req.has_meta()) {
        rsp->set_api_status(types::API_STATUS_TENANT_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (!req.has_key_or_handle()) {
        rsp->set_api_status(types::API_STATUS_TCP_CB_ID_INVALID);
        return HAL_RET_INVALID_ARG;
    }
    auto kh = req.key_or_handle();

    if (kh.key_or_handle_case() == proxy::ProxyKeyHandle::kProxybId) {
        proxy = find_proxy_by_id(kh.proxy_id());
    } else if (kh.key_or_handle_case() == proxy::ProxyKeyHandle::kProxyHandle) {
        proxy = find_proxy_by_handle(kh.proxy_handle());
    } else {
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    if (proxy == NULL) {
        rsp->set_api_status(types::API_STATUS_TCP_CB_NOT_FOUND);
        return HAL_RET_TCP_CB_NOT_FOUND;
    }
    
    proxy_init(&rproxy);
    rproxy.cb_id = proxy->cb_id;
    pd::pd_proxy_args_init(&pd_proxy_args);
    pd_proxy_args.proxy = &rproxy;
    
    ret = pd::pd_proxy_get(&pd_proxy_args);
    if(ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD PROXY: Failed to get, err: ", ret);
        rsp->set_api_status(types::API_STATUS_TCP_CB_NOT_FOUND);
        return HAL_RET_HW_FAIL;
    }

    // fill config spec of this TCP CB 
    rsp->mutable_spec()->mutable_key_or_handle()->set_proxy_id(rproxy.cb_id);
    rsp->mutable_spec()->set_rcv_nxt(rproxy.rcv_nxt);

    // fill operational state of this TCP CB
    rsp->mutable_status()->set_proxy_handle(proxy->hal_handle);
#endif

    // fill stats of this TCP CB
    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

}    // namespace hal
