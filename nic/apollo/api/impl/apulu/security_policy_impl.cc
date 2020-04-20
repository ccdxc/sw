//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of security policy
///
//----------------------------------------------------------------------------

#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/policy.hpp"
#include "nic/apollo/api/impl/rfc/rfc.hpp"
#include "nic/apollo/api/impl/apulu/security_policy_impl.hpp"
#include "nic/apollo/api/impl/apulu/pds_impl_state.hpp"

namespace api {
namespace impl {

/// \defgroup PDS_SECURITY_POLICY_IMPL - security policy datapath implementation
/// \ingroup PDS_SECURITY_POLICY
/// @{

security_policy_impl *
security_policy_impl::factory(pds_policy_spec_t *spec) {
    security_policy_impl    *impl;

    if (spec->rule_info->num_rules >
            security_policy_impl_db()->max_rules(spec->rule_info->af)) {
        PDS_TRACE_ERR("No. of rules %u in the policy %s exceeded the max "
                      "supported scale %u", spec->rule_info->num_rules,
                      spec->key.str(),
                      security_policy_impl_db()->max_rules(spec->rule_info->af));
        return NULL;
    }
    impl = security_policy_impl_db()->alloc();
    new (impl) security_policy_impl();
    return impl;
}

void
security_policy_impl::destroy(security_policy_impl *impl) {
    impl->~security_policy_impl();
    security_policy_impl_db()->free(impl);
}

impl_base *
security_policy_impl::clone(void) {
    security_policy_impl *cloned_impl;

    cloned_impl = security_policy_impl_db()->alloc();
    new (cloned_impl) security_policy_impl();
    // deep copy is not needed as we don't store pointers
    *cloned_impl = *this;
    return cloned_impl;
}

sdk_ret_t
security_policy_impl::free(security_policy_impl *impl) {
    destroy(impl);
    return SDK_RET_OK;
}

sdk_ret_t
security_policy_impl::reserve_resources(api_base *api_obj, api_base *orig_obj,
                                        api_obj_ctxt_t *obj_ctxt) {
    pds_policy_spec_t *spec;
    policy *security_policy;
    uint32_t policy_block_id;

    security_policy = (policy *)api_obj;
    // record the fact that resource reservation was attempted
    // NOTE: even if we partially acquire resources and fail eventually,
    //       this will ensure that proper release of resources will happen
    api_obj->set_rsvd_rsc();
    // allocate available block for this security policy
    if (security_policy_impl_db()->security_policy_idxr(security_policy->af())->alloc(
            &policy_block_id) != SDK_RET_OK) {
        return sdk::SDK_RET_NO_RESOURCE;
    }
    security_policy_root_addr_ =
        security_policy_impl_db()->security_policy_region_addr(security_policy->af()) +
            (security_policy_impl_db()->security_policy_table_size(security_policy->af()) *
                 policy_block_id);
    return SDK_RET_OK;
}

sdk_ret_t
security_policy_impl::release_resources(api_base *api_obj) {
    uint32_t    policy_block_id;
    policy      *security_policy;

    security_policy = (policy *)api_obj;
    if (security_policy_root_addr_ != 0xFFFFFFFFFFFFFFFFUL) {
        policy_block_id =
            (security_policy_root_addr_ -
                 security_policy_impl_db()->security_policy_region_addr(security_policy->af()))/
                 security_policy_impl_db()->security_policy_table_size(security_policy->af());
        security_policy_impl_db()->security_policy_idxr(security_policy->af())->free(policy_block_id);
    }
    return SDK_RET_OK;
}

sdk_ret_t
security_policy_impl::nuke_resources(api_base *api_obj) {
    return this->release_resources(api_obj);
}

sdk_ret_t
security_policy_impl::program_security_policy_(pds_policy_spec_t *spec) {
    sdk_ret_t ret;
    rfc::policy_t policy;

    memset(&policy, 0, sizeof(policy));
    policy.af = spec->rule_info->af;
    policy.max_rules =
        (policy.af ==IP_AF_IPV4) ? PDS_MAX_RULES_PER_IPV4_SECURITY_POLICY:
                                   PDS_MAX_RULES_PER_IPV6_SECURITY_POLICY;
    policy.default_action = spec->rule_info->default_action;
    policy.num_rules = spec->rule_info->num_rules;
    policy.rules = &spec->rule_info->rules[0];
    PDS_TRACE_DEBUG("Processing security policy %s", spec->key.str());
    ret = rfc_policy_create(&policy, security_policy_root_addr_,
              security_policy_impl_db()->security_policy_table_size(spec->rule_info->af));
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to build RFC policy table, err %u", ret);
    }
    return ret;
}

sdk_ret_t
security_policy_impl::update_policy_spec_(pds_policy_spec_t *spec,
                                          api_obj_ctxt_t *obj_ctxt) {
    uint32_t i;
    bool found;
    api_obj_ctxt_t *octxt;
    pds_policy_rule_key_t key;

    for (auto it = obj_ctxt->clist.begin(); it != obj_ctxt->clist.end(); it++) {
        octxt = *it;
        if (octxt->api_op == API_OP_CREATE) {
            // add the route to the end of the table
            spec->rule_info->rules[spec->rule_info->num_rules].key =
                octxt->api_params->policy_rule_spec.key.rule_id;
            spec->rule_info->rules[spec->rule_info->num_rules].attrs =
                octxt->api_params->policy_rule_spec.attrs;
            spec->rule_info->num_rules++;
        } else {
            // either DEL or UPD operation
            if (octxt->api_op == API_OP_DELETE) {
                key = octxt->api_params->policy_rule_key;
            } else {
                // update case
                key = octxt->api_params->policy_rule_spec.key;
            }
            // search and find the object to delete or modify
            found = false;
            for (i = 0; i < spec->rule_info->num_rules; i++) {
                if (key.rule_id == spec->rule_info->rules[i].key) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                PDS_TRACE_ERR("policy rule %s not found in policy %s to "
                              "perform api op %u", key.rule_id.str(),
                              spec->key.str(), octxt->api_op);
                return SDK_RET_INVALID_ARG;
            }
            if (octxt->api_op == API_OP_DELETE) {
                // move the last rule to this spot
                spec->rule_info->rules[i] =
                    spec->rule_info->rules[spec->rule_info->num_rules - 1];
                spec->rule_info->num_rules--;
            } else {
                // update case
                spec->rule_info->rules[i].attrs =
                    octxt->api_params->policy_rule_spec.attrs;
            }
        }
    }
    return SDK_RET_OK;
}

sdk_ret_t
security_policy_impl::compute_updated_spec_(policy *new_policy,
                                            policy *old_policy,
                                            api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;
    uint32_t num_rules;
    pds_policy_spec_t *spec;
    rule_info_t *new_rule_info;

    if (obj_ctxt->clist.size() == 0) {
        SDK_ASSERT((obj_ctxt->upd_bmap & (PDS_POLICY_UPD_RULE_ADD |
                                          PDS_POLICY_UPD_RULE_DEL |
                                          PDS_POLICY_UPD_RULE_UPD)) == 0);
        PDS_TRACE_DEBUG("Processing policy %s create/update with no individual "
                        "rule updates in this batch",
                        new_policy->key2str().c_str());
        // in this case, spec can be used as-is from the object context
        return SDK_RET_OK;
    }

    // we have few cases to handle here:
    // 1. policy object itself is being updated (some other attributes
    //    modifications and/or with new set of rules combined with
    //    individual rule add/del/update operations - all in this batch
    // 2. policy object modification is solely because of individual rule
    //    add/del/updates in this batch
    // in both cases, we need to form new spec
    spec = &obj_ctxt->api_params->policy_spec;
    if ((obj_ctxt->api_op == API_OP_CREATE) ||
        (obj_ctxt->upd_bmap & ~(PDS_POLICY_UPD_RULE_ADD |
                                PDS_POLICY_UPD_RULE_DEL |
                                PDS_POLICY_UPD_RULE_UPD))) {
        // case 1 : both container and contained objects are being modified
        //          (ADD/DEL/UPD), in this we can fully ignore the set of rules
        //          that are persisted in kvstore
        // number of new rules in the worst case will be total of what we have
        // currently plus size of clist (assuming all contained objs are being
        // added)
        // TODO:
        // keep track of this counter in obj_ctxt itself in API engine so we can
        // catch errors where total capacity exceeds max. supported without any
        // extra processing
        num_rules = new_policy->num_rules() + obj_ctxt->clist.size();
        new_rule_info =
            (rule_info_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_POLICY,
                                      POLICY_RULE_INFO_SIZE(num_rules));
        if (!new_rule_info) {
            PDS_TRACE_ERR("Failed to allocate memory for %u rules for "
                          "policy %s update processing", num_rules,
                          spec->key.str());
            ret = SDK_RET_OOM;
            goto end;
        }
        memcpy(new_rule_info, spec->rule_info,
               POLICY_RULE_INFO_SIZE(new_policy->num_rules()));
        if (spec->rule_info) {
            SDK_FREE(PDS_MEM_ALLOC_ID_POLICY, spec->rule_info);
            spec->rule_info = NULL;
        }
        spec->rule_info = new_rule_info;
    } else {
        // case 2 : only contained objects are being modified (ADD/DEL/UPD),
        //          form new spec that consists of current set of rules and
        //          individual rule updates
        // number of new rules in the worst case will be total of what we have
        // currently plus size of clist (assuming all contained objs are being
        // added)
        // TODO:
        // keep track of this counter in obj_ctxt itself in API engine so we can
        // catch errors where total capacity exceeds max. supported without any
        // extra processing
        spec->key = new_policy->key();
        num_rules = old_policy->num_rules() + obj_ctxt->clist.size();
        spec->rule_info =
            (rule_info_t *)SDK_MALLOC(PDS_MEM_ALLOC_ID_POLICY,
                                      POLICY_RULE_INFO_SIZE(num_rules));
        if (!spec->rule_info) {
            PDS_TRACE_ERR("Failed to allocate memory for %u rules for "
                          "policy %s update processing", num_rules,
                          spec->key.str());
            ret = SDK_RET_OOM;
            goto end;
        }
        spec->rule_info->num_rules = old_policy->num_rules();
        ret = policy_db()->retrieve_rules(&spec->key, spec->rule_info);
        if (ret != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to retrieve rules from kvstore for "
                          "policy %s, err %u", spec->key.str(), ret);
            goto end;
        }
    }

