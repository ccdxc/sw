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

#define VPC_MANY_CREATE(seed)                                        \
    ASSERT_TRUE(vpc_util::many_create(seed) == sdk::SDK_RET_OK)

#define VPC_MANY_READ(seed, expected_res)                            \
    ASSERT_TRUE(vpc_util::many_read(                                 \
                             seed,expected_res) == sdk::SDK_RET_OK)

#define VPC_MANY_UPDATE(seed)                                        \
    ASSERT_TRUE(vpc_util::many_update(seed) == sdk::SDK_RET_OK)

#define VPC_MANY_DELETE(seed)                                        \
    ASSERT_TRUE(vpc_util::many_delete(seed) == sdk::SDK_RET_OK)

#define VPC_SEED_INIT vpc_util::stepper_seed_init

typedef struct vpc_stepper_seed_s {
    pds_vpc_key_t key;
    pds_vpc_type_t type;
    ip_prefix_t pfx;
    uint32_t num_vpcs;
} vpc_stepper_seed_t;

/// VPC test utility class
class vpc_util {
public:
    /// \brief constructor
    vpc_util(vpc_stepper_seed_t *seed);
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
    sdk_ret_t update();

    /// \brief Delete a VPC given its key
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del();

    /// \brief Create many VPCs
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(vpc_stepper_seed_t *seed);

    /// \brief Read many VPCs
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(vpc_stepper_seed_t *seed,
                               sdk::sdk_ret_t expected_res = sdk::SDK_RET_OK);

    /// \brief Update many VPCs
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_update(vpc_stepper_seed_t *seed);

    /// \brief Delete multiple VPCs
    ///
    /// Delete "num_vpcs" VPCs of type "vpc_type" starting from id
    ///
    /// \param[in] seed seed for the vpc
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(vpc_stepper_seed_t *seed);

    static void stepper_seed_init(vpc_stepper_seed_t *seed,
                                  pds_vpc_key_t key,
                                  pds_vpc_type_t type,
                                  std::string start_pfx,
                                  uint32_t num_vpcs);
};

}    // namespace api_test

#endif    // __TEST_UTILS_VPC_HPP__
