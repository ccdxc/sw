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
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"

namespace api_test {

/// Policy test utility class
class policy_util {
public:
    /// \brief constructor
    policy_util(pds_policy_id_t id, uint32_t, rule_t *,
                rule_dir_t dir = RULE_DIR_INGRESS,
                uint8_t af = IP_AF_IPV4,
                policy_type_t type = POLICY_TYPE_FIREWALL);

    /// \brief destructor
    ~policy_util();

    /// Test params
    pds_policy_id_t id;
    uint32_t num_rules;
    rule_t *rules;
    rule_dir_t direction;
    policy_type_t type;
    uint8_t af;

    /// \brief Create policy
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create();

    /// \brief Read policy
    ///
    /// \param[in] compare_spec validation to be done or not
    /// \param[out] info policy information
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_policy_info_t *info, bool compare_spec=TRUE);

    /// \brief Update the policy
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(pds_policy_spec_t *spec);

    /// \brief Delete policy
    ///
    /// \param[in] vpc_id VPC id
    /// \param[in] policy_id policy id
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del();

    /// \brief Create many policies
    ///
    /// \param[in] num_policy number of policies to create
    /// \param[in] vpc_id VPC id
    /// \param[in] pfxstr policy prefix (cidr) in string form
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(pds_policy_key_t policy,
                                 uint32_t num_policy);

    /// \brief Read many policies
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(pds_policy_key_t key, uint32_t num_policy,
                               sdk::sdk_ret_t expected_res = sdk::SDK_RET_OK);

    /// \brief Delete multiple policies
    /// Delete "num_policy" policy starting from id
    ///
    /// \param[in] num_policy number of VPCs to be deleted
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(pds_policy_key_t key, uint32_t num_policy);

private:
    void __init();

};

}    // namespace api_test

#endif    // __TEST_UTILS_POLICY_HPP__
