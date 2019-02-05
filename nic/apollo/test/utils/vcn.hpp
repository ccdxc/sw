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
    /// \brief Create VCN
    ///
    /// \param vnc_id VCN id
    /// \param pfxstr VCN prefix in string form
    /// \param vcn_type VCN type
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t create(oci_vcn_id_t vcn_id, std::string pfxstr,
                            oci_vcn_type_t vcn_type = OCI_VCN_TYPE_TENANT);

    /// \brief Create many VCN
    ///
    /// \param num_vcn number of VCN to create
    /// \param pfxstr VCN prefix in string form
    /// \param vcn_type VCN type
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(uint32_t num_vcn, std::string pfxstr,
                                 oci_vcn_type_t vcn_type = OCI_VCN_TYPE_TENANT);
};

} // namespace api_test

#endif // __TEST_UTILS_VCN_HPP__
