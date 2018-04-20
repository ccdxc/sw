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
#include "nic/hal/src/nat/nat.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/fte/acl/acl.hpp"

using sdk::lib::ht_ctxt_t;
using acl::acl_ctx_t;

namespace hal {

#if 0
//------------------------------------------------------------------------------
// ACL rule matching data structure is used to save the NAT rules config
// The ACL rule is presumably faster than a linear search
//------------------------------------------------------------------------------
static hal_ret_t
nat_rule_spec_validate (nat::NatRuleSpec& spec, nat::NatRuleResponse *rsp)
{
    return HAL_RET_OK;
}

static nat_rule_t *
nat_rule_alloc (void)
{
    nat_rule_t *nat_rule;

    nat_rule = (nat_rule_t *) g_hal_state->nat_rule_slab()->alloc();
    return nat_rule;
}

static hal_ret_t
nat_rule_spec_key_extract (nat::NatRuleSpec& spec, nat_rule_key_t *rule_key)
{
    rule_key->rule_id = spec.key_or_handle().rule_id();
    rule_key->vrf_id = spec.key_or_handle().vrf_id();
        nwsec_policy->key.vrf_id = spec.policy_key_or_handle().security_policy_key().vrf_id_or_handle().vrf_id();
}

static hal_ret_t
nat_rule_spec_extract (nat::NatRuleSpec& spec, nat_rule_t *rule)
{
    nat_rule_spec_key_extract(spec, &rule->key);
    return HAL_RET_OK;
}

static hal_ret_t
nat_rule_db_add (nat_rule_t *rule)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_rule_create (nat::NatRuleSpec& spec, nat::NatRuleResponse *rsp)
{
    hal_ret_t     ret;
    nat_rule_t    *rule;

    if ((ret = nat_rule_spec_validate(spec, rsp)) != HAL_RET_OK)
        goto end;

    if ((rule = nat_rule_alloc()) == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    if ((ret = nat_rule_spec_extract(spec, rule)) != HAL_RET_OK)
        goto end;

    if ((ret = nat_rule_db_add(rule)) != HAL_RET_OK)
        goto end;

end:

    return HAL_RET_OK;

}

//------------------------------------------------------------------------------
// process a nat rule update request
//------------------------------------------------------------------------------
hal_ret_t
nat_rule_update (NatRuleSpec& spec, NatRuleResponse *rsp)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a nat rule get request
//------------------------------------------------------------------------------
hal_ret_t
nat_rule_get (NatRuleGetRequest& req, NatRuleGetResponseMsg *rsp)
{
    return HAL_RET_OK;
}
#endif

//------------------------------------------------------------------------------
// hash table for nat pool id to nat pool
//------------------------------------------------------------------------------
void *
nat_pool_id_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t    *ht_entry = NULL;
    nat_pool_t                  *nat_pool = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    nat_pool = (nat_pool_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(nat_pool->key);
}

//------------------------------------------------------------------------------
// hash computation for nat pool
//------------------------------------------------------------------------------
uint32_t
nat_pool_id_compute_hash_func (void *key, uint32_t ht_size)
{
    HAL_ASSERT(key != NULL);
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(nat_pool_key_t)) % ht_size;
}

//------------------------------------------------------------------------------
// key comparision for nat pool hash table
//------------------------------------------------------------------------------
bool
nat_pool_id_compare_key_func (void *key1, void *key2)
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

static inline hal_ret_t
nat_pool_cleanup (nat_pool_t *pool)
{
    // TODO: purge all NAT binding of this pool
    nat_pool_free(pool);
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// lookup a NAT pool by its key
//------------------------------------------------------------------------------
nat_pool_t *
find_nat_pool_by_key (nat_pool_key_t *key)
{
    hal_handle_id_ht_entry_t    *handle_id_entry;
    hal_handle                  *handle;

    handle_id_entry =
        (hal_handle_id_ht_entry_t *)g_hal_state->nat_pool_id_ht()->lookup(key);
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
nat_pool_t *
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
// lookup a NAT pool by its key
//------------------------------------------------------------------------------
static nat_pool_t *
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

    // add mapping from vrf id to its handle
    entry->handle_id = handle;
    sdk_ret = g_hal_state->nat_pool_id_ht()->insert_with_key(&pool->key,
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

    HAL_TRACE_DEBUG("Removing from natpool ({}, {}) from cfg db",
                    pool->key.vrf_id, pool->key.pool_id);
    entry =
        (hal_handle_id_ht_entry_t *)
            g_hal_state->nat_pool_id_ht()->remove(&pool->key);

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
static void
nat_pool_spec_dump (NatPoolSpec& spec)
{
    std::string    nat_pool_cfg;

    google::protobuf::util::MessageToJsonString(spec, &nat_pool_cfg);
    HAL_TRACE_DEBUG("NAT Pool Configuration received:");
    HAL_TRACE_DEBUG("{}", nat_pool_cfg.c_str());
    return;
}

//-----------------------------------------------------------------------------
// validate NAT pool configuration
//-----------------------------------------------------------------------------
static hal_ret_t
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

    if (!kh.pool_key().has_vrf_id_or_handle()) {
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
static hal_ret_t
nat_pool_init_from_spec (vrf_t *vrf, nat_pool_t *pool, const NatPoolSpec& spec)
{
    int                       i;
    hal_ret_t                 ret;
    addr_range_list_elem_t    *addr_range;
    dllist_ctxt_t             *curr, *next;

    pool->key.vrf_id = vrf->vrf_id;
    pool->key.pool_id = spec.key_or_handle().pool_handle();
    for (i = 0; i < spec.address_size(); i++) {
        addr_range = NULL;
        auto addr = spec.address(i);
        if (addr.has_range()) {
            auto range = addr.range();
            if (range.has_ipv4_range()) {
                addr_range =
                    (addr_range_list_elem_t *)g_hal_state->v4_range_list_entry_slab()->alloc();
                if (addr_range == NULL) {
                    ret = HAL_RET_OOM;
                    goto cleanup;
                }
                sdk::lib::dllist_reset(&addr_range->list_ctxt);
                addr_range->ip_range.af = IP_AF_IPV4;
                addr_range->ip_range.vx_range[0].v4_range.ip_lo =
                    range.ipv4_range().low_ipaddr().v4_addr();
                addr_range->ip_range.vx_range[0].v4_range.ip_hi =
                    range.ipv4_range().high_ipaddr().v4_addr();
            } else if (range.has_ipv6_range()) {
                HAL_TRACE_ERR("IPv6 NAT range/subnets not supported, skipping");
                continue;
            } else {
                HAL_TRACE_ERR("No IPv4/IPv6 range found in address obj"
                              "in NAT pool ({}, {}), skipping",
                              pool->key.vrf_id, pool->key.pool_id);
                continue;
            }
        } else if (addr.has_prefix()) {
            // convert this subnet into a range
            auto prefix = addr.prefix();
            if (prefix.has_ipv4_subnet()) {
                addr_range =
                    (addr_range_list_elem_t *)g_hal_state->v4_range_list_entry_slab()->alloc();
                if (addr_range == NULL) {
                    ret = HAL_RET_OOM;
                    goto cleanup;
                }
                sdk::lib::dllist_reset(&addr_range->list_ctxt);
                addr_range->ip_range.af = IP_AF_IPV4;
                addr_range->ip_range.vx_range[0].v4_range.ip_lo =
                    prefix.ipv4_subnet().address().v4_addr() &
                    ~((1 << (32 - prefix.ipv4_subnet().prefix_len())) - 1);
                addr_range->ip_range.vx_range[0].v4_range.ip_hi =
                    addr_range->ip_range.vx_range[0].v4_range.ip_lo +
                    (1 << (32 - prefix.ipv4_subnet().prefix_len())) - 1;
            } else if (prefix.has_ipv6_subnet()) {
                HAL_TRACE_ERR("IPv6 NAT range/subnets not supported, skipping");
                continue;
            } else {
                HAL_TRACE_ERR("No IPv4/IPv6 subnet found in address obj"
                              "in NAT pool ({}, {}), skipping",
                              pool->key.vrf_id, pool->key.pool_id);
                continue;
            }
        } else {
            HAL_TRACE_ERR("Skipping empty addr range/pfx in NAT pool ({}, {})",
                          pool->key.vrf_id, pool->key.pool_id);
            continue;
        }
        if (addr_range) {
            // add this range to the NAT pool
            dllist_add_tail(&pool->addr_ranges, &addr_range->list_ctxt);
        }
    }

    return HAL_RET_OK;

cleanup:

    dllist_for_each_safe(curr, next, &pool->addr_ranges) {
        addr_range = dllist_entry(curr, addr_range_list_elem_t, list_ctxt);
        sdk::lib::dllist_del(&addr_range->list_ctxt);
        if (addr_range->ip_range.af == IP_AF_IPV4) {
             hal::delay_delete_to_slab(HAL_SLAB_V4_RANGE_LIST_ENTRY,
                                       addr_range);
        } else {
             hal::delay_delete_to_slab(HAL_SLAB_V6_RANGE_LIST_ENTRY,
                                       addr_range);
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// allocate hw resources and program the hw tables, if any, for the NAT pool
//------------------------------------------------------------------------------
static hal_ret_t
nat_pool_create_add_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// perform the commit operation for the NAT pool by adding to config db and
// making this config available
//------------------------------------------------------------------------------
static hal_ret_t
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
static hal_ret_t
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
static hal_ret_t
nat_pool_create_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// convert hal_ret_t to API status to return to the app
//-----------------------------------------------------------------------------
static hal_ret_t
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

    auto pool_key = spec.key_or_handle().pool_key();
    // validate the request message
    ret = validate_nat_pool_create(spec, rsp);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("NAT pool config validation failed, err : {}", ret);
        goto end;
    }

    vrf = vrf_lookup_key_or_handle(pool_key.vrf_id_or_handle());
    if (vrf == NULL) {
        HAL_TRACE_ERR("Failed to find vrf with key {}/handle {}",
                      pool_key.vrf_id_or_handle().vrf_id(),
                      pool_key.vrf_id_or_handle().vrf_handle());
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }

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
        if (!kh.pool_key().has_vrf_id_or_handle()) {
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
static hal_ret_t
nat_pool_delete_del_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// perform the commit operation for the NAT pool by deleting from config db
//------------------------------------------------------------------------------
static hal_ret_t
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
static hal_ret_t
nat_pool_delete_abort_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    HAL_TRACE_ERR("Aborting NAT pool delete operation");
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// cleanup any transient state that we are holding
//------------------------------------------------------------------------------
static hal_ret_t
nat_pool_delete_cleanup_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// process a NAT pool delete request
//-----------------------------------------------------------------------------
hal_ret_t
nat_pool_delete (NatPoolDeleteRequest& req,
                 NatPoolDeleteResponse *rsp)
{
    hal_ret_t        ret;
    vrf_t            *vrf;
    nat_pool_t       *pool;
    cfg_op_ctxt_t    cfg_ctxt = { 0 };
    dhl_entry_t      dhl_entry = { 0 };

    auto pool_key = req.key_or_handle().pool_key();
    // validate the request message
    ret = validate_nat_pool_delete_req(req, rsp);
    if (ret != HAL_RET_OK) {
        goto end;
    }

    // get the VRF this nat pool belongs to
    vrf = vrf_lookup_key_or_handle(pool_key.vrf_id_or_handle());
    if (vrf == NULL) {
        HAL_TRACE_ERR("Failed to find vrf with key {}/handle {}",
                      pool_key.vrf_id_or_handle().vrf_id(),
                      pool_key.vrf_id_or_handle().vrf_handle());
        ret = HAL_RET_VRF_NOT_FOUND;
        goto end;
    }

    // get the NAT pool
    pool = find_nat_pool(vrf->vrf_id, req.key_or_handle());
    if (pool == NULL) {
        HAL_TRACE_ERR("Failed to delete natpool ({}, {}), pool not found",
                      vrf->vrf_id, pool_key.pool_id());
        ret = HAL_RET_NAT_POOL_NOT_FOUND;
        goto end;
    }
    HAL_TRACE_DEBUG("Deleting natpool ({}, {})",
                    vrf->vrf_id, pool_key.pool_id());

    // TODO: check if there are NAT Mappings outstanding

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

hal_ret_t
nat_pool_get (NatPoolGetRequest& req,
              NatPoolGetResponseMsg *rsp)
{
    return HAL_RET_OK;
}

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

hal_ret_t
nat_vpn_mapping_create (NatVpnMappingSpec& spec,
                        NatVpnMappingResponse *rsp)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_vpn_mapping_get (NatVpnMappingGetRequest& req,
                     NatVpnMappingGetResponseMsg *rsp)
{
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
