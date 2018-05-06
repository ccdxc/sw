//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
// Handles Config APIs for NAT Policy
//-----------------------------------------------------------------------------

#include <google/protobuf/util/json_util.h>
#include "nic/include/hal_cfg.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/fte/acl/acl.hpp"
#include "nic/hal/src/nw/vrf.hpp"
#include "nic/hal/src/utils/cfg_op_ctxt.hpp"
#include "nat.hpp"


using sdk::lib::ht_ctxt_t;
using acl::acl_ctx_t;

namespace hal {

void *
nat_policy_get_key_func (void *entry)
{
    hal_handle_id_ht_entry_t *ht_entry;
    nat_cfg_pol_t *policy = NULL;

    HAL_ASSERT(entry != NULL);
    ht_entry = (hal_handle_id_ht_entry_t *)entry;
    if (ht_entry == NULL) {
        return NULL;
    }
    policy = (nat_cfg_pol_t *)hal_handle_get_obj(ht_entry->handle_id);
    return (void *)&(policy->key);
}

uint32_t
nat_policy_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key,
               sizeof(nat_cfg_pol_key_t)) % ht_size;
}

bool
nat_policy_compare_key_func (void *key1, void *key2)
{
    dllist_ctxt_t    lentry;

    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (!memcmp(key1, key2, sizeof(nat_cfg_pol_key_t))) {
        return true;
    }
    return false;
}

static inline nat_cfg_pol_t *
nat_cfg_pol_alloc (void)
{
    return ((nat_cfg_pol_t *)g_hal_state->nat_cfg_pol_slab()->alloc());
}

static inline void
nat_cfg_pol_free (nat_cfg_pol_t *pol)
{
    hal::delay_delete_to_slab(HAL_SLAB_NAT_CFG_POL, pol);
}

static inline void
nat_cfg_pol_init (nat_cfg_pol_t *pol)
{
    HAL_SPINLOCK_INIT(&pol->slock, PTHREAD_PROCESS_SHARED);
    dllist_reset(&pol->rule_list);
    pol->ht_ctxt.reset();
    pol->hal_hdl = HAL_HANDLE_INVALID;
}

static inline void
nat_cfg_pol_uninit (nat_cfg_pol_t *pol)
{
    HAL_SPINLOCK_DESTROY(&pol->slock);
}

static inline nat_cfg_pol_t *
nat_cfg_pol_alloc_init (void)
{
    nat_cfg_pol_t *pol;

    if ((pol = nat_cfg_pol_alloc()) ==  NULL)
        return NULL;

    nat_cfg_pol_init(pol);
    return pol;
}

static inline void
nat_cfg_pol_uninit_free (nat_cfg_pol_t *pol)
{
    if (pol) {
        nat_cfg_pol_uninit(pol);
        nat_cfg_pol_free(pol);
    }
}

//-----------------------------------------------------------------------------
// Configuration handling
//-----------------------------------------------------------------------------

static inline void
nat_cfg_pol_db_add (dllist_ctxt_t *head, nat_cfg_pol_t *pol)
{
    dllist_add_tail(head, &pol->list_ctxt);
}

static inline void
nat_cfg_pol_db_del (nat_cfg_pol_t *pol)
{
    dllist_del(&pol->list_ctxt);
}

static inline hal_ret_t
add_nat_cfg_pol_to_db (nat_cfg_pol_t *policy)
{
    hal_ret_t ret;
    sdk_ret_t sdk_ret;
    hal_handle_id_ht_entry_t *entry;

    entry =
        (hal_handle_id_ht_entry_t *)
            g_hal_state->hal_handle_id_ht_entry_slab()->alloc();
    if (entry == NULL) {
        HAL_TRACE_ERR("Failed to add NAT policy to cfg db, no memory");
        return HAL_RET_OOM;
    }

    entry->handle_id = policy->hal_hdl;
    sdk_ret = g_hal_state->nat_policy_ht()->insert_with_key(&policy->key,
                                                            entry,
                                                            &policy->ht_ctxt);
    ret = hal_sdk_ret_to_hal_ret(sdk_ret);
    if (sdk_ret != sdk::SDK_RET_OK) {
        HAL_TRACE_ERR("Failed to add NAT policy ({}, {}) to cfg db, err : {}",
                      policy->key.vrf_id, policy->key.pol_id, ret);
        hal::delay_delete_to_slab(HAL_SLAB_HANDLE_ID_HT_ENTRY, entry);
    }
    return ret;
}

