//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file implements security policy CRUD APIs
///
//----------------------------------------------------------------------------

#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_msg.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/obj_api.hpp"
#include "nic/apollo/api/policy.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/api/policy_state.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"

static inline sdk_ret_t
pds_policy_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                       pds_obj_key_t *key, pds_policy_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_POLICY, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->key = *key;
        } else {
            api_ctxt->api_params->policy_spec = *spec;
            if (spec->rule_info == NULL) {
                PDS_TRACE_ERR("Rejected policy %s, api op %u with no rules",
                              spec->key.str(), op);
                return SDK_RET_INVALID_ARG;
            }
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

static inline policy *
pds_policy_entry_find (pds_obj_key_t *key)
{
    return policy_db()->find_policy(key);
}

//----------------------------------------------------------------------------
// Policy API entry point implementation
//----------------------------------------------------------------------------

sdk_ret_t
pds_policy_create (_In_ pds_policy_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_policy_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_policy_read (_In_ pds_obj_key_t *key, _Out_ pds_policy_info_t *info)
{
    sdk_ret_t ret;
    policy *entry;

    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }

    if ((entry = pds_policy_entry_find(key)) == NULL) {
        return SDK_RET_ENTRY_NOT_FOUND;
    }

    if (info->spec.rule_info == NULL) {
        return entry->read(info);
    }

    if (!info->spec.rule_info->num_rules) {
        info->spec.rule_info->num_rules = entry->num_rules();
    } else {
        uint32_t num_rules_to_read = info->spec.rule_info->num_rules;
        if (num_rules_to_read < entry->num_rules()) {
            ret = entry->read(info);
            if (ret != SDK_RET_OK) {
                return ret;
            }
            // buffer is smaller, read all rules and copy over the
            // requested number allocate memory for reading all the rules
            rule_info_t *rule_info =
                (rule_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                          POLICY_RULE_INFO_SIZE(entry->num_rules()));
            rule_info->num_rules = entry->num_rules();
            // retrieve all rules
            ret = policy_db()->retrieve_rules(key, rule_info);
            if (ret != SDK_RET_OK) {
                SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, rule_info);
                return ret;
            }
            // copy over requested number of rules
            memcpy(info->spec.rule_info, rule_info,
                   POLICY_RULE_INFO_SIZE(num_rules_to_read));
            info->spec.rule_info->num_rules = num_rules_to_read;
            // free allocated memory
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, rule_info);
        } else {
            ret = entry->read(info);
            if (ret != SDK_RET_OK) {
                return ret;
            }
            // read rules from lmdb
            return policy_db()->retrieve_rules(key,
                                               info->spec.rule_info);
        }
    }

    return ret;
}

typedef struct pds_policy_read_args_s {
    policy_read_cb_t cb;
    void *ctxt;
} pds_policy_read_args_t;

bool
pds_policy_info_from_entry (void *entry, void *ctxt)
{
    policy *pol = (policy *)entry;
    pds_policy_read_args_t *args = (pds_policy_read_args_t *)ctxt;
    pds_policy_info_t info;
    uint32_t num_rules = 0;

    memset(&info, 0, sizeof(pds_policy_info_t));

    // read number of rules and allocate memory for rules
    num_rules = pol->num_rules();
    info.spec.rule_info =
        (rule_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                  POLICY_RULE_INFO_SIZE(num_rules));
    info.spec.rule_info->num_rules = num_rules;

    // entry read
    pol->read(&info);

    // retrieve rules from kvstore
    return policy_db()->retrieve_rules(&info.spec.key, info.spec.rule_info);

    // call cb on info
    args->cb(&info, args->ctxt);

    // free memory
    SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, info.spec.rule_info);
    info.spec.rule_info = NULL;

    return false;
}

sdk_ret_t
pds_policy_read_all (policy_read_cb_t policy_read_cb, void *ctxt)
{
    pds_policy_read_args_t args = {0};

    args.ctxt = ctxt;
    args.cb = policy_read_cb;

    return policy_db()->walk(pds_policy_info_from_entry, &args);
}

sdk_ret_t
pds_policy_update (_In_ pds_policy_spec_t *spec, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_policy_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_policy_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_policy_api_handle(bctxt, API_OP_DELETE, key, NULL);
}

//----------------------------------------------------------------------------
// Policy Rule API implementation entry point
//----------------------------------------------------------------------------

static inline sdk_ret_t
pds_policy_rule_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                            pds_obj_key_t *key, pds_policy_rule_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_POLICY_RULE, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->key = *key;
        } else {
            api_ctxt->api_params->policy_rule_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

sdk_ret_t
pds_policy_rule_create (_In_ pds_policy_rule_spec_t *spec,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_policy_rule_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_policy_rule_read (_In_ pds_obj_key_t *key, _Out_ pds_policy_info_t *info)
{
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
pds_policy_rule_update (_In_ pds_policy_rule_spec_t *spec,
                        _In_ pds_batch_ctxt_t bctxt)
{
    return pds_policy_rule_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_policy_rule_delete (_In_ pds_obj_key_t *key, _In_ pds_batch_ctxt_t bctxt)
{
    return pds_policy_rule_api_handle(bctxt, API_OP_DELETE, key, NULL);
}

//----------------------------------------------------------------------------
// Security Profile API entry point implementation
//----------------------------------------------------------------------------

static inline sdk_ret_t
pds_security_profile_api_handle (pds_batch_ctxt_t bctxt, api_op_t op,
                                 pds_obj_key_t *key,
                                 pds_security_profile_spec_t *spec)
{
    sdk_ret_t rv;
    api_ctxt_t *api_ctxt;

    if ((rv = pds_obj_api_validate(op, key, spec)) != SDK_RET_OK) {
        return rv;
    }

    api_ctxt = api::api_ctxt_alloc(OBJ_ID_SECURITY_PROFILE, op);
    if (likely(api_ctxt != NULL)) {
        if (op == API_OP_DELETE) {
            api_ctxt->api_params->key = *key;
        } else {
            api_ctxt->api_params->security_profile_spec = *spec;
        }
        return process_api(bctxt, api_ctxt);
    }
    return SDK_RET_OOM;
}

sdk_ret_t
pds_security_profile_create (_In_ pds_security_profile_spec_t *spec,
                             _In_ pds_batch_ctxt_t bctxt)
{
    return pds_security_profile_api_handle(bctxt, API_OP_CREATE, NULL, spec);
}

sdk_ret_t
pds_security_profile_read (_In_ pds_obj_key_t *key,
                           _Out_ pds_security_profile_info_t *info)
{
    if (key == NULL || info == NULL) {
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
pds_security_profile_update (_In_ pds_security_profile_spec_t *spec,
                             _In_ pds_batch_ctxt_t bctxt)
{
    return pds_security_profile_api_handle(bctxt, API_OP_UPDATE, NULL, spec);
}

sdk_ret_t
pds_security_profile_delete (_In_ pds_obj_key_t *key,
                             _In_ pds_batch_ctxt_t bctxt)
{
    return pds_security_profile_api_handle(bctxt, API_OP_DELETE, key, NULL);
}
