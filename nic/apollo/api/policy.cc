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
#include "nic/apollo/api/policy.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

/**
 * @defgroup PDS_POLICY - security policy functionality
 * @ingroup PDS_POLICY
 * @{
 */

typedef struct policy_update_ctxt_s {
    policy *policy_obj;
    obj_ctxt_t *obj_ctxt;
} __PACK__ policy_update_ctxt_t;

policy::policy() {
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
policy::reserve_resources(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->reserve_resources(this, obj_ctxt);
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
    this->af_ = spec->af;
    this->dir_ = spec->direction;
    memcpy(&this->key_, &spec->key,
           sizeof(pds_policy_key_t));
    return SDK_RET_OK;
}

sdk_ret_t
policy::program_create(obj_ctxt_t *obj_ctxt) {
    PDS_TRACE_DEBUG("Programming security policy %u", key_.id);
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
policy::compute_update(obj_ctxt_t *obj_ctxt) {
    pds_policy_spec_t *spec = &obj_ctxt->api_params->policy_spec;

    if ((af_ != spec->af) || (dir_ != spec->direction)) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"address family\" "
                      "or \"direction\" from %u to %u on policy table %u",
                      dir_, spec->direction, key_.id);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static bool
subnet_upd_walk_cb_(void *api_obj, void *ctxt) {
    subnet_entry *subnet = (subnet_entry *)api_obj;
    policy_update_ctxt_t *upd_ctxt = (policy_update_ctxt_t *)ctxt;

    if (upd_ctxt->policy_obj->dir() == RULE_DIR_INGRESS) {
        if (upd_ctxt->policy_obj->af() == IP_AF_IPV4) {
            for (uint8_t i = 0; i < subnet->num_ing_v4_policy(); i++) {
                if (subnet->ing_v4_policy(i).id ==
                        upd_ctxt->policy_obj->key().id) {
                    upd_ctxt->obj_ctxt->add_deps(subnet, API_OP_UPDATE);
                    return false;
                }
            }
        } else {
            for (uint8_t i = 0; i < subnet->num_ing_v6_policy(); i++) {
                if (subnet->ing_v6_policy(i).id ==
                        upd_ctxt->policy_obj->key().id) {
                    upd_ctxt->obj_ctxt->add_deps(subnet, API_OP_UPDATE);
                    return false;
                }
            }
        }
    } else {
        if (upd_ctxt->policy_obj->af() == IP_AF_IPV4) {
            for (uint8_t i = 0; i < subnet->num_egr_v4_policy(); i++) {
                if (subnet->egr_v4_policy(i).id ==
                        upd_ctxt->policy_obj->key().id) {
                    upd_ctxt->obj_ctxt->add_deps(subnet, API_OP_UPDATE);
                    return false;
                }
            }
        } else {
            for (uint8_t i = 0; i < subnet->num_egr_v6_policy(); i++) {
                if (subnet->egr_v6_policy(i).id ==
                        upd_ctxt->policy_obj->key().id) {
                    upd_ctxt->obj_ctxt->add_deps(subnet, API_OP_UPDATE);
                    return false;
                }
            }
        }
    }
    return false;
}

#if 0
// TODO: we may have to do this impl layer as we don't have h/w ids here
//       to compare with and vnic object is not storing anything in s/w
static bool
vnic_upd_walk_cb_(void *api_obj, void *ctxt) {
    return false;
}
#endif

sdk_ret_t
policy::add_deps(obj_ctxt_t *obj_ctxt) {
    policy_update_ctxt_t upd_ctxt = { 0 };

    upd_ctxt.policy_obj = this;
    upd_ctxt.obj_ctxt = obj_ctxt;
    subnet_db()->walk(subnet_upd_walk_cb_, &upd_ctxt);
    //vnic_db()->walk(vnic_upd_walk_cb_, &upd_ctxt);
    return SDK_RET_OK;
}

sdk_ret_t
policy::program_update(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    // update is same as programming route table in different region
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
policy::activate_config(pds_epoch_t epoch, api_op_t api_op,
                        api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
}

void
policy::fill_spec_(pds_policy_spec_t *spec) {
    memcpy(&spec->key, &key_, sizeof(pds_policy_key_t));
    spec->af = af_;
    spec->direction = dir_;
    spec->num_rules = 0;
    // rules are not stored anywhere
    spec->rules = NULL;
}

sdk_ret_t
policy::read(pds_policy_info_t *info) {
    fill_spec_(&info->spec);
    return impl_->read_hw(this, (impl::obj_key_t *)(&info->spec.key),
                          (impl::obj_info_t *)info);
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
policy::update_db(api_base *orig_obj, obj_ctxt_t *obj_ctxt) {
    if (policy_db()->remove((policy *)orig_obj)) {
        return policy_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
policy::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_POLICY, this);
}

/** @} */    // end of PDS_POLICY

}    // namespace api
