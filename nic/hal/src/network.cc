#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/utils.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {

// ----------------------------------------------------------------------------
// Get key function for network hash table
// ----------------------------------------------------------------------------
void *
network_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry;
    network_t                   *nw = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    nw = find_network_by_handle(ht_entry->handle_id);
    return (void *)&(nw->nw_key);

#if 0
    HAL_ASSERT(entry != NULL);
    return (void *)&(((network_t *)entry)->nw_key);
#endif
}

// ----------------------------------------------------------------------------
// Compute hash function for network hash table
// ----------------------------------------------------------------------------
uint32_t
network_compute_hash_func (void *key, uint32_t ht_size)
{
    return utils::hash_algo::fnv_hash(key, sizeof(network_key_t)) % ht_size;
}

// ----------------------------------------------------------------------------
// Compare key function for network hash table
// ----------------------------------------------------------------------------
bool
network_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(network_key_t))) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// insert network to db
//------------------------------------------------------------------------------
static inline hal_ret_t
network_add_to_db (network_t *nw, hal_handle_t handle)
{
    hal_ret_t                   ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-ep:{}:adding to network key hash table", 
                    __FUNCTION__);
    // allocate an entry to establish mapping from l2key to its handle
    entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->
        hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from tenant id to its handle
    entry->handle_id = handle;
    ret = g_hal_state->network_key_ht()->insert_with_key(&nw->nw_key,
                                                         entry, 
                                                         &entry->ht_ctxt);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:failed to network key to handle mapping, "
                      "err : {}", __FUNCTION__, ret);
        g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);
    }

    // TODO: Check if this is the right place
    nw->hal_handle = handle;

    return ret;
}

//------------------------------------------------------------------------------
// delete an ep from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
network_del_from_db (network_t *nw)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("pi-ep:{}:removing from network key hash table", 
                    __FUNCTION__);
    // remove from hash table
    entry = (hal_handle_id_ht_entry_t *)g_hal_state->ep_l2_ht()->
            remove(&nw->nw_key);

    if (entry) {
        // free up
        g_hal_state->hal_handle_id_ht_entry_slab()->free(entry);
    }

    return HAL_RET_OK;
}



//------------------------------------------------------------------------------
// validate an incoming network create request
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

hal_ret_t
network_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t       ret = HAL_RET_OK;

    // No PD calls
    return ret;
}

hal_ret_t
network_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;
    network_t                       *nw = NULL;
    dllist_ctxt_t                   *lnode = NULL;
    dhl_entry_t                     *dhl_entry = NULL;
    // network_create_app_ctxt_t       *app_ctxt = NULL; 
    hal_handle_t                    hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-ep:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    // assumption is there is only one element in the list
    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    // app_ctxt = (network_create_app_ctxt_t *)cfg_ctxt->app_ctxt;

    nw = (network_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;
    
    HAL_TRACE_DEBUG("pi-network:{}:create commit CB {}",
                    __FUNCTION__, network_to_str(nw));

    // Add network to key DB
    ret = network_add_to_db (nw, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-ep:{}:unable to add network to DB", 
                      __FUNCTION__);
        goto end;
    }

    HAL_TRACE_ERR("pi-network:{}:added network to DB", 
                  __FUNCTION__);
end:
    return ret;
}

// ----------------------------------------------------------------------------
// create abort callback
// ----------------------------------------------------------------------------
hal_ret_t
network_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                       ret = HAL_RET_OK;

    return ret;
}

