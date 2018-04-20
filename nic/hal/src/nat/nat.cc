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
// lookup a NAT pool by its key
//------------------------------------------------------------------------------
nat_pool_t *
nat_pool_lookup_by_key (nat_pool_key_t *key)
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
nat_pool_lookup_by_handle (hal_handle_t handle)
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
// add a natpool to HAL config db
//------------------------------------------------------------------------------
static inline hal_ret_t
nat_pool_add_to_db (nat_pool_t *pool, hal_handle_t handle)
{
    hal_ret_t                   ret;
    sdk_ret_t                   sdk_ret;
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Adding natpool (vrf {}, pool id {}) to cfg db",
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
        HAL_TRACE_ERR("Failed to add natpool (vrf {}, pool id {}) to handle db,"
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
natpool_del_from_db (nat_pool_t *pool)
{
    hal_handle_id_ht_entry_t    *entry;

    HAL_TRACE_DEBUG("Removing from natpool (vrf {}, pool id {}) from cfg db");

    entry =
        (hal_handle_id_ht_entry_t *)
            g_hal_state->nat_pool_id_ht()->remove(&pool->key);

    // free up the hash entry
    if (entry) {
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    } else {
        HAL_TRACE_ERR("natpool (vrf {}, pool id {}) not found");
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
    // chec if key-handle field is set or not
    if (spec.has_key_or_handle()) {
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
nat_pool_init_from_spec (nat_pool_t *pool, const NatPoolSpec& spec)
{
#if 0
    hal_ret_t    ret;

    pool->key.vrf_id = ;
    pool->key.pool_id = ;
#endif
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// process a NAT pool create request
//-----------------------------------------------------------------------------
hal_ret_t
nat_pool_create (NatPoolSpec& spec, NatPoolResponse *rsp)
{
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// process a NAT pool update request
//-----------------------------------------------------------------------------
hal_ret_t
nat_pool_update (NatPoolSpec& spec, NatPoolResponse *rsp)
{
    return HAL_RET_INVALID_OP;
}

//-----------------------------------------------------------------------------
// process a NAT pool delete request
//-----------------------------------------------------------------------------
hal_ret_t
nat_pool_delete (NatPoolDeleteRequest& req,
                 NatPoolDeleteResponse *rsp)
{
    return HAL_RET_OK;
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
