//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#ifndef __TEST_UTILS_POLICY_HPP__
#define __TEST_UTILS_POLICY_HPP__

#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/test/utils/api_base.hpp"
#include "nic/apollo/test/utils/feeder.hpp"
#include "nic/apollo/api/policy_utils.hpp"

namespace api_test {

/// Policy test feeder class
class policy_feeder : public feeder {
public:
    /// Test params
    pds_policy_spec_t spec;
    uint16_t stateful_rules;
    std::string cidr_str;

    /// \brief constructor
    policy_feeder() { };
    policy_feeder(policy_feeder& feeder) {
        init(feeder.spec.key, feeder.stateful_rules,
             feeder.spec.direction, feeder.spec.policy_type, feeder.spec.af,
             feeder.cidr_str, feeder.num_obj);
    }

    // Initialize feeder with the base set of values
    void init(pds_policy_key_t key, uint16_t num_rules, rule_dir_t direction,
              policy_type_t type, uint8_t af, std::string cidr_str,
              uint32_t num_policy = 1);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void key_build(pds_policy_key_t *key) const;
    void spec_build(pds_policy_spec_t *spec) const;

    // Compare routines
    bool key_compare(const pds_policy_key_t *key) const;
    bool spec_compare(const pds_policy_spec_t *spec) const;

};

// Dump prototypes
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

// Function prototypes
void sample_policy_setup(pds_batch_ctxt_t bctxt);
void sample_policy_teardown(pds_batch_ctxt_t bctxt);

}    // namespace api_test

#endif    // __TEST_UTILS_POLICY_HPP__