// ----------------------------------------------------------------------------
// Dummy create cleanup callback
// ----------------------------------------------------------------------------
hal_ret_t
network_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Converts hal_ret_t to API status
//------------------------------------------------------------------------------
hal_ret_t
network_prepare_rsp (NetworkResponse *rsp, hal_ret_t ret, 
                     hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK) {
        rsp->mutable_status()->set_nw_handle(hal_handle);
    }

    rsp->set_api_status(hal_prepare_rsp(ret));

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a tenant create request
// TODO: if tenant exists, treat this as modify
//------------------------------------------------------------------------------
hal_ret_t
network_create (NetworkSpec& spec, NetworkResponse *rsp)
{
    hal_ret_t                       ret = HAL_RET_OK;
    network_t                       *nw = NULL;
    tenant_id_t                     tid;
    tenant_t                        *tenant = NULL;
    network_create_app_ctxt_t       app_ctxt;
    dhl_entry_t                     dhl_entry = { 0 };
    cfg_op_ctxt_t                   cfg_ctxt = { 0 };

    hal_api_trace(" API Begin:  network create ");

    auto kh = spec.key_or_handle();
    auto nw_pfx = kh.ip_prefix();

    // validate the request message
    ret = validate_network_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // fetch the tenant information
    tid = spec.meta().tenant_id();
    tenant = tenant_lookup_by_id(tid);
    if (tenant == NULL) {
        HAL_TRACE_ERR("pi-network:{}: unable to retrieve tenant_id:{}",
                __FUNCTION__, tid);
        ret = HAL_RET_TENANT_NOT_FOUND;
        goto end;
    }

    // instantiate a network
    nw = network_alloc_init();
    if (nw == NULL) {
        ret = HAL_RET_OOM;
        HAL_TRACE_ERR("pi-network:{}: out of memory. err: {}", 
                      ret);
        goto end;
    }

    // allocate hal handle id
    nw->hal_handle = hal_handle_alloc(HAL_OBJ_ID_NETWORK);
    if (nw->hal_handle == HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("pi-network:{}: failed to alloc handle", 
                      __FUNCTION__);
        ret = HAL_RET_HANDLE_INVALID;
        goto end;
    }

    nw->nw_key.tenant_id = tid;
    MAC_UINT64_TO_ADDR(nw->rmac_addr, spec.rmac());
    ip_pfx_spec_to_pfx_spec(&nw->nw_key.ip_pfx, nw_pfx);

    HAL_TRACE_DEBUG("nw: {}, rmac: {}", ippfx2str(&nw->nw_key.ip_pfx),
            macaddr2str(nw->rmac_addr));

    dhl_entry.handle = nw->hal_handle;
    dhl_entry.obj = nw;
    cfg_ctxt.app_ctxt = &app_ctxt;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(nw->hal_handle, &cfg_ctxt, 
                             network_create_add_cb,
                             network_create_commit_cb,
                             network_create_abort_cb, 
                             network_create_cleanup_cb);



end:
    
    if (ret != HAL_RET_OK && nw != NULL) {
        // if there is an error, nw will be freed in abort CB
        // network_free(nw);
        nw = NULL;
    }
    network_prepare_rsp(rsp, ret, nw ? nw->hal_handle : 0);
    HAL_TRACE_DEBUG("----------------------- API End ------------------------");
    return ret;
}

//------------------------------------------------------------------------------
// process a tenant update request
//------------------------------------------------------------------------------
hal_ret_t
network_update (NetworkSpec& spec, NetworkResponse *rsp)
{
    // TODO: Handle rmac change ?
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
    response->mutable_spec()->set_rmac(MAC_TO_UINT64(nw->rmac_addr));

    response->set_api_status(types::API_STATUS_OK);

    rsp->set_api_status(types::API_STATUS_OK);
    return HAL_RET_OK;
}

hal_ret_t
validate_network_delete (NetworkDeleteRequest& req, 
                         NetworkDeleteResponseMsg* rsp)
{
    hal_ret_t   ret = HAL_RET_OK;

    // key-handle field must be set
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("pi-network:{}:spec has no key or handle", __FUNCTION__);
        ret =  HAL_RET_INVALID_ARG;
    }

    return ret;
}

//------------------------------------------------------------------------------
// Lookup network from key or handle
//------------------------------------------------------------------------------
network_t *
network_lookup_key_or_handle (NetworkDeleteRequest& req)
{
    tenant_id_t                     tid;
    network_t                       *nw = NULL;
    ip_prefix_t                     ip_pfx;

    auto kh = req.key_or_handle();
    auto nw_pfx = kh.ip_prefix();

    tid = req.meta().tenant_id();
    ip_pfx_spec_to_pfx_spec(&ip_pfx, nw_pfx);

    if (kh.key_or_handle_case() == NetworkKeyHandle::kIpPrefix) {
        nw = find_network_by_key(tid, &ip_pfx);
    } else if (kh.key_or_handle_case() == NetworkKeyHandle::kNwHandle) {
        nw = find_network_by_handle(kh.nw_handle());
    }

    return nw;
}

//------------------------------------------------------------------------------
// Delete main CB
//------------------------------------------------------------------------------
hal_ret_t
network_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;

    return ret;
}

//------------------------------------------------------------------------------
// Update PI DBs as network_delete_del_cb() was a succcess
//      a. Delete from network key hash table
//      b. Remove object from handle id based hash table
//      c. Free PI network
//------------------------------------------------------------------------------
hal_ret_t
network_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t                   ret = HAL_RET_OK;
    dllist_ctxt_t               *lnode = NULL;
    dhl_entry_t                 *dhl_entry = NULL;
    network_t                   *nw = NULL;
    hal_handle_t                hal_handle = 0;

    if (cfg_ctxt == NULL) {
        HAL_TRACE_ERR("pi-network:{}:invalid cfg_ctxt", __FUNCTION__);
        ret = HAL_RET_INVALID_ARG;
        goto end;
    }

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);

    nw = (network_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    HAL_TRACE_DEBUG("pi-network:{}:delete commit CB {}",
                    __FUNCTION__, network_to_str(nw));

    // a. Remove from network key hash table
    ret = network_del_from_db(nw);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-network:{}:failed to del network {} from db, err : {}", 
                      __FUNCTION__, network_to_str(nw), ret);
        goto end;
    }

    // b. Remove object from handle id based hash table
    hal_handle_free(hal_handle);

    // c. Free PI network 
    network_free(nw);

    // TODO: Decrement the ref counts of dependent objects

