//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vnic test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_VNIC_HPP__
#define __TEST_UTILS_VNIC_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"

namespace api_test {

#define VNIC_SEED_INIT vnic_util::vnic_stepper_seed_init

#define VNIC_MANY_CREATE(seed)                                        \
    ASSERT_TRUE(vnic_util::many_create(seed) == sdk::SDK_RET_OK)

#define VNIC_MANY_READ(seed, exp_result)                              \
    ASSERT_TRUE(vnic_util::many_read(seed,                            \
                                     exp_result) == sdk::SDK_RET_OK)

#define VNIC_MANY_UPDATE(seed)                                        \
    ASSERT_TRUE(vnic_util::many_update(seed) == sdk::SDK_RET_OK)

#define VNIC_MANY_DELETE(seed)                                        \
    ASSERT_TRUE(vnic_util::many_delete(seed) == sdk::SDK_RET_OK)

// VNIC object seed used as base seed in many_* operations
typedef struct vnic_stepper_seed_s {
    uint32_t id;
    uint32_t num_vnics;
    pds_encap_t vnic_encap;
    pds_encap_t fabric_encap;
    uint64_t mac_u64;
    bool src_dst_check;
} vnic_stepper_seed_t;

/// VNIC test utility class
class vnic_util {
public:
    pds_vpc_id_t vpc_id;           ///< VPC id
    pds_subnet_id_t sub_id;        ///< Subnet id
    pds_vnic_id_t vnic_id;         ///< VNIC id
    pds_encap_t vnic_encap;        ///< VNIC encap
    pds_encap_t fabric_encap;      ///< VNIC fabric encap
    std::string vnic_mac;          ///< VNIC mac
    mac_addr_t mac_addr;
    uint64_t mac_u64;
    pds_rsc_pool_id_t rsc_pool_id; ///< Resource pool id
    bool src_dst_check;            ///< Source destination check

    /// \brief Constructor
    vnic_util();

    /// \brief Parameterized constructor
    vnic_util(uint32_t vnic_id, pds_encap_t vnic_encap, pds_encap_t fabric_encap,
              uint64_t mac_u64, bool src_dst_check = false);

    /// \brief Parameterized constructor
    vnic_util(pds_vpc_id_t vpc_id, pds_vnic_id_t vnic_id,
              pds_subnet_id_t sub_id = -1, std::string vnic_mac = "",
              bool src_dst_check = false);

    /// \brief Destructor
    ~vnic_util();

    /// \brief Create VNIC
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void) const;

    /// \brief Read VNIC info
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_vnic_info_t *info) const;

    /// \brief Update VNIC configuration
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void) const;

    /// \brief Delete VNIC
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void) const;

    /// \brief Create many VNICs for the given subnet and VPC
    ///
    /// \param[in] seed vnic seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(vnic_stepper_seed_t *seed);

    /// \brief Read many VNICs for the given subnet and VPC
    ///
    /// \param[in] seed vnic seed
    /// \param[in] exp_result expected result for read operation
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(vnic_stepper_seed_t *seed,
                               sdk::sdk_ret_t exp_result = sdk::SDK_RET_OK);

    /// \brief Update many VNICs for the given subnet and VPC
    ///
    /// \param[in] seed vnic seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_update(vnic_stepper_seed_t *seed);

    /// \brief Delete many VNICs for the given subnet and VPC
    ///
    /// \param[in] seed vnic seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(vnic_stepper_seed_t *seed);

    /// \brief Initialize the seed for vnic
    ///
    /// \param[in] seed_base vnic seed base
    /// \param[in] num_vnics number of vnics
    /// \param[in] seed_mac mac address
    /// \param[in] vnic_encap_type vnic encap type
    /// \param[in] fabric_encap_type fabric encap type
    /// \param[in] src_dst_check source & destination check
    /// \param[out] seed vnic seed
    static void vnic_stepper_seed_init(uint32_t seed_base, uint32_t num_vnics,
                                       uint64_t seed_mac,
                                       pds_encap_type_t vnic_encap_type,
                                       pds_encap_type_t fabric_encap_type,
                                       bool src_dst_check,
                                       vnic_stepper_seed_t *seed);
};

} // namespace api_test

#endif // __TEST_UTILS_VNIC_HPP__
