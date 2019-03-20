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
#include "nic/apollo/api/include/pds_vcn.hpp"

namespace api_test {

/// VCN test utility class
class vcn_util {
public:
    /// \brief constructor
    vcn_util(pds_vcn_type_t type, pds_vcn_id_t id, std::string cidr_str);
    vcn_util(pds_vcn_id_t id, std::string cidr_str);
    vcn_util(pds_vcn_id_t id);

    /// \brief destructor
    ~vcn_util();

    // Test parameters
    pds_vcn_id_t id;         ///  VCN ID
    std::string cidr_str;    // VCN CIDR
    pds_vcn_type_t type;     /// VCN type

    /// \brief Create a VCN from VCN object
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create();

    /// \brief Read VCN
    ///
    /// \param[in] compare_spec validation to be done or not
    /// \param[out] info vcn information
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_vcn_info_t *info, bool compare_spec=TRUE);

    /// \brief Update the VCN
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(pds_vcn_spec_t *spec);

    /// \brief Delete a VCN given its key
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del();

    /// \brief Create many VCNs
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(pds_vcn_key_t key, std::string pfxstr,
                                 uint32_t num_vcns,
                                 pds_vcn_type_t vcn_type = PDS_VCN_TYPE_TENANT);

    /// \brief Read many VCNs
    ///
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(pds_vcn_key_t key, uint32_t num_vcns,
                               sdk::sdk_ret_t expected_res = sdk::SDK_RET_OK);

    /// \brief Delete multiple VCNs
    ///
    /// Delete "num_vcns" VCNs of type "vcn_type" starting from id
    ///
    /// \param[in] num_vcns number of VCNs to be deleted
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(pds_vcn_key_t key, uint32_t num_vcns);
};

}    // namespace api_test

#endif    // __TEST_UTILS_VCN_HPP__
