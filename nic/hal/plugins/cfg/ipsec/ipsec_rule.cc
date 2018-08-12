#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/include/hal_lock.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/include/fte.hpp"
#include "nic/include/hal_state.hpp"
#include "nic/hal/plugins/cfg/ipsec/ipsec.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/plugins/cfg/nw/vrf_api.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include <google/protobuf/util/json_util.h>
#include "nic/include/hal_cfg.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/fte/acl/acl.hpp"
#include "nic/hal/plugins/cfg/nw/vrf.hpp"
//#include "nic/hal/pd/iris/nw/vrf_pd.hpp"
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
ipsec_cfg_pol_key_spec_extract (const kh::IpsecRuleKeyHandle& spec, ipsec_cfg_pol_key_t *key)
{
    vrf_t *vrf;

    key->pol_id = spec.rule_key().ipsec_rule_id();

    if (spec.rule_key().
        vrf_key_or_handle().key_or_handle_case() == kh::VrfKeyHandle::kVrfId) {
        key->vrf_id = spec.rule_key().vrf_key_or_handle().vrf_id();

        if ((vrf = vrf_lookup_by_id(key->vrf_id)) == NULL) {
        HAL_TRACE_DEBUG("Failed here");
            return  HAL_RET_VRF_NOT_FOUND;
        }
    } else {
        if ((vrf = vrf_lookup_by_handle(spec.rule_key().
                vrf_key_or_handle().vrf_handle())) == NULL) {
        HAL_TRACE_DEBUG("Failed here 2");
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

    if ((ret = ipsec_cfg_pol_key_spec_extract(spec.key_or_handle(), &pol->key)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to extract ipsec policy key spec");
        return ret;
    }

    if ((ret = ipsec_cfg_pol_data_spec_extract(spec, pol)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to extract ipsec policy data spec");
        return ret;
    }

    return ret;
}


ipsec_cfg_pol_t *
ipsec_cfg_pol_key_or_handle_lookup (const kh::IpsecRuleKeyHandle& kh)
{
    if (kh.rule_handle() != HAL_HANDLE_INVALID) {
        return ipsec_cfg_pol_hal_hdl_db_lookup(kh.rule_handle());
    } else {
        ipsec_cfg_pol_key_t key = {0};
        ipsec_cfg_pol_key_spec_extract(kh, &key);
        return ipsec_cfg_pol_db_lookup(&key);
    }
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
ipsec_cfg_pol_key_spec_validate (const kh::IpsecRuleKeyHandle& spec, bool create)
{
    if (create) {
        if (spec.rule_handle() != HAL_HANDLE_INVALID)
            return HAL_RET_INVALID_ARG;
    }

    if (spec.rule_key().vrf_key_or_handle().key_or_handle_case() ==
        kh::VrfKeyHandle::kVrfId) {
        if (vrf_lookup_by_id(spec.rule_key().vrf_key_or_handle().
                             vrf_id()) == NULL)
            return  HAL_RET_VRF_NOT_FOUND;
    } else {
        if (vrf_lookup_by_handle(spec.rule_key().vrf_key_or_handle().
                                 vrf_handle()) == NULL)
            return HAL_RET_VRF_NOT_FOUND;
    }

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

    if ((ret = ipsec_cfg_pol_key_spec_validate(
         spec.key_or_handle(), create)) != HAL_RET_OK) {
        HAL_TRACE_DEBUG("Failed to validate ipsec rule key spec");
        return ret;
    }

    return HAL_RET_OK;
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
    ipsec_cfg_pol_create_rsp_build(rsp, ret, pol ? pol->hal_hdl : HAL_HANDLE_INVALID);
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

hal_ret_t
ipsec_cfg_pol_rule_spec_build (ipsec_cfg_pol_t *pol,
                               ipsec::IpsecRuleSpec *spec)
{
    hal_ret_t        ret = HAL_RET_OK;
    dllist_ctxt_t    *entry;
    ipsec_cfg_rule_t *rule;

    dllist_for_each(entry, &pol->rule_list) {
        rule = dllist_entry(entry, ipsec_cfg_rule_t, list_ctxt);
        auto rule_spec = spec->add_rules();
        if ((ret = ipsec_cfg_rule_spec_build(
               rule, rule_spec, spec)) != HAL_RET_OK) {
            HAL_TRACE_DEBUG("Failed here");
            return ret;
        }
    }

    return ret;
}

static inline hal_ret_t
ipsec_cfg_pol_data_spec_build (ipsec_cfg_pol_t *pol, ipsec::IpsecRuleSpec *spec)
{
    hal_ret_t ret;

    if ((ret = ipsec_cfg_pol_rule_spec_build(pol, spec)) != HAL_RET_OK)
        return ret;

    return ret;
}

hal_ret_t
ipsec_cfg_pol_key_spec_build (ipsec_cfg_pol_t *pol, kh::IpsecRuleKeyHandle *spec)
{
    spec->mutable_rule_key()->mutable_vrf_key_or_handle()->set_vrf_id(
        pol->key.vrf_id);
    spec->mutable_rule_key()->set_ipsec_rule_id(pol->key.pol_id);
    spec->set_rule_handle(pol->hal_hdl);
    return HAL_RET_OK;
}

static hal_ret_t
ipsec_cfg_pol_spec_build (ipsec_cfg_pol_t *pol,
                          ipsec::IpsecRuleSpec *spec)
{
    hal_ret_t           ret;

    if ((ret = ipsec_cfg_pol_key_spec_build(
            pol, spec->mutable_key_or_handle())) != HAL_RET_OK)
        return ret;

    if ((ret = ipsec_cfg_pol_data_spec_build(pol, spec)) != HAL_RET_OK) {
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
    ipsec_cfg_pol_spec_build(pol, response->mutable_spec());

    // return false here, so that we don't terminate the walk
    return false;
}

//------------------------------------------------------------------------------
// process a IPSEC CB get request
//------------------------------------------------------------------------------
hal_ret_t
ipsec_rule_get (IpsecRuleGetRequest& req, IpsecRuleGetResponseMsg *rsp)
{
    ipsec_cfg_pol_t  *pol;

    if (!req.has_key_or_handle()) {
        g_hal_state->ipsec_policy_ht()->walk(ipsec_policy_get_ht_cb, rsp);
        HAL_API_STATS_INC(HAL_API_IPSEC_RULE_GET_SUCCESS);
	return HAL_RET_OK;
    }

    auto kh = req.key_or_handle();
    auto response = rsp->add_response();
    if ((pol = ipsec_cfg_pol_key_or_handle_lookup(kh)) == NULL) { 
        response->set_api_status(types::API_STATUS_NOT_FOUND);
        HAL_API_STATS_INC(HAL_API_IPSEC_RULE_GET_FAIL);
        return HAL_RET_IPSEC_RULE_NOT_FOUND;
    }
    ipsec_cfg_pol_spec_build(pol, response->mutable_spec());
    HAL_API_STATS_INC(HAL_API_IPSEC_RULE_GET_SUCCESS);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// DELETE configuration handling
//-----------------------------------------------------------------------------

hal_ret_t
ipsec_cfg_pol_delete_cfg_handle (ipsec_cfg_pol_t *pol)
{
    ipsec_cfg_pol_cleanup(pol);
    return HAL_RET_OK;
}

//-----------------------------------------------------------------------------
// DELETE operational handling
//-----------------------------------------------------------------------------

hal_ret_t
ipsec_cfg_pol_delete_oper_handle (ipsec_cfg_pol_t *pol)
{
    hal_ret_t ret;

    if ((ret = ipsec_cfg_pol_delete_db_handle(&pol->key)) != HAL_RET_OK)
        return ret;

    if ((ret = cfg_ctxt_op_delete_handle(
            HAL_OBJ_ID_IPSEC_POLICY, pol, NULL, hal_cfg_op_null_cb,
            hal_cfg_op_null_cb, hal_cfg_op_null_cb,
            hal_cfg_op_null_cb, pol->hal_hdl)) != HAL_RET_OK)
        return ret;

    if ((ret = ipsec_cfg_pol_acl_cleanup(pol)) != HAL_RET_OK)
        return ret;

    return HAL_RET_OK;
}

//------------------------------------------------------------------------------
// process a IPSEC SPD Rule delete request
//------------------------------------------------------------------------------
hal_ret_t
ipsec_rule_delete (ipsec::IpsecRuleDeleteRequest& req, ipsec::IpsecRuleDeleteResponse *rsp)
{
    hal_ret_t       ret = HAL_RET_OK;
    ipsec_cfg_pol_t *policy;

    if ((policy = ipsec_cfg_pol_key_or_handle_lookup(req.key_or_handle())) == NULL) {
        ret = HAL_RET_IPSEC_RULE_NOT_FOUND;
        goto end;
    }

    if ((ret = ipsec_cfg_pol_delete_oper_handle(policy)) != HAL_RET_OK)
        goto end;

    if ((ret = ipsec_cfg_pol_delete_cfg_handle(policy)) != HAL_RET_OK)
        goto end;

end:
    ipsec_cfg_pol_delete_rsp_build(rsp, ret);
    return ret;
}


//-----------------------------------------------------------------------------
// Operational handling (hal handles, acl libs, etc)
//-----------------------------------------------------------------------------

static inline hal_ret_t
ipsec_cfg_pol_create_commit_cb (cfg_op_ctxt_t *cfg_ctxt)
{
    hal_ret_t ret = HAL_RET_INVALID_ARG;
    ipsec_cfg_pol_create_app_ctxt_t *app_ctx = NULL;

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

end:

    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("ipsec create commit failed");
        //todo: free resources
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

    // build acl before operating on the callbacks
    if ((ret = ipsec_cfg_pol_acl_build(pol, &app_ctxt.acl_ctx)) != HAL_RET_OK)
        return ret;

    if ((ret = cfg_ctxt_op_create_handle(
            HAL_OBJ_ID_IPSEC_POLICY, pol, &app_ctxt, hal_cfg_op_null_cb,
            ipsec_cfg_pol_create_commit_cb, hal_cfg_op_null_cb,
            hal_cfg_op_null_cb, &hal_hdl)) != HAL_RET_OK)
        return ret;

    // save the hal handle and add policy to databases
    pol->hal_hdl = hal_hdl;
    if ((ret = ipsec_cfg_pol_create_db_handle(pol)) != HAL_RET_OK)
        return ret;

    return HAL_RET_OK;
}


//-----------------------------------------------------------------------------
// Rule action routines
//-----------------------------------------------------------------------------

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
ipsec_cfg_rule_spec_build (ipsec_cfg_rule_t *rule, ipsec::IpsecRuleMatchSpec *spec, ipsec::IpsecRuleSpec *rule_spec)
{
    hal_ret_t   ret;

    spec->set_rule_id(rule->key.rule_id);

    spec->mutable_sa_action()->set_sa_action_type(rule->action.sa_action);
    if (rule->action.sa_action == ipsec::IPSEC_SA_ACTION_TYPE_ENCRYPT) {
        spec->mutable_sa_action()->mutable_enc_handle()->set_cb_id(rule->action.sa_action_enc_handle);
    } else if (rule->action.sa_action == ipsec::IPSEC_SA_ACTION_TYPE_DECRYPT) {
        spec->mutable_sa_action()->mutable_dec_handle()->set_cb_id(rule->action.sa_action_dec_handle);
    }

    HAL_TRACE_DEBUG("action type {} enc_handle {} dec_handle {} vrf-id {}", 
                    rule->action.sa_action, rule->action.sa_action_enc_handle, 
                    rule->action.sa_action_dec_handle, rule->action.vrf);

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

const acl::acl_ctx_t *
ipsec_cfg_pol_create_app_ctxt_init (ipsec_cfg_pol_t *pol)
{
    char acl_name[ACL_NAMESIZE];
    ipsec_acl_ctx_name(acl_name, pol->key.vrf_id);
    return (rule_lib_init(acl_name, &ipsec_ip_acl_config_glbl));
}

hal_ret_t
ipsec_cfg_rule_create_oper_handle (ipsec_cfg_rule_t *rule, const acl_ctx_t *acl_ctx)
{
    return rule_match_rule_add(&acl_ctx,
                               &rule->match,
                               rule->prio,
                               (void *) &rule->ref_count);
}

}    // namespace hal
