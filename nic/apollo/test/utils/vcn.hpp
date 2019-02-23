//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the vcn test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_VCN_HPP__
#define __TEST_UTILS_VCN_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/include/api/oci_vcn.hpp"

namespace api_test {

/// VCN test utility class
class vcn_util {
public:
    /// \brief constructor
    vcn_util(oci_vcn_type_t type, oci_vcn_id_t id, std::string cidr_str);

    /// \brief destructor
    ~vcn_util();

    // Test parameters
    oci_vcn_id_t id;         ///  VCN ID
    std::string cidr_str;    // VCN CIDR
    oci_vcn_type_t type;     /// VCN type

    /// \brief Create VCN
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create();

    /// \brief Delete a VCN given its id
    ///
    /// \param[in] vcn_id VCN id
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t del(oci_vcn_id_t vcn_id);

    /// \brief Update a VCN given its id and updated spec
    ///
    /// \param[in] vcn_spec VCN spec
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t update(oci_vcn_spec_t *vcn_spec);

    /// \brief Read a VCN
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read();

    /// \brief Update the VCN
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update();

    /// \brief Delete a VCN given its
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del();

    /// \brief Delete all VCN
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(uint32_t num_vcn, std::string pfxstr,
                                 oci_vcn_type_t vcn_type = OCI_VCN_TYPE_TENANT);

    /// \brief Delete all VCNs configured
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t all_delete(void);

    /// \brief Get all VCNs configured on the NAPLES
    ///
    /// \param[out] count number of VCN objects
    /// \param[out] vcn list of VCN objects
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t all_get(int *count, oci_vcn_spec_t *vcn);
};

}    // namespace api_test

#endif    // __TEST_UTILS_VCN_HPP__
