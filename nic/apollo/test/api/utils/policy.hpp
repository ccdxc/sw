//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_POLICY_HPP__
#define __TEST_API_UTILS_POLICY_HPP__

#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

enum rule_attrs {
    RULE_ATTR_STATEFUL              = bit(0),
    RULE_ATTR_PRIORITY              = bit(1),
    RULE_ATTR_L3_MATCH              = bit(2),
    RULE_ATTR_L4_MATCH              = bit(3),
    RULE_ATTR_ACTION                = bit(4),
};

enum rule_info_attrs {
    RULE_INFO_ATTR_AF               = bit(0),
    RULE_INFO_ATTR_RULE             = bit(1),
    RULE_INFO_ATTR_DEFAULT_ACTION   = bit(2),
};

enum layer_t {
    L3,
    L4,
    LAYER_ALL,
};

enum action_t {
    ALLOW,
    DENY,
    RANDOM,
};

// Policy test feeder class
class policy_feeder : public feeder {
public:
    // Test params
    pds_policy_spec_t spec;
    uint8_t af;
    uint32_t num_rules;
    uint16_t stateful_rules;
    std::string cidr_str;

    // constructor
    policy_feeder() { };
    policy_feeder(policy_feeder& feeder) {
        init(feeder.spec.key, feeder.stateful_rules,
             feeder.af, feeder.cidr_str, feeder.num_obj, feeder.num_rules);
    }

    // Initialize feeder with the base set of values
    void init(pds_obj_key_t key, uint16_t stateful_rules, uint8_t af,
              std::string cidr_str, uint32_t num_policy = 1,
              uint32_t num_rules_per_policy = 0);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_obj_key_t *key) const;
    void spec_build(pds_policy_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_obj_key_t *key) const;
    bool spec_compare(const pds_policy_spec_t *spec) const;
    bool status_compare(const pds_policy_status_t *status1,
                        const pds_policy_status_t *status2) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_policy_spec_t *spec) {
    os << &spec->key
       << " af: " << +(spec->rule_info ? (uint32_t)spec->rule_info->af : 0)
       << " num rules: " << (spec->rule_info ? spec->rule_info->num_rules : 0);
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_policy_info_t *obj) {
    os << " Policy info =>" << &obj->spec << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const policy_feeder& obj) {
    os << "Policy feeder =>" << &obj.spec
       << " cidr_str: " << obj.cidr_str;
    return os;
}

// CRUD prototypes
API_CREATE(policy);
API_READ(policy);
API_UPDATE(policy);
API_DELETE(policy);

// Policy crud helper prototypes
void policy_create(policy_feeder& feeder);
void policy_read(policy_feeder& feeder, sdk_ret_t exp_result = SDK_RET_OK);
void policy_rule_update(policy_feeder& feeder, pds_policy_spec_t *spec,
                        uint64_t chg_bmap, sdk_ret_t exp_result = SDK_RET_OK);
void policy_rule_info_update(policy_feeder& feeder, pds_policy_spec_t *spec,
                             uint64_t chg_bmap, sdk_ret_t exp_result =
                                                                SDK_RET_OK);
void policy_update(policy_feeder& feeder);
void policy_delete(policy_feeder& feeder);

void create_rules(std::string cidr_str, uint8_t af=IP_AF_IPV4,
                  uint16_t stateful_rules=0, rule_info_t **rule_info=NULL,
                  uint16_t num_rules=1, layer_t layer=LAYER_ALL,
                  action_t action=ALLOW, uint32_t priority=0,
                  bool wildcard=false);

// Function prototypes
void sample_policy_setup(pds_batch_ctxt_t bctxt);
void sample_policy_teardown(pds_batch_ctxt_t bctxt);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_POLICY_HPP__