static hal_ret_t
nat_cfg_pol_rule_spec_extract (nat::NatPolicySpec& spec, nat_cfg_pol_t *pol)
{
    hal_ret_t ret = HAL_RET_OK;

    for (int i = 0; i < spec.rules_size(); i++) {
        if ((ret = nat_cfg_rule_spec_handle(
               spec.rules(i), &pol->list_ctxt)) != HAL_RET_OK)
            return ret;
    }

    return ret;
}

hal_ret_t
nat_cfg_pol_data_spec_extract (nat::NatPolicySpec& spec, nat_cfg_pol_t *pol)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = nat_cfg_pol_rule_spec_extract(spec, pol)) != HAL_RET_OK)
        return ret;

    return ret;
}

static hal_ret_t
nat_cfg_pol_key_spec_extract (nat::NatPolicySpec& spec, nat_cfg_pol_key_t *key)
{
    key->pol_id = spec.key_or_handle().policy_key().nat_policy_id();

    if (spec.key_or_handle().policy_key().
        vrf_key_or_handle().key_or_handle_case() == kh::VrfKeyHandle::kVrfId) {
        key->vrf_id = spec.key_or_handle().policy_key().
            vrf_key_or_handle().vrf_id();
    } else {
        vrf_t *vrf = vrf_lookup_by_handle(
            spec.key_or_handle().policy_key().
            vrf_key_or_handle().vrf_handle());
        if (!vrf)
            return HAL_RET_HANDLE_INVALID;
        key->vrf_id = vrf->vrf_id;
    }
    return HAL_RET_OK;
}

static hal_ret_t
nat_cfg_pol_spec_extract (nat::NatPolicySpec& spec, nat_cfg_pol_t *pol)
{
    hal_ret_t ret;

    if ((ret = nat_cfg_pol_key_spec_extract(spec, &pol->key)) != HAL_RET_OK)
        return ret;

    if ((ret = nat_cfg_pol_data_spec_extract(spec, pol)) != HAL_RET_OK)
        return ret;

    return ret;
}

//-----------------------------------------------------------------------------
// dump nat policy spec
//-----------------------------------------------------------------------------
void
nat_cfg_pol_dump (nat::NatPolicySpec& spec)
{
    std::string    nat_pol_cfg;

    if (hal::utils::hal_trace_level() < hal::utils::trace_debug) {
        return;
    }
    google::protobuf::util::MessageToJsonString(spec, &nat_pol_cfg);
    HAL_TRACE_DEBUG("NAT policy configuration:");
    HAL_TRACE_DEBUG("{}", nat_pol_cfg.c_str());
}

static hal_ret_t
nat_cfg_pol_spec_validate (nat::NatPolicySpec& spec)
{
    return HAL_RET_OK;
}

hal_ret_t
nat_cfg_pol_create_cfg_handle (nat::NatPolicySpec& spec,
                               nat_cfg_pol_t **out_pol)
{
    hal_ret_t ret = HAL_RET_OK;
    nat_cfg_pol_t *pol;

    if ((ret = nat_cfg_pol_spec_validate(spec)) != HAL_RET_OK)
        return ret;

    if ((pol = nat_cfg_pol_alloc_init()) == NULL)
        return HAL_RET_OOM;

    if ((ret = nat_cfg_pol_spec_extract(spec, pol)) != HAL_RET_OK)
        return ret;

    *out_pol = pol;
    return HAL_RET_OK;
}

void
nat_cfg_pol_rsp_build (nat::NatPolicyResponse *rsp, hal_ret_t ret,
                       hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK)
        rsp->mutable_policy_status()->set_nat_policy_handle(hal_handle);
    rsp->set_api_status(hal_prepare_rsp(ret));
    return;
}

