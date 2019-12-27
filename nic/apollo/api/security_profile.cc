//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// Security profile API object handling
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/core/mem.hpp"
#include "nic/apollo/api/core/msg.h"
#include "nic/apollo/framework/api_base.hpp"
#include "nic/apollo/framework/api_engine.hpp"
#include "nic/apollo/framework/api_params.hpp"
#include "nic/apollo/api/security_profile.hpp"
#include "nic/apollo/api/pds_state.hpp"

namespace api {

// security profile API implementation
security_profile::security_profile() {
    ht_ctxt_.reset();
}

security_profile::~security_profile() {
}

security_profile *
security_profile::factory(pds_security_profile_spec_t *spec) {
    security_profile *profile;

    // create security profile entry with defaults, if any
    profile = policy_db()->alloc_security_profile();
    if (profile) {
        new (profile) security_profile();
    }
    return profile;
}

void
security_profile::destroy(security_profile *profile) {
    profile->~security_profile();
    policy_db()->free(profile);
}

api_base *
security_profile::clone(api_ctxt_t *api_ctxt) {
    security_profile *cloned_profile;

    cloned_profile = policy_db()->alloc_security_profile();
    if (cloned_profile) {
        new (cloned_profile) security_profile();
        cloned_profile->init_config(api_ctxt);
    }
    return cloned_profile;
}

sdk_ret_t
security_profile::free(security_profile *profile) {
    profile->~security_profile();
    policy_db()->free(profile);
    return SDK_RET_OK;
}

security_profile *
security_profile::build(pds_security_profile_key_t *key) {
    security_profile *profile;

    profile = policy_db()->alloc_security_profile();
    if (profile) {
        new (profile) security_profile();
        memcpy(&profile->key_, key, sizeof(*key));
    }
    return profile;
}

void
security_profile::soft_delete(security_profile *profile) {
    profile->del_from_db();
    profile->~security_profile();
    policy_db()->free(profile);
}

sdk_ret_t
security_profile::init_config(api_ctxt_t *api_ctxt) {
    key_.id = api_ctxt->api_params->security_profile_spec.key.id;
    return SDK_RET_OK;
}

sdk_ret_t
security_profile::populate_msg(pds_msg_t *msg, api_obj_ctxt_t *obj_ctxt) {
    msg->type = PDS_MSG_TYPE_CFG;
    msg->id = PDS_CFG_MSG_ID_SECURITY_PROFILE;
    msg->cfg_msg.op = obj_ctxt->api_op;
    msg->cfg_msg.obj_id = OBJ_ID_SECURITY_PROFILE;
    if (obj_ctxt->api_op == API_OP_DELETE) {
        msg->cfg_msg.security_profile.key =
            obj_ctxt->api_params->security_profile_key;
    } else {
        msg->cfg_msg.security_profile.spec =
            obj_ctxt->api_params->security_profile_spec;
    }
    return SDK_RET_OK;
}

void
security_profile::fill_spec_(pds_security_profile_spec_t *spec) {
}

sdk_ret_t
security_profile::read(pds_security_profile_info_t *info) {
    return SDK_RET_INVALID_OP;
}

sdk_ret_t
security_profile::add_to_db(void) {
    return policy_db()->insert(this);
}

sdk_ret_t
security_profile::del_from_db(void) {
    if (policy_db()->remove(this)) {
        return SDK_RET_OK;
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
security_profile::update_db(api_base *orig_obj, api_obj_ctxt_t *obj_ctxt) {
    if (policy_db()->remove((security_profile *)orig_obj)) {
        return policy_db()->insert(this);
    }
    return SDK_RET_ENTRY_NOT_FOUND;
}

sdk_ret_t
security_profile::delay_delete(void) {
    return delay_delete_to_slab(PDS_SLAB_ID_SECURITY_PROFILE, this);
}

}    // namespace api
