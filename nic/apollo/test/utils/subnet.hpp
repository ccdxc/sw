//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the subnet test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_SUBNET_HPP__
#define __TEST_UTILS_SUBNET_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"

namespace api_test {

/// Subnet test utility class
class subnet_util {
public:
    /// \brief constructor
    subnet_util(pds_subnet_id_t id);
    subnet_util(pds_vpc_id_t vpc_id, pds_subnet_id_t id, std::string cidr_str);
    subnet_util(pds_vpc_id_t vpc_id, pds_subnet_id_t id, std::string cidr_str,
                uint32_t, uint32_t, uint32_t ing_v4_policy = 0, uint32_t ing_v6_policy = 0,
                uint32_t egr_v4_policy = 0, uint32_t egr_v6_policy = 0);

    /// \brief destructor
    ~subnet_util();

    /// Test params
    pds_subnet_id_t id;
    pds_vpc_key_t vpc;
    std::string cidr_str;
    std::string vr_ip;
    std::string vr_mac;
    pds_route_table_key_t v4_route_table;    /// Route table id
    pds_route_table_key_t v6_route_table;    /// Route table id
    pds_policy_key_t ing_v4_policy;
    pds_policy_key_t ing_v6_policy;
    pds_policy_key_t egr_v4_policy;
    pds_policy_key_t egr_v6_policy;

    /// \brief Create subnet
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create();

    /// \brief Read subnet
    ///
    /// \param[in] compare_spec validation to be done or not
    /// \param[out] info subnet information
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_subnet_info_t *info, bool compare_spec=TRUE);

    /// \brief Update the subnet
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(pds_subnet_spec_t *spec);

    /// \brief Delete subnet
    ///
    /// \param[in] vpc_id VPC id
    /// \param[in] subnet_id subnet id
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del();

    /// \brief Create many subnets
    ///
    /// \param[in] num_subnet number of subnets to create
    /// \param[in] vpc_id VPC id
    /// \param[in] pfxstr subnet prefix (cidr) in string form
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(pds_subnet_key_t subnet, pds_vpc_key_t vpc_key,
                                 std::string pfxstr, uint32_t num_subnet);

    /// \brief Read many subnets
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(pds_subnet_key_t key, uint32_t num_subnets,
                               sdk::sdk_ret_t expected_res = sdk::SDK_RET_OK);

    /// \brief Delete multiple subnets
    /// Delete "num_subnets" subnets starting from id
    ///
    /// \param[in] num_subnets number of VPCs to be deleted
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(pds_subnet_key_t key, uint32_t num_subnets);

private:
    void __init();

};

}    // namespace api_test

#endif    // __TEST_UTILS_SUBNET_HPP__
