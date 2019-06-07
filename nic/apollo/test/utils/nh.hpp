//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the nexthop test utility routines
///
//----------------------------------------------------------------------------
#ifndef __TEST_UTILS_NH_HPP__
#define __TEST_UTILS_NH_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"

namespace api_test {

#define NH_SEED_INIT nh_util::stepper_seed_init

// NH object seed used as base seed in many_* operations
typedef struct nh_stepper_seed_s {
    pds_nexthop_id_t id;
    uint32_t num_nh;
    pds_nh_type_t type;
    ip_addr_t ip;
    uint64_t mac;
    uint16_t vlan;
    pds_vpc_id_t vpc_id;
} __PACK__ nh_stepper_seed_t;

/// NH test utility class
class nh_util {
public:
    // Test parameters
    pds_nexthop_id_t id;        ///< NH ID
    ip_addr_t ip;               ///< NH IP address
    uint64_t mac;               ///< NH MAC address
    pds_nh_type_t type;         ///< NH type
    uint16_t vlan;              ///< NH vlan
    pds_vpc_key_t vpc;          ///< NH vpc

    /// \brief parameterized constructor
    nh_util(pds_nexthop_id_t id, pds_nh_type_t type, ip_addr_t ip,
            pds_vpc_key_t vpc, uint16_t vlan, uint64_t mac);

    /// \brief destructor
    ~nh_util();

    /// \brief Create NH
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void) const;

    /// \brief Read NH
    ///
    /// \param[out] info nexthop information
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_nexthop_info_t *info) const;

    /// \brief Update NH
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void) const;

    /// \brief Delete NH
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void) const;

    /// \brief Create multiple NHs
    ///
    /// \param[in] seed NH seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(nh_stepper_seed_t *seed);

    /// \brief Read & validate multiple NHs
    ///
    /// \param[in] seed NH seed
    /// \param[in] exp_result expected result on read & validate
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(nh_stepper_seed_t *seed,
                               sdk::sdk_ret_t exp_result = sdk::SDK_RET_OK);

    /// \brief Update multiple NHs
    ///
    /// \param[in] seed NH seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_update(nh_stepper_seed_t *seed);

    /// \brief Delete multiple NHs
    ///
    /// \param[in] seed NH seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(nh_stepper_seed_t *seed);

    /// \brief Initialize the seed for NH
    ///
    /// \param[out] seed NH seed
    /// \param[in] seed_base base nh ID
    /// \param[in] ip_str base nh IP address
    /// \param[in] num_nh number of NHs to be operated
    /// \param[in] type NH type
    /// \param[in] mac base mac address
    static void stepper_seed_init(nh_stepper_seed_t *seed,
                                  std::string ip_str="0.0.0.0",
                                  uint64_t mac=0x0E0D0A0B0200,
                                  uint32_t num_nh=PDS_MAX_NEXTHOP,
                                  pds_nexthop_id_t seed_base=1,
                                  pds_nh_type_t type=PDS_NH_TYPE_IP,
                                  uint16_t vlan=1, pds_vpc_id_t vpc_id=1);

    /// \brief Indicates whether NH is stateful
    ///
    /// \returns TRUE for NH which is stateful
    static bool is_stateful(void) { return TRUE; }
};

}    // namespace api_test

#endif    // __TEST_UTILS_NH_HPP__
