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

#define SUBNET_MANY_CREATE(seed)                                        \
    ASSERT_TRUE(subnet_util::many_create(seed) == sdk::SDK_RET_OK)

#define SUBNET_MANY_READ(seed, expected_res)                            \
    ASSERT_TRUE(subnet_util::many_read(                                 \
                             seed,expected_res) == sdk::SDK_RET_OK)

#define SUBNET_MANY_UPDATE(seed)                                        \
    ASSERT_TRUE(subnet_util::many_update(seed) == sdk::SDK_RET_OK)

#define SUBNET_MANY_DELETE(seed)                                        \
    ASSERT_TRUE(subnet_util::many_delete(seed) == sdk::SDK_RET_OK)

#define SUBNET_SEED_INIT subnet_util::stepper_seed_init

typedef struct subnet_util_stepper_seed_s {
    pds_subnet_key_t key;
    pds_vpc_key_t vpc;
    std::string cidr_str;
    ip_prefix_t pfx;
    std::string vr_ip;
    std::string vr_mac;
    pds_route_table_key_t v4_route_table;    /// Route table id
    pds_route_table_key_t v6_route_table;    /// Route table id
    pds_policy_key_t ing_v4_policy;
    pds_policy_key_t ing_v6_policy;
    pds_policy_key_t egr_v4_policy;
    pds_policy_key_t egr_v6_policy;
    uint32_t num_subnets;
} subnet_util_stepper_seed_t;

/// Subnet test utility class
class subnet_util {
public:
    /// \brief constructor
    subnet_util(pds_subnet_id_t id);
    subnet_util(pds_vpc_id_t vpc_id, pds_subnet_id_t id, std::string cidr_str);
    subnet_util(pds_vpc_id_t vpc_id, pds_subnet_id_t id, std::string cidr_str,
                uint32_t, uint32_t, uint32_t ing_v4_policy = 0, uint32_t ing_v6_policy = 0,
                uint32_t egr_v4_policy = 0, uint32_t egr_v6_policy = 0);
    subnet_util(subnet_util_stepper_seed_t *seed);

    /// \brief destructor
    ~subnet_util();

    /// Test params
    pds_subnet_id_t id;
    pds_vpc_key_t vpc;
    std::string cidr_str;
    ip_prefix_t pfx;
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
    sdk_ret_t update();

    /// \brief Delete subnet
    ///
    /// \param[in] vpc_id VPC id
    /// \param[in] subnet_id subnet id
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del();

    /// \brief Create many subnets
    ///
    /// \param[in] seed subnet seed
    /// \param[in] num_subnet number of subnets to create
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(subnet_util_stepper_seed_t *seed);

    /// \brief Read many subnets
    ///
    /// \param[in] seed subnet seed
    /// \param[in] num_subnet number of subnets to read
    /// \param[in] expected_res expected result after read operation
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(subnet_util_stepper_seed_t *seed,
                               sdk::sdk_ret_t expected_res = sdk::SDK_RET_OK);

    /// \brief Update multiple subnets
    /// Update "num_subnets" subnets starting from id
    ///
    /// \param[in] seed subnet seed
    /// \param[in] num_subnet number of subnets to update
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_update(subnet_util_stepper_seed_t *seed);

    /// \brief Delete multiple subnets
    /// Delete "num_subnets" subnets starting from id
    ///
    /// \param[in] seed subnet seed
    /// \param[in] num_subnet number of subnets to delete
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(subnet_util_stepper_seed_t *seed);

    static sdk_ret_t stepper_seed_init(subnet_util_stepper_seed_t *seed,
                                       pds_subnet_key_t key,
                                       pds_vpc_key_t vpc_key,
                                       std::string subnet_start_addr,
                                       int num_subnets);

private:
    void __init();

};

}    // namespace api_test

#endif    // __TEST_UTILS_SUBNET_HPP__
