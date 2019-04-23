//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vpc test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_VPC_HPP__
#define __TEST_UTILS_VPC_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"

namespace api_test {

/// VPC test utility class
class vpc_util {
public:
    /// \brief constructor
    vpc_util(pds_vpc_type_t type, pds_vpc_id_t id, std::string cidr_str);
    vpc_util(pds_vpc_id_t id, std::string cidr_str);
    vpc_util(pds_vpc_id_t id);

    /// \brief destructor
    ~vpc_util();

    // Test parameters
    pds_vpc_id_t id;         ///  VPC ID
    std::string cidr_str;    // VPC CIDR
    pds_vpc_type_t type;     /// VPC type

    /// \brief Create a VPC from VPC object
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create();

    /// \brief Read VPC
    ///
    /// \param[in] compare_spec validation to be done or not
    /// \param[out] info vpc information
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_vpc_info_t *info, bool compare_spec=TRUE);

    /// \brief Update the VPC
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(pds_vpc_spec_t *spec);

    /// \brief Delete a VPC given its key
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del();

    /// \brief Create many VPCs
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(pds_vpc_key_t key, std::string pfxstr,
                                 uint32_t num_vpcs,
                                 pds_vpc_type_t vpc_type = PDS_VPC_TYPE_TENANT);

    /// \brief Read many VPCs
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(pds_vpc_key_t key, uint32_t num_vpcs,
                               sdk::sdk_ret_t expected_res = sdk::SDK_RET_OK);

    /// \brief Delete multiple VPCs
    ///
    /// Delete "num_vpcs" VPCs of type "vpc_type" starting from id
    ///
    /// \param[in] num_vpcs number of VPCs to be deleted
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(pds_vpc_key_t key, uint32_t num_vpcs);
};

}    // namespace api_test

#endif    // __TEST_UTILS_VPC_HPP__
