//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vpc peer test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_VPC_PEER_HPP__
#define __TEST_UTILS_VPC_PEER_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"

namespace api_test {

#define VPC_PEER_SEED_INIT vpc_peer_util::stepper_seed_init

typedef struct vpc_peer_stepper_seed_s {
    pds_vpc_peer_key_t key;
    pds_vpc_key_t vpc1;
    pds_vpc_key_t vpc2;
    uint32_t num_vpc_peers;
} vpc_peer_stepper_seed_t;

/// VPC test utility class
class vpc_peer_util {

public:
    /// \brief constructor
    vpc_peer_util(vpc_peer_stepper_seed_t *seed);

    /// \brief destructor
    ~vpc_peer_util();

    // Test parameters
    pds_vpc_peer_key_t key;
    pds_vpc_key_t vpc1;
    pds_vpc_key_t vpc2;

    /// \brief Create a VPC peer from vpc_peer_util object
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void) const;

    /// \brief Read VPC peer
    ///
    /// \param[in] compare_spec validation to be done or not
    /// \param[out] info vpc information
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_vpc_peer_info_t *info) const;

    /// \brief Update VPC peer
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void) const;

    /// \brief Delete a VPC peer given its key
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void) const;

    /// \brief Create many VPC peers
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(vpc_peer_stepper_seed_t *seed);

    /// \brief Read many VPC peers
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(vpc_peer_stepper_seed_t *seed,
                               sdk::sdk_ret_t expected_res = sdk::SDK_RET_OK);

    /// \brief Update many VPC peers
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_update(vpc_peer_stepper_seed_t *seed);

    /// \brief Delete multiple VPC peers
    ///
    /// Delete "num_vpc_peers" VPC peers starting from id
    ///
    /// \param[in] seed seed for the vpc
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(vpc_peer_stepper_seed_t *seed);

    /// \brief Initialize the seed for vpc
    ///
    /// \param[out] seed vpc peer seed
    /// \param[in] key vpc key
    /// \param[in] num_vpc_peers number of vpc peers
    static void stepper_seed_init(vpc_peer_stepper_seed_t *seed,
                                  pds_vpc_peer_key_t key,
                                  pds_vpc_key_t vpc1,
                                  pds_vpc_key_t vpc2,
                                  uint32_t num_vpc_peers);

    /// \brief Indicates whether VPC peer is stateful
    ///
    /// \returns TRUE for VPC peer which is stateful
    static bool is_stateful(void) { return TRUE; }
};

}    // namespace api_test

#endif    // __TEST_UTILS_VPC_PEER_HPP__
