//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the policy test utility routines implementation
///
//----------------------------------------------------------------------------

#include <iostream>
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
                         policy_type_t type)
{
    __init();
    this->id = id;
    this->num_rules = num_rules;
    this->rules = rules;
    this->direction = direction;
    this->type = type;
    this->af = af;
}

policy_util::policy_util(policy_seed_stepper_t *seed)
{
    __init();
    this->id = seed->id;
    this->num_rules = seed->num_rules;
    this->stateful_rules = seed->stateful_rules;
    this->direction = seed->direction;
    this->type = seed->type;
    this->af = seed->af;
    this->pfx = seed->pfx;
}

policy_util::~policy_util() {}

static inline void
create_rules(std::string pfx, uint16_t num_rules,
             rule_t **rules, uint16_t stateful_rules)
{
    ip_prefix_t ip_pfx;

    *rules =
        (rule_t *)SDK_MALLOC(PDS_MEM_ALLOC_SECURITY_POLICY,
                             num_rules * sizeof(rule_t));
    extract_ip_pfx((char *)pfx.c_str(), &ip_pfx);
    for (uint32_t i = 0; i < num_rules; i++) {
        if (stateful_rules-- < PDS_MAX_RULES_PER_SECURITY_POLICY) {
            (*rules)[i].stateful = true;
            (*rules)[i].match.l4_match.sport_range.port_lo = 0;
            (*rules)[i].match.l4_match.sport_range.port_hi = 65535;
            (*rules)[i].match.l4_match.dport_range.port_lo = 0;
            (*rules)[i].match.l4_match.dport_range.port_hi = 65535;
        }
        else {
            (*rules)[i].stateful = false;
            (*rules)[i].match.l4_match.icmp_type = 1;
            (*rules)[i].match.l4_match.icmp_code = 1;
        }
        (*rules)[i].match.l3_match.ip_proto = 1;
        str2ipv4pfx((char*)pfx.c_str(),
                    &(*rules)[i].match.l3_match.ip_pfx);
        // TODO:only IPv4 support available
        ip_pfx.addr.addr.v4_addr += 1;
        pfx = ippfx2str(&ip_pfx);
        (*rules)[i].action_data.fw_action.action = SECURITY_RULE_ACTION_ALLOW;
    }
}

sdk::sdk_ret_t
policy_util::create()
{
    pds_policy_spec_t spec = {};

    spec.key.id = this->id;
    spec.num_rules = this->num_rules;
    create_rules(this->pfx, this->num_rules, &(this->rules), this->stateful_rules);
    spec.rules = this->rules;
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
    int af;

    memset(&key, 0, sizeof(pds_policy_key_t));
    memset(info, 0, sizeof(pds_policy_info_t));
    key.id = this->id;
    rv = pds_policy_read(&key, info);
    if (rv != sdk::SDK_RET_OK) {
        return rv;
    }

    if (info->spec.af)
        af = IP_AF_IPV6;
    else
        af = IP_AF_IPV4;
    std::cout << "key: " << info->spec.key.id << ", direction: "
        << info->spec.direction << ", af: " << af << "\n";
    std::cout << "pfx: " << this->pfx <<"\n";

    if (compare_spec) {
        SDK_ASSERT(info->spec.key.id == this->id);
        //SDK_ASSERT(info->spec.num_rules == this->num_rules);
        //SDK_ASSERT(memcmp(info->spec.rules, this->rules, num_rules * sizeof(rule_t)));
        SDK_ASSERT(info->spec.direction == this->direction);
        //SDK_ASSERT(info->spec.policy_type == this->type);
        SDK_ASSERT(info->spec.af == this->af);
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
policy_util::del()
{
    pds_policy_key_t policy_key = {};
    policy_key.id = this->id;
    return (pds_policy_delete(&policy_key));
}

static inline sdk::sdk_ret_t
policy_util_object_stepper (policy_seed_stepper_t *seed, uint32_t num_objs,
                            utils_op_t op, sdk_ret_t expected_result)
{
    uint32_t init_id;
    std::string init_pfx = seed->pfx;
    sdk::sdk_ret_t rv = sdk::SDK_RET_OK;
    pds_policy_info_t info = {};
    ip_prefix_t ip_pfx;
    ip_addr_t ipaddr;

    extract_ip_pfx((char *)seed->pfx.c_str(), &ip_pfx);
    if (seed->id == 0) seed->id = 1;
    num_objs += seed->id;
    init_id = seed->id;
    for (uint32_t idx = init_id; idx < num_objs; idx++) {
        seed->id = idx;
        policy_util policy_obj(seed);
        switch (op) {
        case OP_MANY_CREATE:
            rv = policy_obj.create();
            break;
        case OP_MANY_DELETE:
            rv = policy_obj.del();
            break;
        case OP_MANY_READ:
            rv = policy_obj.read(&info);
            break;
        default:
            return sdk::SDK_RET_INVALID_OP;
        }
        if (rv != expected_result) {
            return rv;
        }

        ip_prefix_ip_next(&ip_pfx, &ipaddr);
        ip_pfx.addr = ipaddr;
        seed->pfx = ippfx2str(&ip_pfx);
    }
    // roll back to original value of seed->id
    seed->id = init_id;
    seed->pfx = init_pfx;
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
policy_util::many_create(policy_seed_stepper_t *seed,
                         uint32_t num_policy)
{
    return (policy_util_object_stepper(seed, num_policy,
                                       OP_MANY_CREATE, sdk::SDK_RET_OK));
}

sdk::sdk_ret_t
policy_util::many_read(policy_seed_stepper_t *seed, uint32_t num_policy,
                       sdk::sdk_ret_t expected_result)
{
    return (policy_util_object_stepper(seed, num_policy,
                                       OP_MANY_READ, expected_result));
}

sdk::sdk_ret_t
policy_util::many_delete(policy_seed_stepper_t *seed, uint32_t num_policy)
{
    return (policy_util_object_stepper(seed, num_policy,
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
