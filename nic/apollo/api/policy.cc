//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// policy handling
///
//----------------------------------------------------------------------------


#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/subnet.hpp"
#include "nic/apollo/api/vnic.hpp"
#include "nic/apollo/api/policy.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

/// \defgroup PDS_POLICY - security policy functionality
/// \ingroup PDS_POLICY
/// @{

typedef struct policy_upd_ctxt_s {
    policy *policy_obj;
    api_obj_ctxt_t *obj_ctxt;
    uint64_t upd_bmap;
} __PACK__ policy_upd_ctxt_t;

policy::policy() {
    af_ = IP_AF_NIL;
    num_rules_ = 0;
    ht_ctxt_.reset();
    impl_ = NULL;
}

policy *
policy::factory(pds_policy_spec_t *spec) {
    policy    *new_policy;

    /**< create security policy instance with defaults, if any */
    new_policy = policy_db()->alloc();
    if (new_policy) {
        new (new_policy) policy();
        new_policy->impl_ =
            impl_base::factory(impl::IMPL_OBJ_ID_SECURITY_POLICY,
                               spec);
        if (new_policy->impl_ == NULL) {
            policy::destroy(new_policy);
            return NULL;
        }
    }
    return new_policy;
}

policy::~policy() {
}

void
policy::destroy(policy *policy) {
    policy->nuke_resources_();
    if (policy->impl_) {
        impl_base::destroy(impl::IMPL_OBJ_ID_SECURITY_POLICY, policy->impl_);
    }
    policy->~policy();
    policy_db()->free(policy);
}

api_base *
policy::clone(api_ctxt_t *api_ctxt) {
    policy *cloned_policy;

    cloned_policy = policy_db()->alloc();
    if (cloned_policy) {
        new (cloned_policy) policy();
        if (cloned_policy->init_config(api_ctxt) != SDK_RET_OK) {
            goto error;
        }
        cloned_policy->impl_ = impl_->clone();
        if (unlikely(cloned_policy->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone policy %s impl", key2str().c_str());
            goto error;
        }
    }
    return cloned_policy;

error:

    cloned_policy->~policy();
    policy_db()->free(cloned_policy);
    return NULL;
}

api_base *
policy::clone(void) {
    policy *cloned_policy;

    cloned_policy = policy_db()->alloc();
    if (cloned_policy) {
        new (cloned_policy) policy();
        if (cloned_policy->init_config_(this) != SDK_RET_OK) {
            goto error;
        }
        cloned_policy->impl_ = impl_->clone();
        if (unlikely(cloned_policy->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone policy %s impl",
                          key2str().c_str());
            goto error;
        }
    }
    return cloned_policy;

error:

    cloned_policy->~policy();
    policy_db()->free(cloned_policy);
    return NULL;
}

sdk_ret_t
policy::free(policy *policy) {
    if (policy->impl_) {
        impl_base::free(impl::IMPL_OBJ_ID_SECURITY_POLICY, policy->impl_);
    }
    policy->~policy();
    policy_db()->free(policy);
    return SDK_RET_OK;
}

sdk_ret_t
policy::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, orig_obj, obj_ctxt);
}

sdk_ret_t
policy::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
policy::nuke_resources_(void) {
    return impl_->nuke_resources(this);
}

sdk_ret_t
policy::init_config(api_ctxt_t *api_ctxt) {
    pds_policy_spec_t    *spec;

    spec = &api_ctxt->api_params->policy_spec;
    this->af_ = spec->rule_info->af;
    this->num_rules_ = spec->rule_info->num_rules;
    memcpy(&this->key_, &spec->key, sizeof(pds_obj_key_t));
    return SDK_RET_OK;
}

sdk_ret_t
policy::init_config_(policy *policy) {
    pds_obj_key_t key = policy->key();

    memcpy(&key_, &key, sizeof(key_));
    af_ = policy->af();
    dir_ = policy->dir();
    num_rules_ = policy->num_rules();
    return SDK_RET_OK;
}

sdk_ret_t
policy::program_create(api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Programming security policy %s", key_.str());
    ret = impl_->program_hw(this, obj_ctxt);
    // for container objects, element count can change during update processing
    // as individual rule add/del/upd can happen in the same batch as policy
    // create, so we need to reflect that in the object
    if (ret == SDK_RET_OK) {
        num_rules_ =
            obj_ctxt->api_params->policy_spec.rule_info->num_rules;
    }
    return ret;
}