    // compute the updated spec now
    ret = update_policy_spec_(spec, obj_ctxt);
    if (ret != SDK_RET_OK) {
        goto end;
    }
    PDS_TRACE_DEBUG("Policy %s rule count changed from %u to %u",
                    spec->key.str(), old_policy->num_rules(),
                    spec->rule_info->num_rules);
    return SDK_RET_OK;

end:

    if (spec->rule_info) {
        SDK_FREE(PDS_MEM_ALLOC_ID_POLICY, spec->rule_info);
        spec->rule_info = NULL;
    }
    return ret;
}

sdk_ret_t
security_policy_impl::program_hw(api_base *api_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;

    // update the spec, if needed
    ret = compute_updated_spec_((policy *)api_obj, (policy *)api_obj,
                                obj_ctxt);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // and program it in the pipeline
    ret = program_security_policy_(&obj_ctxt->api_params->policy_spec);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program policy %s create, err %u",
                      obj_ctxt->api_params->policy_spec.key.str(), ret);
    }
    return ret;
}

sdk_ret_t
security_policy_impl::update_hw(api_base *orig_obj, api_base *curr_obj,
                                api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;

    // update the spec, if needed
    ret = compute_updated_spec_((policy *)curr_obj, (policy *)orig_obj,
                                obj_ctxt);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // and program it in the pipeline
    ret = program_security_policy_(&obj_ctxt->api_params->policy_spec);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to program policy %s update, err %u",
                      obj_ctxt->api_params->policy_spec.key.str(), ret);
    }
    return ret;
}

