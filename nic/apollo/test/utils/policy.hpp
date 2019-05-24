//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the policy test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_POLICY_HPP__
#define __TEST_UTILS_POLICY_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"

namespace api_test {

#define POLICY_MANY_CREATE(seed)                                        \
    ASSERT_TRUE(policy_util::many_create(seed) == sdk::SDK_RET_OK)

#define POLICY_MANY_READ(seed, expected_res)                            \
    ASSERT_TRUE(policy_util::many_read(seed, expected_res) == sdk::SDK_RET_OK)

#define POLICY_MANY_UPDATE(seed)                                        \
    ASSERT_TRUE(policy_util::many_update(seed) == sdk::SDK_RET_OK)

#define POLICY_MANY_DELETE(seed)                                        \
    ASSERT_TRUE(policy_util::many_delete(seed) == sdk::SDK_RET_OK)

#define POLICY_SEED_INIT policy_util::stepper_seed_init

typedef struct policy_seed_stepper_s {
    pds_policy_id_t id;
    uint16_t num_rules;
    uint16_t stateful_rules;
    rule_t *rules;
    rule_dir_t direction;
    policy_type_t type;
    uint8_t af;
    std::string pfx;
    uint32_t num_policy;
} policy_seed_stepper_t;

/// Policy test utility class
class policy_util {
public:

    /// Test params
    pds_policy_id_t id;
    uint16_t num_rules;
    uint16_t stateful_rules;
    rule_t *rules;
    rule_dir_t direction;
    policy_type_t type;
    uint8_t af;
    std::string pfx;

    /// \brief constructor
    policy_util(pds_policy_id_t id, uint32_t, rule_t *,
                rule_dir_t dir = RULE_DIR_INGRESS,
                uint8_t af = IP_AF_IPV4,
                policy_type_t type = POLICY_TYPE_FIREWALL);

    /// parameterized constructor
    policy_util(policy_seed_stepper_t *seed);

    /// \brief destructor
    ~policy_util();

    /// \brief Create policy
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void);

    /// \brief Read policy
    ///
    /// \param[out] info policy information
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_policy_info_t *info) const;

    /// \brief Update the policy
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void);

    /// \brief Delete policy
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void) const;

    /// \brief Create many policies
    ///
    /// \param[in] seed seed for the policy
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(policy_seed_stepper_t *seed);

    /// \brief Create many policies
    ///
    /// \param[in] seed seed for the policy
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_update(policy_seed_stepper_t *seed);

    /// \brief Read many policies
    ///
    /// \param[in] seed seed for the policy
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(policy_seed_stepper_t *seed,
                               sdk::sdk_ret_t expected_res = sdk::SDK_RET_OK);

    /// \brief Delete multiple policies
    /// Delete "num_policy" policy starting from id
    ///
    /// \param[in] seed seed for the policies to be deleted
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(policy_seed_stepper_t *seed);

    /// \brief Initialize the seed for policy
    ///
    /// \param[out] seed policy seed
    /// \param[in] id policy id
    /// \param[in] stateless_rules number of stateless rules in policy
    /// \param[in] dir direction
    /// \param[in] type policy type
    /// \param[in] af address family type
    /// \param[in] pfx base prefix
    /// \param[in] num_policy number of policies
    static void stepper_seed_init(policy_seed_stepper_t *seed, uint32_t id,
                                  uint32_t stateless_rules, rule_dir_t dir,
                                  policy_type_t type, uint8_t af,
                                  std::string pfx, uint32_t num_policy);

private:
    void __init();

};

}    // namespace api_test

#endif    // __TEST_UTILS_POLICY_HPP__