static hal_ret_t
nat_cfg_pol_rule_spec_build (nat::NatPolicyGetRequest& req,
                             nat::NatPolicyGetResponseMsg *res)
{
    return HAL_RET_OK;
}

static hal_ret_t
nat_cfg_pol_spec_build (nat::NatPolicyGetRequest& req,
                        nat::NatPolicyGetResponseMsg *res)
{
    hal_ret_t ret;

    if ((ret = nat_cfg_pol_rule_spec_build(req, res)) != HAL_RET_OK)
        return ret;

    return ret;
}

hal_ret_t
nat_cfg_pol_get_cfg_handle (NatPolicyGetRequest& req,
                            NatPolicyGetResponseMsg *res)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = nat_cfg_pol_spec_build(req, res)) != HAL_RET_OK)
        return ret;

    return ret;
}

//-----------------------------------------------------------------------------
// Operational handling (hal handles, acl libs, etc)
//-----------------------------------------------------------------------------

static inline hal_ret_t
nat_cfg_pol_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t ret = HAL_RET_INVALID_ARG;
    nat_cfg_pol_t *policy;
    nat_cfg_pol_create_app_ctxt_t *app_ctx = NULL;
    hal_handle_t hal_handle;
    dllist_ctxt_t *lnode;
    dhl_entry_t *dhl_entry;

    if (!cfg_ctxt || !cfg_ctxt->app_ctxt)
        goto end;

    app_ctx = (nat_cfg_pol_create_app_ctxt_t *) cfg_ctxt->app_ctxt;

    if ((ret = acl::acl_commit(app_ctx->acl_ctx)) != HAL_RET_OK)
        goto end;

    acl_deref(app_ctx->acl_ctx);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    hal_handle = dhl_entry->handle;
    policy = (nat_cfg_pol_t *) dhl_entry->obj;
    HAL_TRACE_DEBUG("policy key ({}, {}), handle {}",
                    policy->key.vrf_id, policy->key.pol_id, hal_handle);
    ret = add_nat_cfg_pol_to_db(policy);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add NAT policy ({}, {}) to db, err : {}",
                      policy->key.vrf_id, policy->key.pol_id, ret);
        goto end;
    }

end:

    if (ret != HAL_RET_OK) {
        //todo: free resources
    }
    return ret;
}

static hal_ret_t
nat_cfg_pol_rule_oper_handle (
    nat_cfg_pol_t *pol, nat_cfg_pol_create_app_ctxt_t *app_ctxt)
{
    hal_ret_t ret = HAL_RET_OK;
#if 0
    nat_cfg_rule_t *rule;
    dllist_ctxt_t *entry;

    dllist_for_each(entry, &pol->rule_list) {
        rule = dllist_entry(entry, nat_cfg_rule_t, list_ctxt);
        if ((ret = nat_cfg_rule_create_oper_handle(
               rule, app_ctxt)) != HAL_RET_OK)
            return ret;
    }
#endif
    return ret;
}

static inline void
nat_cfg_pol_oper_init (nat_cfg_pol_t *pol, hal_handle_t hal_hdl)
{
    pol->hal_hdl = hal_hdl;
}

hal_ret_t
nat_cfg_pol_create_oper_handle (nat_cfg_pol_t *pol)
{
    hal_ret_t ret;
    hal_handle_t hal_hdl;
    nat_cfg_pol_create_app_ctxt_t app_ctxt = { 0 };

    app_ctxt.acl_ctx = nat_cfg_pol_create_app_ctxt_init(pol);

    if ((ret = cfg_ctxt_op_create_handle(HAL_OBJ_ID_NAT_POLICY, pol, &app_ctxt,
                                         hal_cfg_op_null_cb,
                                         nat_cfg_pol_create_commit_cb,
                                         hal_cfg_op_null_cb,
                                         hal_cfg_op_null_cb,
                                         &hal_hdl)) != HAL_RET_OK)
        return ret;

    nat_cfg_pol_oper_init(pol, hal_hdl);

    if ((ret = nat_cfg_pol_rule_oper_handle(pol, &app_ctxt)) != HAL_RET_OK)
        return ret;

    return HAL_RET_OK;
}

}  // namespace hal
