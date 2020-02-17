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
    ht_ctxt_.reset();
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
        cloned_policy->impl_ = impl_->clone();
        if (unlikely(cloned_policy->impl_ == NULL)) {
            PDS_TRACE_ERR("Failed to clone policy %s impl", key2str().c_str());
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
policy::reserve_resources(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
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
    memcpy(&this->key_, &spec->key, sizeof(pds_obj_key_t));
    return SDK_RET_OK;
}

sdk_ret_t
policy::program_create(api_obj_ctxt_t *obj_ctxt) {
    PDS_TRACE_DEBUG("Programming security policy %s, af %u, dir %u",
                    key2str().c_str(), af_, dir_);
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
policy::compute_update(api_obj_ctxt_t *obj_ctxt) {
    pds_policy_spec_t *spec = &obj_ctxt->api_params->policy_spec;

    if ((af_ != spec->af) || (dir_ != spec->direction)) {
        PDS_TRACE_ERR("Attempt to modify immutable attr \"address family\" "
                      "or \"direction\" from %u to %u on policy table %s",
                      dir_, spec->direction, key2str().c_str());
        return SDK_RET_INVALID_ARG;
    }
    // in all other cases we have to recompute the policy table and program in
    // the datapath
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
                    api_obj_add_to_deps(OBJ_ID_SUBNET,
                                        upd_ctxt->obj_ctxt->api_op,
                                        (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        } else {
            for (uint8_t i = 0; i < subnet->num_ing_v6_policy(); i++) {
                if (subnet->ing_v6_policy(i) ==
                        upd_ctxt->policy_obj->key()) {
                    api_obj_add_to_deps(OBJ_ID_SUBNET,
                                        upd_ctxt->obj_ctxt->api_op,
                                        (api_base *)api_obj,
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
                    api_obj_add_to_deps(OBJ_ID_SUBNET,
                                        upd_ctxt->obj_ctxt->api_op,
                                        (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        } else {
            for (uint8_t i = 0; i < subnet->num_egr_v6_policy(); i++) {
                if (subnet->egr_v6_policy(i) ==
                        upd_ctxt->policy_obj->key()) {
                    api_obj_add_to_deps(OBJ_ID_SUBNET,
                                        upd_ctxt->obj_ctxt->api_op,
                                        (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        }
    }

end:

    return false;
}

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
                    api_obj_add_to_deps(OBJ_ID_VNIC, upd_ctxt->obj_ctxt->api_op,
                                        (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        } else {
            for (uint8_t i = 0; i < vnic->num_ing_v6_policy(); i++) {
                if (vnic->ing_v6_policy(i) ==
                        upd_ctxt->policy_obj->key()) {
                    api_obj_add_to_deps(OBJ_ID_VNIC, upd_ctxt->obj_ctxt->api_op,
                                        (api_base *)api_obj,
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
                    api_obj_add_to_deps(OBJ_ID_VNIC, upd_ctxt->obj_ctxt->api_op,
                                        (api_base *)api_obj,
                                        upd_ctxt->upd_bmap);
                    goto end;
                }
            }
        } else {
            for (uint8_t i = 0; i < vnic->num_egr_v6_policy(); i++) {
                if (vnic->egr_v6_policy(i) ==
                        upd_ctxt->policy_obj->key()) {
                    api_obj_add_to_deps(OBJ_ID_VNIC, upd_ctxt->obj_ctxt->api_op,
                                        (api_base *)api_obj,
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
    // update is same as programming route table in different region
    return impl_->program_hw(this, obj_ctxt);
}

sdk_ret_t
policy::activate_config(pds_epoch_t epoch, api_op_t api_op,
                        api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    return impl_->activate_hw(this, orig_obj, epoch, api_op, obj_ctxt);
}

void
policy::fill_spec_(pds_policy_spec_t *spec) {
    memcpy(&spec->key, &key_, sizeof(pds_obj_key_t));
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

/// @}    // end of PDS_POLICY

}    // namespace api
