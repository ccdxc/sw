//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the tep test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_TEP_HPP__
#define __TEST_UTILS_TEP_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"

namespace api_test {

#define TEP_SEED_INIT tep_util::tep_stepper_seed_init

#define TEP_CREATE(obj)                                              \
    ASSERT_TRUE(obj.create() == sdk::SDK_RET_OK)

#define TEP_READ(obj, info)                                          \
    ASSERT_TRUE(obj.read(info) == sdk::SDK_RET_OK)

#define TEP_READ_FAIL(obj, info, exp_result)                         \
    ASSERT_TRUE(obj.read(info,                                       \
                         exp_result) == sdk::SDK_RET_OK)

#define TEP_UPDATE(obj)                                              \
    ASSERT_TRUE(obj.update() == sdk::SDK_RET_OK)

#define TEP_DELETE(obj)                                              \
    ASSERT_TRUE(obj.del() == sdk::SDK_RET_OK)

#define TEP_MANY_CREATE(seed)                                        \
    ASSERT_TRUE(tep_util::many_create(seed) == sdk::SDK_RET_OK)

#define TEP_MANY_READ(seed)                                          \
    ASSERT_TRUE(tep_util::many_read(seed) == sdk::SDK_RET_OK)

#define TEP_MANY_READ_FAIL(seed, exp_result)                         \
    ASSERT_TRUE(tep_util::many_read(seed,                            \
                                    exp_result) == sdk::SDK_RET_OK)

#define TEP_MANY_UPDATE(seed)                                        \
    ASSERT_TRUE(tep_util::many_update(seed) == sdk::SDK_RET_OK)

#define TEP_MANY_DELETE(seed)                                        \
    ASSERT_TRUE(tep_util::many_delete(seed) == sdk::SDK_RET_OK)

// TEP object seed used as base seed in many_* operations
typedef struct tep_stepper_seed_s {
    ip_addr_t ip_addr;
    pds_tep_type_t type;
    pds_encap_t encap;
    bool nat;
    uint32_t num_tep;
} tep_stepper_seed_t;

/// TEP test utility class
class tep_util {
public:
    // Test parameters
    ip_addr_t ip_addr;            ///< TEP IP
    pds_tep_type_t type;          ///< TEP type
    pds_encap_t encap;            ///< TEP encap
    bool nat;                     ///< NAT state

    /// \brief parameterized constructor
    tep_util(std::string ip_str, pds_tep_type_t type,
             pds_encap_t encap, bool nat=FALSE);

    /// \brief parameterized constructor
    tep_util(ip_addr_t ip_addr, pds_tep_type_t type,
             pds_encap_t encap, bool nat=FALSE);

    /// \brief destructor
    ~tep_util();

    /// \brief Create TEP
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void);

    /// \brief Read TEP
    ///
    /// \param[in] compare_spec validation to be done or not
    /// \param[out] info tep information
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_tep_info_t *info, bool compare_spec=TRUE);

    /// \brief Update TEP
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update();

    /// \brief Delete TEP
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del();

    /// \brief Create multiple TEPs
    ///
    /// \param[in] seed TEP seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(tep_stepper_seed_t *seed);

    /// \brief Read & validate multiple TEPs
    ///
    /// \param[in] seed TEP seed
    /// \param[in] exp_result expected result on read & validate
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(tep_stepper_seed_t *seed,
                               sdk::sdk_ret_t exp_result = sdk::SDK_RET_OK);

    /// \brief Update multiple TEPs
    ///
    /// \param[in] seed TEP seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_update(tep_stepper_seed_t *seed);

    /// \brief Delete multiple TEPs
    /// Delete "num_tep" TEPs of type "tep_type" with IPs in range
    /// ip_str...ip_str+num_tep
    ///
    /// \param[in] seed TEP seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(tep_stepper_seed_t *seed);

    /// \brief Initialize the seed for TEP
    ///
    /// \param[in] num_tep number of TEPs to be operated
    /// \param[in] ip_str starting IP address
    /// \param[in] type TEP type
    /// \param[in] encap TEP encap
    /// \param[in] nat NAT state
    /// \param[out] seed TEP seed
    static void tep_stepper_seed_init(uint32_t num_tep, std::string ip_str,
                                      pds_tep_type_t type, pds_encap_t encap,
                                      bool nat, tep_stepper_seed_t *seed);
};

}    // namespace api_test

#endif    // __TEST_UTILS_TEP_HPP__
