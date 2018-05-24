#include "nic/include/base.h"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/hal/src/nw/session.hpp"
#include "nic/include/fte.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/src/ipsec/ipsec.hpp"
#include "nic/hal/src/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/export/vrf_api.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include <google/protobuf/util/json_util.h>
#include "nic/include/hal_cfg.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/fte/acl/acl.hpp"
#include "nic/hal/src/nw/vrf.hpp"
#include "nic/hal/src/utils/cfg_op_ctxt.hpp"
#include "nic/gen/hal/include/hal_api_stats.hpp"

namespace hal {
void *
ipsec_rule_get_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((ipsec_rule_t *)entry)->rule_id);
}

uint32_t
ipsec_rule_compute_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(ipsec_rule_id_t)) % ht_size;
}

bool
ipsec_rule_compare_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(ipsec_rule_id_t *)key1 == *(ipsec_rule_id_t *)key2) {
        return true;
    }
    return false;
}

void *
ipsec_rule_get_handle_key_func (void *entry)
{
    HAL_ASSERT(entry != NULL);
    return (void *)&(((ipsec_rule_t *)entry)->hal_handle);
}

uint32_t
ipsec_rule_compute_handle_hash_func (void *key, uint32_t ht_size)
{
    return sdk::lib::hash_algo::fnv_hash(key, sizeof(hal_handle_t)) % ht_size;
}

