//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the switchport test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_SWITCHPORT_HPP__
#define __TEST_UTILS_SWITCHPORT_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/include/api/oci_switchport.hpp"

namespace api_test {

/// SWITCHPORT test utility class
class switchport_util {
public:
    // Test parameters
    std::string switchport_ip_str;    // SwitchPort IP
    std::string mac_addr_str;         // SwitchPort MAC Address
    std::string gateway_ip_str;       // Gateway IP

    /// \brief constructor
    switchport_util();

    /// \brief parameterized constructor
    switchport_util(std::string switchport_ip_str, std::string mac_addr_str,
                    std::string gateway_ip_str);

    /// \brief destructor
    ~switchport_util();

    /// \brief Create SWITCHPORT
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void);

    /// \brief Read SWITCHPORT
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(oci_switchport_info_t *info);

    /// \brief Update SWITCHPORT
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void);

    /// \brief Delete SWITCHPORT
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t del(void);

    /// \brief Validate SWITCHPORT
    ///
    /// validates the test parameters of this object against info
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t validate(oci_switchport_info_t *info);
};

}    // namespace api_test

#endif    // __TEST_UTILS_SWITCHPORT_HPP__
