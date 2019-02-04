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
#include "nic/apollo/include/api/oci_vcn.hpp"
#include "nic/apollo/include/api/oci_subnet.hpp"

namespace api_test {

/// Subnet test utility class
class subnet_util {
public:
    /// \brief Create subnet
    ///
    /// \param vcn_id VCN id
    /// \param subnet_id subnet id
    /// \param pfxstr subnet prefix (cidr) in string form
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t create(oci_vcn_id_t vcn_id, oci_subnet_id_t subnet_id,
                            std::string pfxstr);

    /// \brief Create many subnets
    ///
    /// \param num_subnet number of subnets to create
    /// \param vcn_id VCN id
    /// \param pfxstr subnet prefix (cidr) in string form
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(uint32_t num_subnet, oci_vcn_id_t vcn_id,
                                 std::string pfxstr);
};

} // namespace api_test

#endif // __TEST_UTILS_SUBNET_HPP__
