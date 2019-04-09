//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the policy test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/apollo/test/utils/policy.hpp"
#include "nic/apollo/test/utils/utils.hpp"
#include <iostream>

using namespace std;
namespace api_test {

policy_util::policy_util(pds_policy_id_t id,
                         uint32_t num_rules,
                         rule_t *rules,
                         rule_dir_t direction,
                         uint8_t af,
                         policy_type_t type) {
    __init();
    this->id = id;
    this->num_rules = num_rules;
    this->rules = rules;
    this->direction = direction;
    this->type = type;
    this->af = af;
}

policy_util::~policy_util() {}


sdk::sdk_ret_t
policy_util::create() {
    pds_policy_spec_t spec = {};

    spec.key.id = this->id;
    spec.num_rules = this->num_rules;
    spec.rules = this->rules;
    //memcpy(&spec.rules, this->rules, sizeof(rule_t) * num_rules);
    spec.direction = this->direction;
    spec.policy_type = this->type;
    spec.af = this->af;

    return (pds_policy_create(&spec));
}

sdk::sdk_ret_t
policy_util::read(pds_policy_info_t *info, bool compare_spec)
{
    sdk_ret_t rv = sdk::SDK_RET_OK;
    pds_policy_key_t key;

    memset(&key, 0, sizeof(pds_policy_key_t));
    memset(info, 0, sizeof(pds_policy_info_t));
    key.id = this->id;
    //rv = pds_policy_read(&key, info);
    if (rv != sdk::SDK_RET_OK) {
        return rv;
    }
    if (compare_spec) {

    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
policy_util::del() {
    pds_policy_key_t policy_key = {};
    policy_key.id = this->id;
    return (pds_policy_delete(&policy_key));
}

static inline sdk::sdk_ret_t
policy_util_object_stepper (pds_policy_key_t start_key,
                            uint32_t num_objs,
                            utils_op_t op, sdk_ret_t expected_result)
{
#if 0
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    pds_policy_info_t info = {};

    if (start_key.id == 0) start_key.id = 1;

    if (op == OP_MANY_CREATE) {

    }
    for (uint32_t idx = start_key.id; idx < start_key.id + num_objs; idx++) {
        policy_util policy_obj(idx);
        switch (op) {
        case OP_MANY_CREATE:
            SDK_ASSERT((rv = policy_obj.create()) == sdk::SDK_RET_OK);
            break;
        case OP_MANY_DELETE:
            rv = policy_obj.del();
            break;
        case OP_MANY_READ:
            rv = policy_obj.read(&info, TRUE);
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return rv;
        }
    }
#endif
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
policy_util::many_create(pds_policy_key_t start_key,
                         uint32_t num_policy) {
    return (policy_util_object_stepper(start_key,
                                       num_policy, OP_MANY_CREATE,
                                       sdk::SDK_RET_OK));
}

sdk::sdk_ret_t
policy_util::many_read(pds_policy_key_t start_key, uint32_t num_policy,
                       sdk::sdk_ret_t expected_result) {
    return (policy_util_object_stepper(start_key, num_policy,
                                       OP_MANY_READ, expected_result));
}

sdk::sdk_ret_t
policy_util::many_delete(pds_policy_key_t start_key, uint32_t num_policy) {
    return (policy_util_object_stepper(start_key, num_policy,
                                       OP_MANY_DELETE, sdk::SDK_RET_OK));
}

ostream& operator << (ostream& os, policy_util& obj)
{
    os << "policy id : " << obj.id;
    return os;
}

void policy_util::__init()
{
    this->id = 0;
}

}    // namespace api_test
