//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Handles CRUD APIs for NAT feature
//-----------------------------------------------------------------------------

#include <google/protobuf/util/json_util.h>
#include "nic/include/base.h"
#include "nic/include/hal_state.hpp"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"
#include "nic/hal/src/nw/vrf.hpp"
#include "nic/hal/src/nat/nat.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/utils/nat/addr_db.hpp"
#include "nic/hal/src/utils/addr_list.hpp"

using sdk::lib::ht_ctxt_t;
using hal::utils::nat::addr_entry_key_t;
using hal::utils::nat::addr_entry_t;

namespace hal {

//------------------------------------------------------------------------------
// return key for hash table that maps nat pool key to its handle
//------------------------------------------------------------------------------
void *
nat_pool_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry = NULL;
    nat_pool_t                  *nat_pool = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    nat_pool = (nat_pool_t *)hal_handle_get_obj(ht_entry->handle_id);
    if (nat_pool) {
       return (void *)&(nat_pool->key);
    }
    return NULL;
}

//------------------------------------------------------------------------------
// hash computation for nat pool key to handle hash table
//------------------------------------------------------------------------------
uint32_t
nat_pool_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(nat_pool_key_t)) % ht_size;
}

//------------------------------------------------------------------------------
// key comparision for nat pool key to handle hash table
//------------------------------------------------------------------------------
bool
nat_pool_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(nat_pool_key_t))) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// allocate a NAT pool instance
//------------------------------------------------------------------------------
static inline nat_pool_t *
nat_pool_alloc (void)
{
    nat_pool_t    *pool;

    pool = (nat_pool_t *)g_hal_state->nat_pool_slab()->alloc();
    if (pool == NULL) {
        return NULL;
    }
    return pool;
}

//------------------------------------------------------------------------------
// initialize a NAT pool instance
//------------------------------------------------------------------------------
static inline nat_pool_t *
nat_pool_init (nat_pool_t *pool)
{
    if (!pool) {
        return NULL;
    }
    HAL_SPINLOCK_INIT(&pool->slock, PTHREAD_PROCESS_SHARED);
    dllist_reset(&pool->addr_ranges);
    pool->hal_handle = HAL_HANDLE_INVALID;

    return pool;
}

//------------------------------------------------------------------------------
// allocate and initialize a NAT pool instance
//------------------------------------------------------------------------------
static inline nat_pool_t *
nat_pool_alloc_init (void)
{
    return nat_pool_init(nat_pool_alloc());
}

//------------------------------------------------------------------------------
// free NAT pool instance back to its slab
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_free (nat_pool_t *pool)
{
    HAL_SPINLOCK_DESTROY(&pool->slock);
    hal::delay_delete_to_slab(HAL_SLAB_NAT_POOL, pool);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// free up all the state associated with a NAT pool
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_cleanup (nat_pool_t *pool)
{
    // TODO: purge all NAT binding of this pool
    addr_list_cleanup(&pool->addr_ranges);
    nat_pool_free(pool);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// lookup a NAT pool by its key
//------------------------------------------------------------------------------
static inline nat_pool_t *
find_nat_pool_by_key (nat_pool_key_t *key)
{
    hal_handle_id_ht_entry_t    *handle_id_entry;
    hal_handle                  *handle;

    handle_id_entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->nat_pool_ht()->lookup(key);
    if (handle_id_entry) {
        if (handle_id_entry->handle_id != HAL_HANDLE_INVALID) {
            handle = hal_handle_get_from_handle_id(handle_id_entry->handle_id);
            if (handle && (handle->obj_id() == HAL_OBJ_ID_NAT_POOL)) {
                return (nat_pool_t *)handle->obj();
            }
        }
    }
    return NULL;
}

//------------------------------------------------------------------------------
// lookup a NAT pool by its handle
//------------------------------------------------------------------------------
static inline nat_pool_t *
find_nat_pool_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle : {}", handle);
        return NULL;
    }

    if (hal_handle->obj_id() != HAL_OBJ_ID_NAT_POOL) {
        HAL_TRACE_DEBUG("Object id mismatch for handle {}, obj id found {}",
                        handle, hal_handle->obj_id());
        return NULL;
    }

    return (nat_pool_t *)hal_handle_get_obj(handle);
}

//------------------------------------------------------------------------------
// lookup a NAT pool by key-handle spec
//------------------------------------------------------------------------------
static inline nat_pool_t *
find_nat_pool_by_key_or_handle (const NatPoolKeyHandle& kh)
{
    nat_pool_key_t    key;
    vrf_t             *vrf;

    if (kh.has_pool_key()) {
        if (kh.pool_key().vrf_kh().key_or_handle_case() ==
                VrfKeyHandle::kVrfId) {
            key.vrf_id = kh.pool_key().vrf_kh().vrf_id();
        } else {
            vrf = vrf_lookup_by_handle(kh.pool_key().vrf_kh().vrf_handle());
            if (vrf == NULL) {
                HAL_TRACE_ERR("vrf {} not found",
                              kh.pool_key().vrf_kh().vrf_handle());
                return NULL;
            }
            key.vrf_id = vrf->vrf_id;
        }
        key.pool_id = kh.pool_key().pool_id();
        return find_nat_pool_by_key(&key);
    }

    return find_nat_pool_by_handle(kh.pool_handle());
}