end:
    return ret;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
network_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// If delete fails, nothing to do
//------------------------------------------------------------------------------
hal_ret_t
network_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a tenant delete request
//------------------------------------------------------------------------------
hal_ret_t
network_delete (NetworkDeleteRequest& req, NetworkDeleteResponseMsg *rsp)
{
    hal_ret_t           ret = HAL_RET_OK;
    network_t           *nw = NULL;
    ip_prefix_t         ip_pfx;
    cfg_op_ctxt_t       cfg_ctxt = { 0 };
    dhl_entry_t         dhl_entry = { 0 };

    hal_api_trace(" API Begin: network delete ");

    auto kh = req.key_or_handle();
    auto nw_pfx = kh.ip_prefix();

    // validate the request message
    ret = validate_network_delete(req, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("pi-network:{}:network delete validation failed, ret : {}",
                      __FUNCTION__, ret);
        goto end;
    }

    nw = network_lookup_key_or_handle(req);
    if (nw == NULL) {
        HAL_TRACE_ERR("pi-network:{}:failed to find nw handle {}",
                      __FUNCTION__, kh.nw_handle());
        ret = HAL_RET_NETWORK_NOT_FOUND;
        goto end;
    }

    ip_pfx_spec_to_pfx_spec(&ip_pfx, nw_pfx);
    HAL_TRACE_DEBUG("pi-network:{}:deleting nw pfx:{}", 
                    __FUNCTION__, ippfx2str(&ip_pfx));

    // form ctxt and call infra add
    dhl_entry.handle = nw->hal_handle;
    dhl_entry.obj = nw;
    cfg_ctxt.app_ctxt = NULL;
    utils::dllist_reset(&cfg_ctxt.dhl);
    utils::dllist_reset(&dhl_entry.dllist_ctxt);
    utils::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(nw->hal_handle, &cfg_ctxt, 
                             network_delete_del_cb,
                             network_delete_commit_cb,
                             network_delete_abort_cb, 
                             network_delete_cleanup_cb);

end:
    rsp->add_api_status(hal_prepare_rsp(ret));
    hal_api_trace(" API End: network delete ");
    return ret;
}

//-----------------------------------------------------------------------------
// Adds l2segs into nw list
//-----------------------------------------------------------------------------
hal_ret_t
network_add_l2seg (network_t *nw, l2seg_t *l2seg)
{
    hal_ret_t                   ret = HAL_RET_OK;
    hal_handle_id_list_entry_t  *entry = NULL;

    if (l2seg == NULL || nw == NULL) {
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

    network_lock(nw);      // lock
    // Insert into the list
    utils::dllist_add(&nw->l2seg_list_head, &entry->dllist_ctxt);
    network_unlock(nw);    // unlock

end:
    HAL_TRACE_DEBUG("pi-network:{}:add l2seg:{} to nw:{}, ret:{}",
                    __FUNCTION__, l2seg->seg_id, network_to_str(nw), ret);
    return ret;
}

//-----------------------------------------------------------------------------
// Remove l2seg from nw list
//-----------------------------------------------------------------------------
hal_ret_t
network_del_l2seg (network_t *nw, l2seg_t *l2seg)
{
    hal_ret_t                   ret = HAL_RET_IF_NOT_FOUND;
    hal_handle_id_list_entry_t  *entry = NULL;
    dllist_ctxt_t               *curr = NULL, *next = NULL;


    network_lock(nw);      // lock
    dllist_for_each_safe(curr, next, &nw->l2seg_list_head) {
        entry = dllist_entry(curr, hal_handle_id_list_entry_t, dllist_ctxt);
        if (entry->handle_id == l2seg->hal_handle) {
            // Remove from list
            utils::dllist_del(&entry->dllist_ctxt);
            // Free the entry
            g_hal_state->hal_handle_id_list_entry_slab()->free(entry);

            ret = HAL_RET_OK;
        }
    }
    network_unlock(nw);    // unlock

    HAL_TRACE_DEBUG("pi-network:{}:del l2seg:{} from network:{}, ret:{}",
                    __FUNCTION__, l2seg->seg_id, network_to_str(nw), ret);
    return ret;
}

const char *
network_to_str (network_t *nw)
{
    static thread_local char       nw_str[4][50];
    static thread_local uint8_t    nw_str_next = 0;
    char                           *buf;

    buf = nw_str[nw_str_next++ & 0x3];
    memset(buf, 0, 50);
    if (nw) {
        snprintf(buf, 50, "(tid:ippfx) : (%d:%s)", nw->nw_key.tenant_id,
                 ippfx2str(&nw->nw_key.ip_pfx));
    }
    return buf;
}


}    // namespace hal