sdk_ret_t
security_policy_impl::activate_hw(api_base *api_obj, api_base *orig_obj,
                                  pds_epoch_t epoch, api_op_t api_op,
                                  api_obj_ctxt_t *obj_ctxt)
{
    sdk_ret_t ret;
    pds_policy_spec_t *spec;

    switch (api_op) {
    case API_OP_CREATE:
        spec = &obj_ctxt->api_params->policy_spec;
        ret = policy_db()->persist((policy *)api_obj, spec);
        break;

    case API_OP_UPDATE:
        spec = &obj_ctxt->api_params->policy_spec;
        if ((ret = policy_db()->perish(spec->key)) ==
                SDK_RET_OK) {
            ret = policy_db()->persist((policy *)api_obj, spec);
        }
        break;

    case API_OP_DELETE:
        ret = policy_db()->perish(obj_ctxt->api_params->key);
        break;

    default:
        ret = SDK_RET_INVALID_OP;
        break;
    }
    return ret;
}

void
security_policy_impl::fill_status_(pds_policy_status_t *status) {
    status->policy_base_addr = security_policy_root_addr_;
}

sdk_ret_t
security_policy_impl::read_hw(api_base *api_obj, obj_key_t *key,
                              obj_info_t *info) {
    pds_policy_info_t *policy_info = (pds_policy_info_t *)info;

    fill_status_(&policy_info->status);

    return SDK_RET_OK;
}

/// \@}

}    // namespace impl
}    // namespace api