//------------------------------------------------------------------------------
// lookup a NAT pool by its key
//------------------------------------------------------------------------------
static inline nat_pool_t *
find_nat_pool (vrf_id_t vrf_id, const NatPoolKeyHandle& kh)
{
    nat_pool_key_t    key;

    if (kh.has_pool_key()) {
        key.vrf_id = vrf_id;
        key.pool_id = kh.pool_key().pool_id();
        return find_nat_pool_by_key(&key);
    }
    return find_nat_pool_by_handle(kh.pool_handle());
}

//------------------------------------------------------------------------------
// add a natpool to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_add_to_db (nat_pool_t *pool, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding natpool ({}, {}) to cfg db",
                    pool->key.vrf_id, pool->key.pool_id);

    // allocate an entry to establish mapping from natpool key to its handle
    entry = (hal_handle_id_ht_entry_t *)
                g_hal_state->hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from pool key to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->nat_pool_ht()->insert_with_key(&pool->key,
                                                          entry,
                                                          &entry->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add natpool ({}, {}) to handle db,"
                      " err : {}", pool->key.vrf_id, pool->key.pool_id, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
        return ret;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// delete a natpool from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_del_from_db (nat_pool_t *pool)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Removing natpool ({}, {}) from cfg db",
                    pool->key.vrf_id, pool->key.pool_id);
    entry =
        (hal_handle_id_ht_entry_t *)
            g_hal_state->nat_pool_ht()->remove(&pool->key);

    // free up the hash entry
    if (entry) {
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    } else {
        HAL_TRACE_ERR("Failed to delete natpool ({}, {}), pool not found",
                      pool->key.vrf_id, pool->key.pool_id);
        return HAL_RET_NAT_POOL_NOT_FOUND;
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// dump natpool spec
//-----------------------------------------------------------------------------
static inline void
nat_pool_spec_dump (NatPoolSpec& spec)
{
    std::string    pool_cfg;

    if (hal::utils::hal_trace_level() < hal::utils::trace_debug)  {
        return;
    }
    google::protobuf::util::MessageToJsonString(spec, &pool_cfg);
    HAL_TRACE_DEBUG("NAT pool configuration:");
    HAL_TRACE_DEBUG("{}", pool_cfg.c_str());
    return;
}

//-----------------------------------------------------------------------------
// validate NAT pool configuration
//-----------------------------------------------------------------------------
static inline hal_ret_t
validate_nat_pool_create (NatPoolSpec& spec, NatPoolResponse *rsp)
{
    // check if key-handle field is set or not
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("NAT pool id/handle not set in request");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() != NatPoolKeyHandle::kPoolKey) {
        // key-handle field set, but key is not populated
        HAL_TRACE_ERR("NAT pool key fields are not set");
        rsp->set_api_status(types::API_STATUS_NAT_POOL_KEY_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (!kh.pool_key().has_vrf_kh()) {
        // vrf key/handle not set in the NAT pool key
        HAL_TRACE_ERR("VRF id/handle missing in NAT pool key");
        rsp->set_api_status(types::API_STATUS_NAT_POOL_KEY_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // check if atleast one prefix or range is configured
    if (spec.address_size() == 0) {
        HAL_TRACE_ERR("NAT pool doesn't have any NAT addresses");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// initialize NAT pool object given its configuration/spec
//-----------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_init_from_spec (vrf_t *vrf, nat_pool_t *pool, const NatPoolSpec& spec)
{
    pool->key.vrf_id = vrf->vrf_id;
    pool->key.pool_id = spec.key_or_handle().pool_handle();
    for (int i = 0; i < spec.address_size(); i++) {
        hal_ret_t ret = addr_list_elem_address_spec_handle(spec.address(i),
                                                           &pool->addr_ranges);
        if (ret == HAL_RET_OOM) {
            addr_list_cleanup(&pool->addr_ranges);
            return ret;
        }
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// allocate hw resources and program the hw tables, if any, for the NAT pool
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// perform the commit operation for the NAT pool by adding to config db and
// making this config available
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t        ret;
    dllist_ctxt_t    *lnode = NULL;
    dhl_entry_t      *dhl_entry = NULL;
    nat_pool_t       *pool;
    hal_handle_t     hal_handle;

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    pool = (nat_pool_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    // add nat pool to the cfg db
    ret = nat_pool_add_to_db(pool, hal_handle);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add natpool ({}, {}) to cfg db",
                      pool->key.vrf_id, pool->key.pool_id);
    }
    HAL_TRACE_DEBUG("Committed natpool ({}, {}) to cfg db",
                    pool->key.vrf_id, pool->key.pool_id);
    return ret;
}

//------------------------------------------------------------------------------
// abort NAT pool create operation
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    dllist_ctxt_t    *lnode = NULL;
    dhl_entry_t      *dhl_entry = NULL;
    nat_pool_t       *pool;

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    pool = (nat_pool_t *)dhl_entry->obj;
    hal_handle_free(dhl_entry->handle);
    nat_pool_cleanup(pool);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// cleanup any transient state that we are holding
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// convert hal_ret_t to API status to return to the app
//-----------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_prepare_rsp (NatPoolResponse *rsp, hal_ret_t ret,
                      hal_handle_t hal_handle)
{
    if ((ret == HAL_RET_OK) || (ret == HAL_RET_ENTRY_EXISTS)) {
        rsp->mutable_pool_status()->set_pool_handle(hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// process a NAT pool create request
//-----------------------------------------------------------------------------
hal_ret_t
nat_pool_create (NatPoolSpec& spec, NatPoolResponse *rsp)
{
    hal_ret_t        ret;
    vrf_t            *vrf;
    nat_pool_t       *pool = NULL;
    dhl_entry_t      dhl_entry = { 0 };
    cfg_op_ctxt_t    cfg_ctxt  = { 0 };

    // dump the received config
    nat_pool_spec_dump(spec);

    auto pool_key = spec.key_or_handle().pool_key();
    // validate the request message
    ret = validate_nat_pool_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("NAT pool config validation failed, err : {}", ret);
        goto end;
    }

    // lookup the vrf
    vrf = vrf_lookup_key_or_handle(pool_key.vrf_kh());
    if (vrf == NULL) {
        HAL_TRACE_ERR("Failed to find vrf with key {}/handle {}",
                      pool_key.vrf_kh().vrf_id(),
                      pool_key.vrf_kh().vrf_handle());
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }

    // check to see if this mapping exists
    if ((pool = find_nat_pool(vrf->vrf_id, spec.key_or_handle()))) {
        HAL_TRACE_ERR("Failed to create natpool, pool ({}, {}) exists already",
                      vrf->vrf_id, pool_key.pool_id());
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    // instanitate NAT pool object
    pool = nat_pool_alloc_init();
    if (pool == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init natpool ({}, {})",
                      vrf->vrf_id, pool_key.pool_id());
        ret = HAL_RET_OOM;
        goto end;
    }

    // copy the config from incoming request
    ret = nat_pool_init_from_spec(vrf, pool, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize natpool ({}, {})",
                     vrf->vrf_id, pool_key.pool_id());
        goto end;
    }

    // allocate HAL handle id
    pool->hal_handle = hal_handle_alloc(HAL_OBJ_ID_NAT_POOL);
    if (pool->hal_handle == HAL_HANDLE_INVALID) {
         HAL_TRACE_ERR("Failed to alloc handle for natpool ({}, {})",
                       vrf->vrf_id, pool->key.pool_id);
         rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
         nat_pool_cleanup(pool);
         ret = HAL_RET_HANDLE_INVALID;
         goto end;
    }

    // form ctxt and handover to the HAL infra
    dhl_entry.handle = pool->hal_handle;
    dhl_entry.obj = pool;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(pool->hal_handle, &cfg_ctxt,
                             nat_pool_create_add_cb,
                             nat_pool_create_commit_cb,
                             nat_pool_create_abort_cb,
                             nat_pool_create_cleanup_cb);

end:

    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
        if (pool) {
            // abort callback will take care of cleaning up
            pool = NULL;
        }
        HAL_API_STATS_INC(HAL_API_NAT_POOL_CREATE_FAIL);
    } else {
        HAL_API_STATS_INC(HAL_API_NAT_POOL_CREATE_SUCCESS);
    }
    nat_pool_prepare_rsp(rsp, ret,
                         pool ? pool->hal_handle : HAL_HANDLE_INVALID);
    return ret;
}

//-----------------------------------------------------------------------------
// process a NAT pool update request
//-----------------------------------------------------------------------------
hal_ret_t
nat_pool_update (NatPoolSpec& spec, NatPoolResponse *rsp)
{
    HAL_TRACE_ERR("NAT pool update operation not supported");
    return HAL_RET_INVALID_OP;
}

//------------------------------------------------------------------------------
// validate a nat pool delete request
//------------------------------------------------------------------------------
static inline hal_ret_t
validate_nat_pool_delete_req (NatPoolDeleteRequest& req,
                              NatPoolDeleteResponse *rsp)
{
    // check if key-handle field is set or not
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("NAT pool id/handle not set in request");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    // if key is set, make sure that VRF key/handle is set
    auto kh = req.key_or_handle();
    if (kh.key_or_handle_case() == NatPoolKeyHandle::kPoolKey) {
        if (!kh.pool_key().has_vrf_kh()) {
            // vrf key/handle not set in the NAT pool key
            HAL_TRACE_ERR("VRF id/handle missing in NAT pool key");
            rsp->set_api_status(types::API_STATUS_NAT_POOL_KEY_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// free hw resources and cleanup the hw tables, if any, for the NAT pool
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// perform the commit operation for the NAT pool by deleting from config db
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t       ret;
    dllist_ctxt_t   *lnode = NULL;
    dhl_entry_t     *dhl_entry = NULL;
    nat_pool_t      *pool;
    hal_handle_t    hal_handle;

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    pool = (nat_pool_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    ret = nat_pool_del_from_db(pool);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    HAL_TRACE_DEBUG("Deleted natpool ({}, {}) from cfg db",
                    pool->key.vrf_id, pool->key.pool_id);

    // free all sw resources associated with this pool
    hal_handle_free(hal_handle);
    nat_pool_cleanup(pool);

    return ret;
}

//------------------------------------------------------------------------------
// abort NAT pool delete operation
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    HAL_TRACE_ERR("Aborting NAT pool delete operation");
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// cleanup any transient state that we are holding
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// dump nat pool delete request
//-----------------------------------------------------------------------------
static inline void
nat_pool_delete_req_dump (NatPoolDeleteRequest& req)
{
    std::string    del_req;

    if (hal::utils::hal_trace_level() < hal::utils::trace_debug)  {
        return;
    }
    google::protobuf::util::MessageToJsonString(req, &del_req);
    HAL_TRACE_DEBUG("NAT pool delete request:");
    HAL_TRACE_DEBUG("{}", del_req.c_str());
    return;
}

//-----------------------------------------------------------------------------
// process a NAT pool delete request
//-----------------------------------------------------------------------------
hal_ret_t
nat_pool_delete (NatPoolDeleteRequest& req, NatPoolDeleteResponse *rsp)
{
    hal_ret_t        ret;
    nat_pool_t       *pool;
    cfg_op_ctxt_t    cfg_ctxt = { 0 };
    dhl_entry_t      dhl_entry = { 0 };

    // validate the request message
    ret = validate_nat_pool_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // get the NAT pool
    pool = find_nat_pool_by_key_or_handle(req.key_or_handle());
    if (pool == NULL) {
        HAL_TRACE_ERR("Failed to delete natpool, pool not found");
        nat_pool_delete_req_dump(req);
        ret = HAL_RET_NAT_POOL_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("Deleting natpool ({}, {})",
                    pool->key.vrf_id, pool->key.pool_id);

    // TODO: check if there are NAT mappings outstanding

    // form ctxt and handover to the HAL infra
    dhl_entry.handle = pool->hal_handle;
    dhl_entry.obj = pool;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(pool->hal_handle, &cfg_ctxt,
                             nat_pool_delete_del_cb,
                             nat_pool_delete_commit_cb,
                             nat_pool_delete_abort_cb,
                             nat_pool_delete_cleanup_cb);

end:

    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC (HAL_API_NAT_POOL_DELETE_SUCCESS);
    } else {
        HAL_API_STATS_INC (HAL_API_NAT_POOL_DELETE_FAIL);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return ret;
}

//------------------------------------------------------------------------------
// process a get request for a given nat pool
//------------------------------------------------------------------------------
static inline void
nat_pool_process_get (nat_pool_t *pool, NatPoolGetResponse *rsp)
{
    sdk::lib::dllist_ctxt_t    *entry;
    addr_list_elem_t           *addr_range;

    auto pool_key =
        rsp->mutable_spec()->mutable_key_or_handle()->mutable_pool_key();
    pool_key->mutable_vrf_kh()->set_vrf_id(pool->key.vrf_id);
    pool_key->set_pool_id(pool->key.pool_id);

    dllist_for_each(entry, &pool->addr_ranges) {
        auto addr_spec = rsp->mutable_spec()->add_address();
         addr_range =
             dllist_entry(entry, addr_list_elem_t, list_ctxt);
         ip_range_to_spec(addr_spec->mutable_range(), &addr_range->ip_range);
    }
    rsp->set_api_status(types::API_STATUS_OK);
}

//------------------------------------------------------------------------------
// callback invoked from nat pool hash table while processing get request
//------------------------------------------------------------------------------
static inline bool
nat_pool_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t    *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    NatPoolGetResponseMsg       *rsp = (NatPoolGetResponseMsg *)ctxt;
    NatPoolGetResponse          *response = rsp->add_response();
    nat_pool_t                  *pool;

    pool = (nat_pool_t *)hal_handle_get_obj(entry->handle_id);
    nat_pool_process_get(pool, response);

    // return false here, so that we don't terminate the walk
    return false;
}

//------------------------------------------------------------------------------
// process a NAT pool get request
//------------------------------------------------------------------------------
hal_ret_t
nat_pool_get (NatPoolGetRequest& req, NatPoolGetResponseMsg *rsp)
{
    nat_pool_t    *pool;

    // if the natpool key-handle field is not set, then this is a request
    // for information for all pools, so run through all pools in the
    // cfg db and populate the response
    if (!req.has_key_or_handle()) {
        g_hal_state->nat_pool_ht()->walk(nat_pool_get_ht_cb, rsp);
    } else {
        auto kh = req.key_or_handle();
        pool = find_nat_pool_by_key_or_handle(kh);
        auto response = rsp->add_response();
        if (pool == NULL) {
            response->set_api_status(types::API_STATUS_NOT_FOUND);
            HAL_API_STATS_INC(HAL_API_NAT_POOL_GET_FAIL);
            return HAL_RET_NAT_POOL_NOT_FOUND;
        } else {
            nat_pool_process_get(pool, response);
        }
    }

    HAL_API_STATS_INC(HAL_API_NAT_POOL_GET_SUCCESS);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// NAT Policy config routines
//------------------------------------------------------------------------------
hal_ret_t
nat_policy_create (NatPolicySpec& spec, NatPolicyResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_policy_update (NatPolicySpec& spec, NatPolicyResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_policy_delete (NatPolicyDeleteRequest& req,
                   NatPolicyDeleteResponse *res)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_policy_get (NatPolicyGetRequest& req, NatPolicyGetResponseMsg *res)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// return key for hash table that maps nat mapping key to its handle
//------------------------------------------------------------------------------
void *
nat_mapping_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry = NULL;
    addr_entry_t                *mapping = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    mapping = (addr_entry_t *)hal_handle_get_obj(ht_entry->handle_id);
    if (mapping) {
       return (void *)&(mapping->key);
    }
    return NULL;
}

//------------------------------------------------------------------------------
// hash computation for nat mapping key to handle hash table
//------------------------------------------------------------------------------
uint32_t
nat_mapping_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key,
                                         sizeof(addr_entry_key_t)) % ht_size;
}

//------------------------------------------------------------------------------
// key comparision for nat mapping key to handle hash table
//------------------------------------------------------------------------------
bool
nat_mapping_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(addr_entry_key_t))) {
        return true;
    }
    return false;
}

//------------------------------------------------------------------------------
// allocate a NAT mapping instance
//------------------------------------------------------------------------------
static inline addr_entry_t *
nat_mapping_alloc (void)
{
    return hal::utils::nat::addr_entry_alloc();
}


//------------------------------------------------------------------------------
// initialize a NAT mapping instance
//------------------------------------------------------------------------------
static inline addr_entry_t *
nat_mapping_init (addr_entry_t *mapping)
{
    if (!mapping) {
        return NULL;
    }
    mapping->hal_handle = HAL_HANDLE_INVALID;
    mapping->db_node.reset();

    return mapping;
}

//------------------------------------------------------------------------------
// allocate and initialize a NAT mapping instance
//------------------------------------------------------------------------------
static inline addr_entry_t *
nat_mapping_alloc_init (void)
{
    return nat_mapping_init(nat_mapping_alloc());
}

//------------------------------------------------------------------------------
// free NAT mapping instance back to its slab
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_free (addr_entry_t *mapping)
{
    hal::utils::nat::addr_entry_free(mapping);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// clean up all the state associated with a NAT mapping
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_cleanup (addr_entry_t *mapping)
{
    nat_mapping_free(mapping);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// lookup a NAT mapping by its handle
//------------------------------------------------------------------------------
static inline addr_entry_t *
find_nat_mapping_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_DEBUG("Failed to find object with handle : {}", handle);
        return NULL;
    }

    if (hal_handle->obj_id() != HAL_OBJ_ID_NAT_MAPPING) {
        HAL_TRACE_DEBUG("Object id mismatch for handle {}, obj id found {}",
                        handle, hal_handle->obj_id());
        return NULL;
    }

    return (addr_entry_t *)hal_handle_get_obj(handle);
}

//------------------------------------------------------------------------------
// lookup a NAT mapping by key-handle spec
//------------------------------------------------------------------------------
static inline addr_entry_t *
find_nat_mapping_by_key_or_handle (const NatMappingKeyHandle& kh)
{
    addr_entry_key_t    key;
    vrf_t               *vrf;

    if (kh.has_svc()) {
        if (kh.svc().vrf_kh().key_or_handle_case() == VrfKeyHandle::kVrfId) {
            key.vrf_id = kh.svc().vrf_kh().vrf_id();
        } else {
            vrf = vrf_lookup_by_handle(kh.svc().vrf_kh().vrf_handle());
            if (vrf == NULL) {
                HAL_TRACE_ERR("vrf {} not found",
                              kh.svc().vrf_kh().vrf_handle());
                return NULL;
            }
            key.vrf_id = vrf->vrf_id;
        }
        ip_addr_spec_to_ip_addr(&key.ip_addr, kh.svc().ip_addr());
        return hal::utils::nat::addr_entry_get(&key);
    }
    return find_nat_mapping_by_handle(kh.mapping_handle());
}

//------------------------------------------------------------------------------
// lookup a NAT mapping by its key
//------------------------------------------------------------------------------
static inline addr_entry_t *
find_nat_mapping (vrf_id_t vrf_id, const NatMappingKeyHandle& kh)
{
    addr_entry_key_t    key;

    if (kh.key_or_handle_case() == NatMappingKeyHandle::kSvc) {
        key.vrf_id = vrf_id;
        ip_addr_spec_to_ip_addr(&key.ip_addr, kh.svc().ip_addr());
        return addr_entry_get(&key);
    }
    return find_nat_mapping_by_handle(kh.mapping_handle());
}

//------------------------------------------------------------------------------
// add a nat mapping to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_add_to_db (addr_entry_t *mapping, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding nat mapping ({}, {}) -> ({}, {}) to cfg db",
                    mapping->key.vrf_id, ipaddr2str(&mapping->key.ip_addr),
                    mapping->tgt_vrf_id, ipaddr2str(&mapping->tgt_ip_addr));

    // allocate an entry to establish mapping from natpool key to its handle
    entry = (hal_handle_id_ht_entry_t *)
                g_hal_state->hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        return HAL_RET_OOM;
    }

    // add mapping from to its handle
    entry->handle_id = handle;
    sdk_ret =
        g_hal_state->nat_mapping_ht()->insert_with_key(&mapping->key,
                                                       entry, &entry->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add nat mapping ({}, {}) -> ({}, {}) to "
                      "handle db, err : {}",
                      mapping->key.vrf_id, ipaddr2str(&mapping->key.ip_addr),
                      mapping->tgt_vrf_id, ipaddr2str(&mapping->tgt_ip_addr));
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
        return ret;
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// delete a nat mapping from the config database
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_del_from_db (addr_entry_t *mapping)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Removing nat mapping ({}, {}) -> ({}, {}) from cfg db",
                    mapping->key.vrf_id, ipaddr2str(&mapping->key.ip_addr),
                    mapping->tgt_vrf_id, ipaddr2str(&mapping->tgt_ip_addr));

    entry =
        (hal_handle_id_ht_entry_t *)
            g_hal_state->nat_mapping_ht()->remove(&mapping->key);

    // free up the hash entry
    if (entry) {
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    } else {
        HAL_TRACE_ERR("Failed to delete nat mapping ({}, {}) --> ({}, {}), "
                      "mapping not found",
                      mapping->key.vrf_id, ipaddr2str(&mapping->key.ip_addr),
                      mapping->tgt_vrf_id, ipaddr2str(&mapping->tgt_ip_addr));
        return HAL_RET_NAT_MAPPING_NOT_FOUND;
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// dump nat mapping spec
//-----------------------------------------------------------------------------
static inline void
nat_mapping_spec_dump (NatMappingSpec& spec)
{
    std::string    nat_mapping_cfg;

    if (hal::utils::hal_trace_level() < hal::utils::trace_debug)  {
        return;
    }

    google::protobuf::util::MessageToJsonString(spec, &nat_mapping_cfg);
    HAL_TRACE_DEBUG("NAT mapping configuration:");
    HAL_TRACE_DEBUG("{}", nat_mapping_cfg.c_str());
    return;
}

//-----------------------------------------------------------------------------
// validate NAT mapping configuration
//-----------------------------------------------------------------------------
static inline hal_ret_t
validate_nat_mapping_create (NatMappingSpec& spec, NatMappingResponse *rsp)
{
    // check if key-handle field is set or not
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("NAT mapping key-handle not set in request");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    auto kh = spec.key_or_handle();
    if (kh.key_or_handle_case() != NatMappingKeyHandle::kSvc) {
        // key-handle field set, but key is not populated
        HAL_TRACE_ERR("NAT mapping key fields are not set");
        rsp->set_api_status(types::API_STATUS_NAT_MAPPING_KEY_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (!kh.svc().has_vrf_kh()) {
        // vrf key/handle not set in the service
        HAL_TRACE_ERR("VRF id/handle missing in NAT mapping key");
        rsp->set_api_status(types::API_STATUS_NAT_MAPPING_KEY_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    if (!kh.svc().has_ip_addr()) {
        // IP address not set in the service
        HAL_TRACE_ERR("Service IP in NAT mapping key");
        rsp->set_api_status(types::API_STATUS_NAT_MAPPING_KEY_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // we don't support port in the key currently
    if (kh.svc().port() != 0) {
        HAL_TRACE_ERR("Service with port isn't supported");
        rsp->set_api_status(types::API_STATUS_NAT_MAPPING_KEY_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    // NAT pool must be set
    if (!spec.has_nat_pool()) {
        HAL_TRACE_ERR("NAT pool not specificed");
        rsp->set_api_status(types::API_STATUS_NAT_MAPPING_KEY_INVALID);
        return HAL_RET_INVALID_ARG;
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// initialize NAT mapping object given its configuration/spec
//-----------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_init_from_spec (vrf_t *vrf, addr_entry_t *mapping,
                            const NatMappingSpec& spec)
{
    nat_pool_t    *pool;

    pool = find_nat_pool(vrf->vrf_id, spec.nat_pool());
    if (!pool) {
        HAL_TRACE_ERR("Nat pool ({}, {}) not found",
                      vrf->vrf_id,
                      spec.nat_pool().has_pool_key() ?
                          spec.nat_pool().pool_key().pool_id() :
                          spec.nat_pool().pool_handle());
        return HAL_RET_NAT_POOL_NOT_FOUND;
    }

    mapping->key.vrf_id = vrf->vrf_id;
    ip_addr_spec_to_ip_addr(&mapping->key.ip_addr,
                            spec.key_or_handle().svc().ip_addr());
    mapping->nat_pool_id = pool->key.pool_id;
    mapping->origin = NAT_MAPPING_ORIGIN_CFG;
    mapping->bidir = spec.bidir() ? TRUE : FALSE;

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// allocate hw resources and program the hw tables, if any, for the NAT mapping
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// perform the commit operation for the NAT mapping by adding to config db and
// making this config available
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t        ret;
    dllist_ctxt_t    *lnode = NULL;
    dhl_entry_t      *dhl_entry = NULL;
    addr_entry_t     *mapping;
    hal_handle_t     hal_handle;

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    mapping = (addr_entry_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    // add nat pool to the cfg db
    ret = nat_mapping_add_to_db(mapping, hal_handle);
    return ret;
}

//------------------------------------------------------------------------------
// abort NAT mapping create operation
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_create_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    dllist_ctxt_t    *lnode = NULL;
    dhl_entry_t      *dhl_entry = NULL;
    addr_entry_t     *mapping;

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    mapping = (addr_entry_t *)dhl_entry->obj;
    hal_handle_free(dhl_entry->handle);
    nat_mapping_cleanup(mapping);

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// cleanup any transient state that we are holding
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// convert hal_ret_t to API status to return to the app
//-----------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_prepare_rsp (NatMappingResponse *rsp, hal_ret_t ret,
                         hal_handle_t hal_handle)
{
    if ((ret == HAL_RET_OK) || (ret == HAL_RET_ENTRY_EXISTS)) {
        rsp->mutable_status()->set_handle(hal_handle);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// process a NAT mapping create request
//-----------------------------------------------------------------------------
hal_ret_t
nat_mapping_create (NatMappingSpec& spec,
                    NatMappingResponse *rsp)
{
    hal_ret_t        ret;
    vrf_t            *vrf;
    addr_entry_t     *mapping = NULL;
    dhl_entry_t      dhl_entry = { 0 };
    cfg_op_ctxt_t    cfg_ctxt  = { 0 };

    // dump the incoming config
    nat_mapping_spec_dump(spec);

    auto svc = spec.key_or_handle().svc();

    // validate the request message
    ret = validate_nat_mapping_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("NAT mapping create cfg validation error, err : {}", ret);
        goto end;
    }

    // lookup the vrf
    vrf = vrf_lookup_key_or_handle(svc.vrf_kh());
    if (vrf == NULL) {
        HAL_TRACE_ERR("Failed to find vrf with key {}/handle {}",
                      svc.vrf_kh().vrf_id(), svc.vrf_kh().vrf_handle());
        ret = HAL_RET_VRF_NOT_FOUND;
    }

    // check to see if the NAT address mapping exists for this already
    if ((mapping = find_nat_mapping(vrf->vrf_id, spec.key_or_handle()))) {
        HAL_TRACE_ERR("Failed to create NAT mapping, mapping exists already");
        ret = HAL_RET_ENTRY_EXISTS;
        goto end;
    }

    // instanitate NAT address map object
    mapping = nat_mapping_alloc_init();
    if (mapping == NULL) {
        HAL_TRACE_ERR("Failed to alloc/init nat addr map");
        nat_mapping_spec_dump(spec);
        ret = HAL_RET_OOM;
        goto end;
    }

    // copy the config from incoming request
    ret = nat_mapping_init_from_spec(vrf, mapping, spec);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to initialize nat mapping, err : {}", ret);
        nat_mapping_spec_dump(spec);
        goto end;
    }

    // allocate HAL handle id
    mapping->hal_handle = hal_handle_alloc(HAL_OBJ_ID_NAT_MAPPING);
    if (mapping->hal_handle == HAL_HANDLE_INVALID) {
         HAL_TRACE_ERR("Failed to alloc handle for NAT mapping ({}, {})",
                       vrf->vrf_id, ipaddr2str(&mapping->key.ip_addr));
         rsp->set_api_status(types::API_STATUS_HANDLE_INVALID);
         nat_mapping_cleanup(mapping);
         ret = HAL_RET_HANDLE_INVALID;
         goto end;
    }

    // TODO: allocate NAT address

    // form ctxt and handover to the HAL infra
    dhl_entry.handle = mapping->hal_handle;
    dhl_entry.obj = mapping;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_add_obj(mapping->hal_handle, &cfg_ctxt,
                             nat_mapping_create_add_cb,
                             nat_mapping_create_commit_cb,
                             nat_mapping_create_abort_cb,
                             nat_mapping_create_cleanup_cb);

end:

    if ((ret != HAL_RET_OK) && (ret != HAL_RET_ENTRY_EXISTS)) {
        if (mapping) {
            // abort callback will take care of cleaning up
            mapping = NULL;
        }
        HAL_API_STATS_INC(HAL_API_NAT_MAPPING_CREATE_FAIL);
    } else {
        HAL_API_STATS_INC(HAL_API_NAT_MAPPING_CREATE_SUCCESS);
    }
    nat_mapping_prepare_rsp(rsp, ret,
                            mapping ? mapping->hal_handle : HAL_HANDLE_INVALID);
    return ret;
}

//-----------------------------------------------------------------------------
// dump nat mapping delete request
//-----------------------------------------------------------------------------
static inline void
nat_mapping_delete_req_dump (NatMappingDeleteRequest& req)
{
    std::string    del_req;

    if (hal::utils::hal_trace_level() < hal::utils::trace_debug)  {
        return;
    }
    google::protobuf::util::MessageToJsonString(req, &del_req);
    HAL_TRACE_DEBUG("NAT mapping delete request:");
    HAL_TRACE_DEBUG("{}", del_req.c_str());
    return;
}

//------------------------------------------------------------------------------
// validate a nat mapping delete request
//------------------------------------------------------------------------------
static inline hal_ret_t
validate_nat_mapping_delete_req (NatMappingDeleteRequest& req,
                                 NatMappingDeleteResponse *rsp)
{
    // check if key-handle field is set or not
    if (!req.has_key_or_handle()) {
        HAL_TRACE_ERR("NAT mapping key/handle not set in request");
        rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        return HAL_RET_INVALID_ARG;
    }

    // if key is set, make sure that VRF key/handle is set
    auto kh = req.key_or_handle();
    if (kh.key_or_handle_case() ==  NatMappingKeyHandle::kSvc) {
        if (!kh.svc().has_vrf_kh()) {
            // vrf key/handle not set in the NAT mapping key
            HAL_TRACE_ERR("VRF id/handle missing in NAT mapping key");
            rsp->set_api_status(types::API_STATUS_NAT_POOL_KEY_INVALID);
            return HAL_RET_INVALID_ARG;
        }
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// free hw resources and cleanup the hw tables, if any, for the NAT mapping
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// perform the commit operation for the NAT mapping by deleting from config db
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_delete_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t       ret;
    dllist_ctxt_t   *lnode = NULL;
    dhl_entry_t     *dhl_entry = NULL;
    addr_entry_t    *mapping;
    hal_handle_t    hal_handle;

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    mapping = (addr_entry_t *)dhl_entry->obj;
    hal_handle = dhl_entry->handle;

    ret = nat_mapping_del_from_db(mapping);
    if (ret != HAL_RET_OK) {
        return ret;
    }
    HAL_TRACE_DEBUG("Deleted nat mapping ({}, {}) --> ({}, {}) from cfg db",
                    mapping->key.vrf_id, ipaddr2str(&mapping->key.ip_addr),
                    mapping->tgt_vrf_id, ipaddr2str(&mapping->tgt_ip_addr));

    // free all sw resources associated with this pool
    hal_handle_free(hal_handle);
    nat_mapping_cleanup(mapping);

    return ret;
}

//------------------------------------------------------------------------------
// abort NAT mapping delete operation
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    HAL_TRACE_ERR("Aborting NAT mapping delete operation");
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// cleanup any transient state that we are holding
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_mapping_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// process a NAT mapping delete request
//-----------------------------------------------------------------------------
hal_ret_t
nat_mapping_delete (NatMappingDeleteRequest& req, NatMappingDeleteResponse *rsp)
{
    hal_ret_t        ret;
    addr_entry_t     *mapping;
    cfg_op_ctxt_t    cfg_ctxt = { 0 };
    dhl_entry_t      dhl_entry = { 0 };

    // validate the request message
    ret = validate_nat_mapping_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // get the NAT mapping
    mapping = find_nat_mapping_by_key_or_handle(req.key_or_handle());
    if (mapping == NULL) {
        HAL_TRACE_ERR("Failed to delete NAT mapping, mapping not found");
        nat_mapping_delete_req_dump(req);
        ret = HAL_RET_NAT_MAPPING_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("Deleting NAT mapping ({}, {}) --> ({}, {}) from cfg db",
                    mapping->key.vrf_id, ipaddr2str(&mapping->key.ip_addr),
                    mapping->tgt_vrf_id, ipaddr2str(&mapping->tgt_ip_addr));

    // TODO: handle the case where flows are using this mapping still

    // form ctxt and handover to the HAL infra
    dhl_entry.handle = mapping->hal_handle;
    dhl_entry.obj = mapping;
    cfg_ctxt.app_ctxt = NULL;
    sdk::lib::dllist_reset(&cfg_ctxt.dhl);
    sdk::lib::dllist_reset(&dhl_entry.dllist_ctxt);
    sdk::lib::dllist_add(&cfg_ctxt.dhl, &dhl_entry.dllist_ctxt);
    ret = hal_handle_del_obj(mapping->hal_handle, &cfg_ctxt,
                             nat_mapping_delete_del_cb,
                             nat_mapping_delete_commit_cb,
                             nat_mapping_delete_abort_cb,
                             nat_mapping_delete_cleanup_cb);

end:

    if (ret == HAL_RET_OK) {
        HAL_API_STATS_INC (HAL_API_NAT_MAPPING_DELETE_SUCCESS);
    } else {
        HAL_API_STATS_INC (HAL_API_NAT_MAPPING_DELETE_FAIL);
    }
    rsp->set_api_status(hal_prepare_rsp(ret));
    return ret;
}

//------------------------------------------------------------------------------
// process a get request for a given NAT mapping
//------------------------------------------------------------------------------
static inline void
nat_mapping_process_get (addr_entry_t *mapping, NatMappingGetResponse *rsp)
{
    // fill the spec portion
    auto spec = rsp->mutable_spec();
    auto svc = spec->mutable_key_or_handle()->mutable_svc();
    svc->mutable_vrf_kh()->set_vrf_id(mapping->key.vrf_id);
    ip_addr_to_spec(svc->mutable_ip_addr(), &mapping->key.ip_addr);
    auto pool = spec->mutable_nat_pool();
    pool->mutable_pool_key()->mutable_vrf_kh()->set_vrf_id(mapping->tgt_vrf_id);
    pool->mutable_pool_key()->set_pool_id(mapping->nat_pool_id);
    spec->set_bidir(mapping->bidir == TRUE ? true : false);

    // fill the status portion
    rsp->mutable_status()->set_handle(mapping->hal_handle);
    ip_addr_to_spec(rsp->mutable_status()->mutable_ip_addr(),
                    &mapping->tgt_ip_addr);

    // fill the stats portion
    rsp->mutable_stats()->set_num_tcp_sessions(mapping->num_tcp_sessions);
    rsp->mutable_stats()->set_num_udp_sessions(mapping->num_udp_sessions);
    rsp->mutable_stats()->set_num_other_sessions(mapping->num_other_sessions);

    rsp->set_api_status(types::API_STATUS_OK);
}

//------------------------------------------------------------------------------
// callback invoked from nat mapping hash table while processing get request
//------------------------------------------------------------------------------
static inline bool
nat_mapping_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t    *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    NatMappingGetResponseMsg    *rsp = (NatMappingGetResponseMsg *)ctxt;
    NatMappingGetResponse       *response = rsp->add_response();
    addr_entry_t                *mapping;

    mapping = (addr_entry_t *)hal_handle_get_obj(entry->handle_id);
    nat_mapping_process_get(mapping, response);

    // return false here, so that we don't terminate the walk
    return false;
}

//------------------------------------------------------------------------------
// process a NAT mapping get request
//------------------------------------------------------------------------------
hal_ret_t
nat_mapping_get (NatMappingGetRequest& req, NatMappingGetResponseMsg *rsp)
{
    addr_entry_t    *mapping;

    // if the NAT mapping key-handle field is not set, then this is a request
    // for information for all mappings, so run through all pools in the db and
    // populate the response
    if (!req.has_key_or_handle()) {
        g_hal_state->nat_mapping_ht()->walk(nat_mapping_get_ht_cb, rsp);
    } else {
        auto kh = req.key_or_handle();
        mapping = find_nat_mapping_by_key_or_handle(kh);
        auto response = rsp->add_response();
        if (mapping == NULL) {
            response->set_api_status(types::API_STATUS_NOT_FOUND);
            HAL_API_STATS_INC(HAL_API_NAT_MAPPING_GET_FAIL);
            return HAL_RET_NAT_MAPPING_NOT_FOUND;
        }
        nat_mapping_process_get(mapping, response);
    }

    HAL_API_STATS_INC(HAL_API_NAT_MAPPING_GET_SUCCESS);
    return HAL_RET_OK;
}

hal_ret_t
hal_nat_init_cb (hal_cfg_t *hal_cfg)
{
    return HAL_RET_OK;
}

hal_ret_t
hal_nat_cleanup_cb (void)
{
    return HAL_RET_OK;
}

}    // namespace hal