bool
ipsec_rule_compare_handle_key_func (void *key1, void *key2)
{
    HAL_ASSERT((key1 != NULL) && (key2 != NULL));
    if (*(hal_handle_t *)key1 == *(hal_handle_t *)key2) {
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
// Configuration handling
//-----------------------------------------------------------------------------

void
ipsec_cfg_pol_dump (ipsec::IpsecRuleSpec& spec)
{
    std::string pol_str;

    if (hal::utils::hal_trace_level() < hal::utils::trace_debug)
        return;

    google::protobuf::util::MessageToJsonString(spec, &pol_str);
    HAL_TRACE_DEBUG("IPSec Rules policy configuration:");
    HAL_TRACE_DEBUG("{}", pol_str.c_str());
}

static hal_ret_t
ipsec_cfg_pol_rule_spec_extract (ipsec::IpsecRuleSpec& spec, ipsec_cfg_pol_t *pol)
{
    hal_ret_t ret = HAL_RET_OK;

    for (int i = 0; i < spec.rules_size(); i++) {
        if ((ret = ipsec_cfg_rule_spec_handle(
               spec.rules(i), &pol->rule_list)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
            return ret;
        }
    }

    return ret;
}

hal_ret_t
ipsec_cfg_pol_data_spec_extract (ipsec::IpsecRuleSpec& spec, ipsec_cfg_pol_t *pol)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = ipsec_cfg_pol_rule_spec_extract(spec, pol)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    return ret;
}

static hal_ret_t
ipsec_cfg_pol_key_spec_extract (ipsec::IpsecRuleSpec& spec, ipsec_cfg_pol_key_t *key)
{
    vrf_t *vrf;

    key->pol_id = spec.key_or_handle().rule_key().ipsec_rule_id();

    if (spec.key_or_handle().rule_key().
        vrf_key_or_handle().key_or_handle_case() == kh::VrfKeyHandle::kVrfId) {
        key->vrf_id = spec.key_or_handle().rule_key().
            vrf_key_or_handle().vrf_id();

        if ((vrf = vrf_lookup_by_id(key->vrf_id)) == NULL) {
        HAL_TRACE_DEBUG("Failed here");
            return  HAL_RET_VRF_NOT_FOUND;
        }
    } else {
        if ((vrf = vrf_lookup_by_handle(spec.key_or_handle().rule_key().
                vrf_key_or_handle().vrf_handle())) == NULL) {
        HAL_TRACE_DEBUG("Failed here");
            return HAL_RET_VRF_NOT_FOUND;
         }

        key->vrf_id = vrf->vrf_id;
    }
    return HAL_RET_OK;
}

static hal_ret_t
ipsec_cfg_pol_spec_extract (ipsec::IpsecRuleSpec& spec, ipsec_cfg_pol_t *pol)
{
    hal_ret_t ret;

    if ((ret = ipsec_cfg_pol_key_spec_extract(spec, &pol->key)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    if ((ret = ipsec_cfg_pol_data_spec_extract(spec, pol)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    return ret;
}

//------------------------------------------------------------------------------
// validate an incoming IPSECCB create request
// TODO:
// 1. check if IPSECCB exists already
//------------------------------------------------------------------------------
static inline hal_ret_t
validate_ipsec_rule_create (IpsecRuleSpec& spec)
{
    // must have key-handle set
    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{}: no key or handle for create request", __FUNCTION__);
        return HAL_RET_INVALID_ARG;
    }

    if (spec.key_or_handle().rule_handle() != HAL_HANDLE_INVALID) {
        HAL_TRACE_ERR("{}: handle set for create request", __FUNCTION__);
        return HAL_RET_INVALID_ARG;
    }
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// insert this IPSEC CB in all meta data structures
//------------------------------------------------------------------------------
static inline hal_ret_t
add_ipsec_rule_to_db (ipsec_rule_t *ipsec)
{
    //g_hal_state->ipsec_rule_id_ht()->insert(ipsec, &ipsec->ht_ctxt);
    return HAL_RET_OK;
}

static hal_ret_t
ipsec_cfg_pol_spec_validate (ipsec::IpsecRuleSpec& spec, bool create)
{
    hal_ret_t ret;

    if (!spec.has_key_or_handle()) {
        HAL_TRACE_ERR("{}: no key information set in request", __FUNCTION__);
        return HAL_RET_INVALID_ARG;
    }

    if (create) {
        if ((ret = validate_ipsec_rule_create(spec)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
            return ret;
        }
    }

    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// IPSec config alloc and init routines
//-----------------------------------------------------------------------------

static inline ipsec_cfg_pol_t *
ipsec_cfg_pol_alloc (void)
{
    return ((ipsec_cfg_pol_t *)g_hal_state->ipsec_cfg_pol_slab()->alloc());
}

static inline void
ipsec_cfg_pol_free (ipsec_cfg_pol_t *pol)
{
    hal::delay_delete_to_slab(HAL_SLAB_IPSEC_CFG_POL, pol);
}

static inline void
ipsec_cfg_pol_init (ipsec_cfg_pol_t *pol)
{
    HAL_SPINLOCK_INIT(&pol->slock, PTHREAD_PROCESS_SHARED);
    dllist_reset(&pol->rule_list);
    pol->ht_ctxt.reset();
    pol->hal_hdl = HAL_HANDLE_INVALID;
}

static inline void
ipsec_cfg_pol_uninit (ipsec_cfg_pol_t *pol)
{
    HAL_SPINLOCK_DESTROY(&pol->slock);
}

static inline ipsec_cfg_pol_t *
ipsec_cfg_pol_alloc_init (void)
{
    ipsec_cfg_pol_t *pol;

    if ((pol = ipsec_cfg_pol_alloc()) ==  NULL) {
        HAL_TRACE_DEBUG("Failed here");
        return NULL;
    }

    ipsec_cfg_pol_init(pol);
    return pol;
}

static inline void
ipsec_cfg_pol_uninit_free (ipsec_cfg_pol_t *pol)
{
    if (pol) {
        ipsec_cfg_pol_uninit(pol);
        ipsec_cfg_pol_free(pol);
    }
}

hal_ret_t
ipsec_cfg_pol_create_cfg_handle (ipsec::IpsecRuleSpec& spec,
                                 ipsec_cfg_pol_t **out_pol)
{
    hal_ret_t ret = HAL_RET_OK;
    ipsec_cfg_pol_t *pol;

    if ((ret = ipsec_cfg_pol_spec_validate(spec, true)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    if ((pol = ipsec_cfg_pol_alloc_init()) == NULL) {
        HAL_TRACE_DEBUG("Failed here");
        return HAL_RET_OOM;
    }

    if ((ret = ipsec_cfg_pol_spec_extract(spec, pol)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    *out_pol = pol;
    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB create request
// TODO: if IPSEC CB exists, treat this as modify (vrf id in the meta must
// match though)
//------------------------------------------------------------------------------
hal_ret_t
ipsec_rule_create (IpsecRuleSpec& spec, IpsecRuleResponse *rsp)
{
    hal_ret_t ret;
    ipsec_cfg_pol_t *pol = NULL;

    ipsec_cfg_pol_dump(spec);
    if ((ret = ipsec_cfg_pol_create_cfg_handle(spec, &pol)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        goto end;
    }

    if ((ret = ipsec_cfg_pol_create_oper_handle(pol)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        goto end;
    }

end:
    ipsec_cfg_pol_rsp_build(rsp, ret, pol ? pol->hal_hdl : HAL_HANDLE_INVALID);
    return ret;
}

//------------------------------------------------------------------------------
// process a IPSEC CB update request
//------------------------------------------------------------------------------
hal_ret_t
ipsec_rule_update (IpsecRuleSpec& spec, IpsecRuleResponse *rsp)
{
    return HAL_RET_OK;
}

void
ipsec_cfg_pol_rsp_build (ipsec::IpsecRuleResponse *rsp, hal_ret_t ret,
                         hal_handle_t hal_handle)
{
    if (ret == HAL_RET_OK)
        rsp->mutable_status()->set_handle(hal_handle);
    rsp->set_api_status(hal_prepare_rsp(ret));
}

hal_ret_t
ipsec_cfg_pol_rule_spec_build (ipsec_cfg_pol_t *pol,
                               ipsec::IpsecRuleSpec *spec)
{
    hal_ret_t        ret;
    dllist_ctxt_t    *entry;
    ipsec_cfg_rule_t *rule;

    dllist_for_each(entry, &pol->rule_list) {
        rule = (ipsec_cfg_rule_t  *)((char *)entry -
                    (sizeof(ipsec_cfg_rule_action_t) + sizeof(rule_match_t) + sizeof(ipsec_cfg_rule_key_t)));
        auto rule_spec = spec->add_rules();
        if ((ret = ipsec_cfg_rule_spec_build(
               rule, rule_spec)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
            return ret;
        }
    }

    return HAL_RET_OK;
}

static hal_ret_t
ipsec_cfg_pol_spec_build (ipsec_cfg_pol_t *pol,
                          ipsec::IpsecRuleSpec *spec)
{
    hal_ret_t           ret;
    ipsec_cfg_pol_key_t *key;

    key = &(pol->key);
    spec->mutable_key_or_handle()->mutable_rule_key()->mutable_vrf_key_or_handle()->set_vrf_id(key->vrf_id);
    spec->mutable_key_or_handle()->mutable_rule_key()->set_ipsec_rule_id(key->pol_id);
    spec->mutable_key_or_handle()->set_rule_handle(pol->hal_hdl);

    if ((ret = ipsec_cfg_pol_rule_spec_build(pol, spec)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    return ret;
}

hal_ret_t
ipsec_cfg_pol_get_cfg_handle (ipsec_cfg_pol_t *pol,
                              ipsec::IpsecRuleGetResponse *response)
{
    hal_ret_t ret = HAL_RET_OK;
    auto spec = response->mutable_spec();

    if ((ret = ipsec_cfg_pol_spec_build(pol, spec)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    return ret;
}

//------------------------------------------------------------------------------
// lookup a IPSec policy by its handle
//------------------------------------------------------------------------------
static inline ipsec_cfg_pol_t *
ipsec_find_policy_by_handle (hal_handle_t handle)
{
    if (handle == HAL_HANDLE_INVALID) {
        return NULL;
    }
    auto hal_handle = hal_handle_get_from_handle_id(handle);
    if (!hal_handle) {
        HAL_TRACE_ERR("Failed to find object with handle : {}", handle);
        return NULL;
    }

    if (hal_handle->obj_id() != HAL_OBJ_ID_IPSEC_POLICY) {
        HAL_TRACE_ERR("Object id mismatch for handle {}, obj id found {}",
                        handle, hal_handle->obj_id());
        return NULL;
    }

    return (ipsec_cfg_pol_t *)hal_handle_get_obj(handle);
}

//------------------------------------------------------------------------------
// lookup a IPSec policy by key-handle spec
//------------------------------------------------------------------------------
static inline ipsec_cfg_pol_t *
ipsec_find_policy_by_key_or_handle (const IpsecRuleKeyHandle& kh)
{
    if (kh.has_rule_key()) {
        ipsec_cfg_pol_key_t key = {0};

        key.pol_id = kh.rule_key().ipsec_rule_id();
        key.vrf_id = kh.rule_key().vrf_key_or_handle().vrf_id();
        
        return (ipsec_cfg_pol_t *) g_hal_state->ipsec_policy_ht()->lookup((void *)&key);
    } else {
        return ipsec_find_policy_by_handle(kh.rule_handle());
    }
}

//------------------------------------------------------------------------------
// callback invoked from ipsec policy hash table while processing get request
//------------------------------------------------------------------------------
static inline bool
ipsec_policy_get_ht_cb (void *ht_entry, void *ctxt)
{
    hal_handle_id_ht_entry_t    *entry = (hal_handle_id_ht_entry_t *)ht_entry;
    ipsec::IpsecRuleGetResponseMsg     *rsp = (ipsec::IpsecRuleGetResponseMsg *)ctxt;
    ipsec::IpsecRuleGetResponse *response = rsp->add_response();
    ipsec_cfg_pol_t             *pol;

    pol = (ipsec_cfg_pol_t *)hal_handle_get_obj(entry->handle_id);
    ipsec_cfg_pol_get_cfg_handle(pol, response);

    // return false here, so that we don't terminate the walk
    return false;
}

//------------------------------------------------------------------------------
// process a IPSEC CB get request
//------------------------------------------------------------------------------
hal_ret_t
ipsec_rule_get (IpsecRuleGetRequest& req, IpsecRuleGetResponseMsg *rsp)
{
    hal_ret_t        ret;
    ipsec_cfg_pol_t  *pol;

    if (!req.has_key_or_handle()) {
        g_hal_state->ipsec_policy_ht()->walk(ipsec_policy_get_ht_cb, rsp);
    } else {
        auto kh = req.key_or_handle();
        pol = ipsec_find_policy_by_key_or_handle(kh);
        if (pol == NULL) {
            auto response = rsp->add_response();
            response->set_api_status(types::API_STATUS_NOT_FOUND);
            HAL_API_STATS_INC(HAL_API_IPSEC_RULE_GET_FAIL);
        HAL_TRACE_DEBUG("Failed here");
            return HAL_RET_IPSEC_RULE_NOT_FOUND;
        }
        auto response = rsp->add_response();
        if ((ret = ipsec_cfg_pol_get_cfg_handle(pol, response)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
            return ret;
        }
    }

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC CB delete request
//------------------------------------------------------------------------------
hal_ret_t
ipsec_rule_delete (ipsec::IpsecRuleDeleteRequest& req, ipsec::IpsecRuleDeleteResponseMsg *rsp)
{
    return HAL_RET_OK;
}


//-----------------------------------------------------------------------------
// Operational handling (hal handles, acl libs, etc)
//-----------------------------------------------------------------------------

static inline hal_ret_t
ipsec_cfg_pol_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t ret = HAL_RET_INVALID_ARG;
    ipsec_cfg_pol_t *policy;
    ipsec_cfg_pol_create_app_ctxt_t *app_ctx = NULL;
    hal_handle_t hal_handle;
    dllist_ctxt_t *lnode;
    dhl_entry_t *dhl_entry;

    if (!cfg_ctxt || !cfg_ctxt->app_ctxt) {
        HAL_TRACE_DEBUG("Failed here");
        goto end;
    }

    app_ctx = (ipsec_cfg_pol_create_app_ctxt_t *) cfg_ctxt->app_ctxt;

    if ((ret = acl::acl_commit(app_ctx->acl_ctx)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        goto end;
    }

    acl_deref(app_ctx->acl_ctx);

    lnode = cfg_ctxt->dhl.next;
    dhl_entry = dllist_entry(lnode, dhl_entry_t, dllist_ctxt);
    hal_handle = dhl_entry->handle;
    policy = (ipsec_cfg_pol_t *) dhl_entry->obj;
    HAL_TRACE_DEBUG("policy key ({}, {}), handle {}",
                    policy->key.vrf_id, policy->key.pol_id, hal_handle);
    ret = ipsec_cfg_pol_create_db_handle(policy);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("Failed to add IPSec policy ({}, {}) to db, err : {}",
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
ipsec_cfg_pol_rule_oper_handle (
    ipsec_cfg_pol_t *pol, ipsec_cfg_pol_create_app_ctxt_t *app_ctxt)
{
    hal_ret_t ret = HAL_RET_OK;
    ipsec_cfg_rule_t *rule;
    dllist_ctxt_t    *entry;
    uint32_t         prio=0;

    dllist_for_each(entry, &pol->rule_list) {
        rule = dllist_entry(entry, ipsec_cfg_rule_t, list_ctxt);
        rule->prio = prio++;
        if ((ret = ipsec_cfg_rule_create_oper_handle(
               rule, app_ctxt->acl_ctx)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
            return ret;
        }
    }
    return ret;
}

static inline void
ipsec_cfg_pol_oper_init (ipsec_cfg_pol_t *pol, hal_handle_t hal_hdl)
{
    pol->hal_hdl = hal_hdl;
}

hal_ret_t
ipsec_cfg_pol_create_oper_handle (ipsec_cfg_pol_t *pol)
{
    hal_ret_t ret;
    hal_handle_t hal_hdl;
    ipsec_cfg_pol_create_app_ctxt_t app_ctxt = { 0 };

    app_ctxt.acl_ctx = ipsec_cfg_pol_create_app_ctxt_init(pol);

    if ((ret = cfg_ctxt_op_create_handle(HAL_OBJ_ID_IPSEC_POLICY, pol, &app_ctxt,
                                         hal_cfg_op_null_cb,
                                         ipsec_cfg_pol_create_commit_cb,
                                         hal_cfg_op_null_cb,
                                         hal_cfg_op_null_cb,
                                         &hal_hdl)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    ipsec_cfg_pol_oper_init(pol, hal_hdl);

    if ((ret = ipsec_cfg_pol_rule_oper_handle(pol, &app_ctxt)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    return HAL_RET_OK;
}


//-----------------------------------------------------------------------------
// Rule action routines
//-----------------------------------------------------------------------------

static hal_ret_t
ipsec_cfg_rule_action_spec_extract (const ipsec::IpsecSAAction& spec,
                                    ipsec_cfg_rule_action_t *action)
{
    hal_ret_t ret = HAL_RET_OK;
    return ret;
}

//-----------------------------------------------------------------------------
// Rule routines
//-----------------------------------------------------------------------------

static inline ipsec_cfg_rule_t *
ipsec_cfg_rule_alloc (void)
{
    return ((ipsec_cfg_rule_t *)g_hal_state->ipsec_cfg_rule_slab()->alloc());
}

void
ipsec_cfg_rule_free (void *rule)
{
    hal::delay_delete_to_slab(HAL_SLAB_IPSEC_CFG_RULE, (ipsec_cfg_rule_t *)rule);
}

static inline void
ipsec_cfg_rule_init (ipsec_cfg_rule_t *rule)
{
    rule_match_init(&rule->match);
    dllist_reset(&rule->list_ctxt);
}

static inline void
ipsec_cfg_rule_uninit (ipsec_cfg_rule_t *rule)
{
    return;
}

static inline ipsec_cfg_rule_t *
ipsec_cfg_rule_alloc_init (void)
{
    ipsec_cfg_rule_t *rule;

    if ((rule = ipsec_cfg_rule_alloc()) ==  NULL)
        return NULL;

    ipsec_cfg_rule_init(rule);
    return rule;
}

static inline void
ipsec_cfg_rule_uninit_free (ipsec_cfg_rule_t *rule)
{
    if (rule) {
        ipsec_cfg_rule_uninit(rule);
        ipsec_cfg_rule_free(rule);
    }
}

static inline void
ipsec_cfg_rule_db_add (dllist_ctxt_t *head, ipsec_cfg_rule_t *rule)
{
    dllist_add_tail(head, &rule->list_ctxt);
}

static inline void
ipsec_cfg_rule_db_del (ipsec_cfg_rule_t *rule)
{
    dllist_del(&rule->list_ctxt);
}

static hal_ret_t
ipsec_cfg_rule_data_spec_extract (const ipsec::IpsecRuleMatchSpec& spec,
                                  ipsec_cfg_rule_t *rule)
{
    hal_ret_t ret = HAL_RET_OK;

    if ((ret = rule_match_spec_extract(
           spec.match(), &rule->match)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    if ((ret = ipsec_cfg_rule_action_spec_extract(
           spec.sa_action(), &rule->action)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    return ret;
}

static inline hal_ret_t
ipsec_cfg_rule_key_spec_extract (const ipsec::IpsecRuleMatchSpec& spec,
                                 ipsec_cfg_rule_key_t *key)
{
    key->rule_id = spec.rule_id();
    return HAL_RET_OK;
}

static inline hal_ret_t
ipsec_cfg_rule_spec_extract (const ipsec::IpsecRuleMatchSpec& spec, ipsec_cfg_rule_t *rule)
{
    hal_ret_t ret;

    if ((ret = ipsec_cfg_rule_key_spec_extract(
           spec, &rule->key)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    if ((ret = ipsec_cfg_rule_data_spec_extract(
           spec, rule)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

   return ret;
}

hal_ret_t
ipsec_cfg_rule_spec_handle (const ipsec::IpsecRuleMatchSpec& spec, dllist_ctxt_t *head)
{
    hal_ret_t ret;
    ipsec_cfg_rule_t *rule;

    if ((rule = ipsec_cfg_rule_alloc_init()) == NULL) {
        HAL_TRACE_DEBUG("Failed here");
        return HAL_RET_OOM;
    }

    if ((ret = ipsec_cfg_rule_spec_extract(spec, rule)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    ipsec_cfg_rule_db_add(head, rule);
    return ret;
}

hal_ret_t
ipsec_cfg_rule_spec_build (ipsec_cfg_rule_t *rule, ipsec::IpsecRuleMatchSpec *spec)
{
    hal_ret_t   ret;

    spec->set_rule_id(rule->key.rule_id);

    //auto action = spec->mutable_sa_action();
    //action->set_src_nat_action(rule->action.src_nat_action);
    //action->set_dst_nat_action(rule->action.dst_nat_action);
    // TODO handle SA action

    if ((ret = rule_match_spec_build(
           &rule->match, spec->mutable_match())) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed here");
        return ret;
    }

    return ret;
}

//-----------------------------------------------------------------------------
// Operational handling (hal handles, acl libs, etc)
//-----------------------------------------------------------------------------

acl_config_t ipsec_ip_acl_config_glbl;

#if 0
const char *
ipsec_acl_ctx_name (vrf_id_t vrf_id)
{
    thread_local static char name[ACL_NAMESIZE];

    std::snprintf(name, sizeof(name), "ipsec-ipv4-rules:%lu", vrf_id);
    HAL_TRACE_DEBUG("Acl Name: {}", name);
    return name;
}
#endif

const acl::acl_ctx_t *
ipsec_cfg_pol_create_app_ctxt_init (ipsec_cfg_pol_t *pol)
{
    return (rule_lib_init(ipsec_acl_ctx_name(
               pol->key.vrf_id), &ipsec_ip_acl_config_glbl));
}

hal_ret_t
ipsec_cfg_rule_create_oper_handle (ipsec_cfg_rule_t *rule, const acl_ctx_t *acl_ctx)
{
    rule_data_t     *rule_data;
    rule_data = rule_data_alloc_init();
    rule_data->userdata = rule;
    rule_data->data_free = ipsec_cfg_rule_free;
    return rule_match_rule_add(&acl_ctx,
                               &rule->match,
                               rule->prio,
                               rule_data);
}

}    // namespace hal
