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

/// TEP test utility class
class tep_util {
public:
    // Test parameters
    std::string ip_str;           ///< TEP IP
    pds_tep_encap_type_t type;    ///< TEP type

    /// \brief default constructor
    tep_util();

    /// \brief parameterized constructor
    tep_util(std::string ip_str);

    /// \brief parameterized constructor
    tep_util(std::string ip_str, pds_tep_encap_type_t type);

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
    /// Create "num_tep" TEPs of type "tep_type" with IPs in range
    /// ip_str...ip_str+num_tep
    ///
    /// \param[in] num_tep number of TEPs to be created
    /// \param[in] ip_str starting IP address
    /// \param[in] type TEP encap type
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(uint32_t num_tep, std::string ip_str,
                                 pds_tep_encap_type_t type =
                                     PDS_TEP_ENCAP_TYPE_VNIC);

    /// \brief Read & validate multiple TEPs
    /// Read "num_tep" TEPs of type "tep_type" with IPs in range
    /// ip_str...ip_str+num_tep and validate
    ///
    /// \param[in] expected_result expected result on read & validate
    /// \param[in] num_tep number of TEPs to be created
    /// \param[in] ip_str starting IP address
    /// \param[in] type TEP encap type
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(uint32_t num_tep, std::string ip_str,
                               pds_tep_encap_type_t type =
                                   PDS_TEP_ENCAP_TYPE_VNIC,
                               sdk::sdk_ret_t expected_res = sdk::SDK_RET_OK);

    /// \brief Delete multiple TEPs
    /// Delete "num_tep" TEPs of type "tep_type" with IPs in range
    /// ip_str...ip_str+num_tep
    ///
    /// \param[in] num_tep number of TEPs to be deleted
    /// \param[in] ip_str starting IP address
    /// \param[in] type TEP encap type
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(uint32_t num_tep, std::string ip_str,
                                 pds_tep_encap_type_t type =
                                     PDS_TEP_ENCAP_TYPE_VNIC);
};

}    // namespace api_test

#endif    // __TEST_UTILS_TEP_HPP__
