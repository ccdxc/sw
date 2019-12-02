/**
 * Copyright (c) 2019 Pensando Systems, Inc.
 *
 * @file    policy.cc
 *
 * @brief   policy handling
 */

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/policy.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

/**
 * @defgroup PDS_POLICY - security policy functionality
 * @ingroup PDS_POLICY
 * @{
 */

policy::policy() {
    //SDK_SPINLOCK_INIT(&slock_, PTHREAD_PROCESS_PRIVATE);
    ht_ctxt_.reset();
}

policy *
policy::factory(pds_policy_spec_t *spec) {
    policy    *new_policy;

    if (spec->policy_type != POLICY_TYPE_FIREWALL) {
        /**< we don't support any other policy type currently */
        return NULL;
    }
    /**< create security policy instance with defaults, if any */
    new_policy = policy_db()->alloc();
    if (new_policy) {
        new (new_policy) policy();
        new_policy->impl_ = impl_base::factory(impl::IMPL_OBJ_ID_SECURITY_POLICY,
                                               spec);
        if (new_policy->impl_ == NULL) {
            policy::destroy(new_policy);
            return NULL;
        }
    }
    return new_policy;
}

policy::~policy() {
    // TODO: fix me
    //SDK_SPINLOCK_DESTROY(&slock_);
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
        cloned_policy->impl_ = impl_->clone();
        if (unlikely(cloned_policy->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone policy %u impl", key_.id);
            goto error;
        }
        cloned_policy->init_config(api_ctxt);
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
policy::init_config(api_ctxt_t *api_ctxt) {
    pds_policy_spec_t    *spec;

    spec = &api_ctxt->api_params->policy_spec;
    this->af_ = spec->af;
    this->dir_ = spec->direction;
    memcpy(&this->key_, &spec->key,
           sizeof(pds_policy_key_t));
    return SDK_RET_OK;
}

sdk_ret_t
policy::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
}

sdk_ret_t
policy::program_create(obj_ctxt_t *obj_ctxt) {
    PDS_TRACE_DEBUG("Programming security policy %u", key_.id);
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
policy::nuke_resources_(void) {
    return impl_->nuke_resources(this);
}

sdk_ret_t
policy::release_resources(void) {
    return impl_->release_resources(this);
}

sdk_ret_t
policy::cleanup_config(obj_ctxt_t *obj_ctxt) {
    return impl_->cleanup_hw(this, obj_ctxt);
}

sdk_ret_t
policy::program_update(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
policy::activate_config(pds_epoch_t epoch, api_op_t api_op,
                        api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
}

sdk_ret_t
policy::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return sdk::SDK_RET_INVALID_OP;
}

sdk_ret_t
policy::add_to_db(void) {
    return policy_db()->policy_ht()->insert_with_key(&key_,
                                                           this, &ht_ctxt_);
}

sdk_ret_t
policy::del_from_db(void) {
    policy_db()->policy_ht()->remove(&key_);
    return SDK_RET_OK;
}

sdk_ret_t
policy::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_POLICY, this);
}

void
policy::fill_spec_(pds_policy_spec_t *spec) {
    memcpy(&spec->key, &this->key_, sizeof(pds_policy_key_t));
    spec->af = af_;
    spec->direction = dir_;
    spec->num_rules = 0;
    // rules are not stored anywhere
    spec->rules = NULL;
    return;
}

sdk_ret_t
policy::read(pds_policy_info_t *info) {
    fill_spec_(&info->spec);
    return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                          (impl::obj_info_t *)info);
}
/** @} */    // end of PDS_POLICY

}    // namespace api