sdk_ret_t
policy::compute_update(api_obj_ctxt_t *obj_ctxt) {
    pds_policy_spec_t *spec = &obj_ctxt->api_params->policy_spec;

    // we can change individual rules in the policy but not the address family
    if (af_ != spec->rule_info->af) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"address family\" "
                      "on policy %s", key_.str());
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static bool
subnet_upd_walk_cb_ (void *api_obj, void *ctxt) {
    subnet_entry *subnet;
    policy_upd_ctxt_t *upd_ctxt = (policy_upd_ctxt_t *)ctxt;

    subnet = (subnet_entry *)api_framework_obj((api_base *)api_obj);
    if (upd_ctxt->policy_obj->dir() == RULE_DIR_INGRESS) {
        if (upd_ctxt->policy_obj->af() == IP_AF_IPV4) {
            for (uint8_t i = 0; i < subnet->num_ing_v4_policy(); i++) {
                if (subnet->ing_v4_policy(i) ==
                        upd_ctxt->policy_obj->key()) {
                    api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                                        OBJ_ID_POLICY, upd_ctxt->policy_obj,
                                        OBJ_ID_SUBNET, (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        } else {
            for (uint8_t i = 0; i < subnet->num_ing_v6_policy(); i++) {
                if (subnet->ing_v6_policy(i) ==
                        upd_ctxt->policy_obj->key()) {
                    api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                                        OBJ_ID_POLICY, upd_ctxt->policy_obj,
                                        OBJ_ID_SUBNET, (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        }
    } else {
        if (upd_ctxt->policy_obj->af() == IP_AF_IPV4) {
            for (uint8_t i = 0; i < subnet->num_egr_v4_policy(); i++) {
                if (subnet->egr_v4_policy(i) ==
                        upd_ctxt->policy_obj->key()) {
                    api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                                        OBJ_ID_POLICY, upd_ctxt->policy_obj,
                                        OBJ_ID_SUBNET, (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        } else {
            for (uint8_t i = 0; i < subnet->num_egr_v6_policy(); i++) {
                if (subnet->egr_v6_policy(i) ==
                        upd_ctxt->policy_obj->key()) {
                    api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                                        OBJ_ID_POLICY, upd_ctxt->policy_obj,
                                        OBJ_ID_SUBNET, (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        }
    }

end:

    return false;
}

// TODO: looks like this needs to be revisited since dir_ is not set anywhere
static bool
vnic_upd_walk_cb_ (void *api_obj, void *ctxt) {
    vnic_entry *vnic;
    policy_upd_ctxt_t *upd_ctxt = (policy_upd_ctxt_t *)ctxt;

    vnic = (vnic_entry *)api_framework_obj((api_base *)api_obj);
    if (upd_ctxt->policy_obj->dir() == RULE_DIR_INGRESS) {
        if (upd_ctxt->policy_obj->af() == IP_AF_IPV4) {
            for (uint8_t i = 0; i < vnic->num_ing_v4_policy(); i++) {
                if (vnic->ing_v4_policy(i) ==
                        upd_ctxt->policy_obj->key()) {
                    api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                                        OBJ_ID_POLICY, upd_ctxt->policy_obj,
                                        OBJ_ID_VNIC, (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        } else {
            for (uint8_t i = 0; i < vnic->num_ing_v6_policy(); i++) {
                if (vnic->ing_v6_policy(i) ==
                        upd_ctxt->policy_obj->key()) {
                    api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                                        OBJ_ID_POLICY, upd_ctxt->policy_obj,
                                        OBJ_ID_VNIC, (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        }
    } else {
        if (upd_ctxt->policy_obj->af() == IP_AF_IPV4) {
            for (uint8_t i = 0; i < vnic->num_egr_v4_policy(); i++) {
                if (vnic->egr_v4_policy(i) ==
                        upd_ctxt->policy_obj->key()) {
                    api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                                        OBJ_ID_POLICY, upd_ctxt->policy_obj,
                                        OBJ_ID_VNIC, (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        } else {
            for (uint8_t i = 0; i < vnic->num_egr_v6_policy(); i++) {
                if (vnic->egr_v6_policy(i) ==
                        upd_ctxt->policy_obj->key()) {
                    api_obj_add_to_deps(upd_ctxt->obj_ctxt->api_op,
                                        OBJ_ID_POLICY, upd_ctxt->policy_obj,
                                        OBJ_ID_VNIC, (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        }
    }

end:

    return false;
}

sdk_ret_t
policy::add_deps(api_obj_ctxt_t *obj_ctxt) {
    policy_upd_ctxt_t upd_ctxt = { 0 };

    upd_ctxt.policy_obj = this;
    upd_ctxt.obj_ctxt = obj_ctxt;

    // walk the subnets and add affected subnets to dependency list
    upd_ctxt.upd_bmap = PDS_SUBNET_UPD_POLICY;
    subnet_db()->walk(subnet_upd_walk_cb_, &upd_ctxt);

    // walk the vnics and add affected vnics to dependency list
    upd_ctxt.upd_bmap = PDS_VNIC_UPD_POLICY;
    vnic_db()->walk(vnic_upd_walk_cb_, &upd_ctxt);
    return SDK_RET_OK;
}

sdk_ret_t
policy::program_update(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    sdk_ret_t ret;

    PDS_TRACE_DEBUG("Updating security policy %s", key_.str());
    ret = impl_->update_hw(orig_obj, this, obj_ctxt);
    // for container objects, element count can change during update processing
    // as individual rule add/del/upd can happen in the same batch as policy
    // update, so we need to reflect that in the object
    if (ret == SDK_RET_OK) {
        num_rules_ =
            obj_ctxt->api_params->policy_spec.rule_info->num_rules;
    }
    return ret;
}

sdk_ret_t
policy::activate_config(pds_epoch_t epoch, api_op_t api_op,
                        api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
}

sdk_ret_t
policy::fill_spec_(pds_policy_spec_t *spec) {
    sdk_ret_t ret = SDK_RET_OK;

    memcpy(&spec->key, &key_, sizeof(pds_obj_key_t));
    if (spec->rule_info) {
        uint32_t num_rules = spec->rule_info->num_rules;
        spec->rule_info->af = af_;
        if (!num_rules) {
            // set num rules and return
            spec->rule_info->num_rules = num_rules_;
        } else if (num_rules < num_rules_) {
            // buffer is smaller, read all rules and copy over the
            // requested number allocate memory for reading all the rules
            rule_info_t *rule_info =
                (rule_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                          POLICY_RULE_INFO_SIZE(num_rules_));
            rule_info->num_rules = num_rules_;
            // retrieve all rules
            ret = policy_db()->retrieve_rules(&key_, rule_info);
            if (ret != SDK_RET_OK) {
                SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, rule_info);
                return ret;
            }
            // copy over requested number of rules
            memcpy(spec->rule_info, rule_info, POLICY_RULE_INFO_SIZE(num_rules));
            spec->rule_info->num_rules = num_rules;
            // free allocated memory
            SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, rule_info);
        } else {
            // retrieve rules from lmdb
            ret = policy_db()->retrieve_rules(&key_, spec->rule_info);
        }
    }
    return ret;
}

sdk_ret_t
policy::read(pds_policy_info_t *info) {
    sdk_ret_t ret;

    ret = fill_spec_(&info->spec);
    if (ret != SDK_RET_OK) {
        return ret;
    }
    return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                          (impl::obj_info_t *)info);
}

sdk_ret_t
policy::add_to_db(void) {
    return policy_db()->insert(this);
}

sdk_ret_t
policy::del_from_db(void) {
    if (policy_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
policy::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (policy_db()->remove((policy *)orig_obj)) {
        return policy_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
policy::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_POLICY, this);
}

policy_rule *
policy_rule::factory(pds_policy_rule_spec_t *spec) {
    policy_rule *rule;

    rule = policy_rule_db()->alloc();
    if (rule) {
        new (rule) policy_rule();
    }
    return rule;
}

void
policy_rule::destroy(policy_rule *rule) {
    rule->~policy_rule();
    policy_rule_db()->free(rule);
}

api_base *
policy_rule::clone(api_ctxt_t *api_ctxt) {
    policy_rule *cloned_rule;

    cloned_rule = policy_rule_db()->alloc();
    if (cloned_rule) {
        new (cloned_rule) policy_rule();
        if (cloned_rule->init_config(api_ctxt) != SDK_RET_OK) {
            goto error;
        }
    }
    return cloned_rule;

error:

    cloned_rule->~policy_rule();
    policy_rule_db()->free(cloned_rule);
    return NULL;
}

sdk_ret_t
policy_rule::free(policy_rule *rule) {
    rule->~policy_rule();
    policy_rule_db()->free(rule);
    return SDK_RET_OK;
}

policy_rule *
policy_rule::build(pds_policy_rule_key_t *key) {
    policy_rule *rule;

    rule = policy_rule_db()->alloc();
    if (rule) {
        new (rule) policy_rule();
        memcpy(&rule->key_, key, sizeof(*key));
    }
    return rule;
}

void
policy_rule::soft_delete(policy_rule *rule) {
    rule->del_from_db();
    rule->~policy_rule();
    policy_rule_db()->free(rule);
}

sdk_ret_t
policy_rule::init_config(api_ctxt_t *api_ctxt) {
    pds_policy_rule_spec_t *spec = &api_ctxt->api_params->policy_rule_spec;

    memcpy(&key_, &spec->key, sizeof(key_));
    return SDK_RET_OK;
}

sdk_ret_t
policy_rule::add_deps(api_obj_ctxt_t *obj_ctxt) {
    policy *policy;
    pds_obj_key_t policy_key;

    if ((obj_ctxt->api_op == API_OP_CREATE) ||
        (obj_ctxt->api_op == API_OP_UPDATE)) {
        if (obj_ctxt->cloned_obj) {
            policy_key = ((policy_rule *)obj_ctxt->cloned_obj)->key_.policy_id;
        } else {
            policy_key = key_.policy_id;
        }
        policy = policy_find(&policy_key);
        if (!policy) {
            PDS_TRACE_ERR("Failed to perform api op %u on rule %s, "
                          "policy %s not found", obj_ctxt->api_op,
                          key_.rule_id.str(), policy_key.str());
        }
        api_obj_add_to_deps(API_OP_UPDATE, OBJ_ID_POLICY_RULE, this,
                            OBJ_ID_POLICY, policy,
                            (obj_ctxt->api_op == API_OP_CREATE) ?
                                 PDS_POLICY_UPD_RULE_ADD :
                                 PDS_POLICY_UPD_RULE_UPD);
    } else {
        policy = policy_find(&key_.policy_id);
        if (!policy) {
            PDS_TRACE_ERR("Failed to perform api op %u on rule %s, "
                          "policy %s not found", obj_ctxt->api_op,
                          key_.rule_id.str(), key_.policy_id.str());
        }
        api_obj_add_to_deps(API_OP_UPDATE, OBJ_ID_POLICY_RULE, this,
                            OBJ_ID_POLICY, policy, PDS_POLICY_UPD_RULE_DEL);
    }
    return SDK_RET_OK;
}

sdk_ret_t
policy_rule::read(pds_policy_rule_info_t *info) {
    sdk_ret_t ret;
    uint32_t num_rules;
    rule_info_t *rule_info;
    pds_policy_info_t policy_info;

    memset(&policy_info, 0, sizeof(policy_info));
    // get number of rules
    policy_info.spec.rule_info =
        (rule_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                  POLICY_RULE_INFO_SIZE(0));
    ret = pds_policy_read(&key_.policy_id, &policy_info);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to get policy %s size, err %u",
                      key_.policy_id.str(), ret);
        goto end;
    }
    num_rules = policy_info.spec.rule_info->num_rules;
    SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, policy_info.spec.rule_info);
    policy_info.spec.rule_info =
        (rule_info_t *)SDK_CALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                                  POLICY_RULE_INFO_SIZE(num_rules));
    policy_info.spec.rule_info->num_rules = num_rules;
    ret = pds_policy_read(&key_.policy_id, &policy_info);
    if (ret != SDK_RET_OK) {
        PDS_TRACE_ERR("Failed to read all policy %s rules, err %u",
                      key_.policy_id.str(), ret);
        goto end;
    }
    // walk the route table and find the route of interest
    rule_info = policy_info.spec.rule_info;
    for (uint32_t i = 0; i < num_rules; i++) {
        if (key_.policy_id == rule_info->rules[i].key) {
            // fill the spec
            memcpy(&info->spec.key, &key_, sizeof(key_));
            memcpy(&info->spec.attrs, &rule_info->rules[i].attrs,
                   sizeof(info->spec.attrs));
            break;
        }
        // continue the search
    }

end:

    if (policy_info.spec.rule_info) {
         SDK_FREE(PDS_MEM_ALLOC_SECURITY_POLICY, policy_info.spec.rule_info);
         policy_info.spec.rule_info = NULL;
    }
    return ret;
}

sdk_ret_t
policy_rule::add_to_db(void) {
    return policy_rule_db()->insert(this);
}

sdk_ret_t
policy_rule::del_from_db(void) {
    if (policy_rule_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
policy_rule::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (policy_rule_db()->remove((policy_rule *)orig_obj)) {
        return policy_rule_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
policy_rule::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_POLICY_RULE, this);
}

/// @}    // end of PDS_POLICY

}    // namespace api
