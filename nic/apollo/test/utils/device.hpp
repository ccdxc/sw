//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the device test utility routines
///
//----------------------------------------------------------------------------

#ifndef __TEST_UTILS_DEVICE_HPP__
#define __TEST_UTILS_DEVICE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/include/api/pds_device.hpp"

namespace api_test {

/// Device test utility class
class device_util {
public:
    // Test parameters
    std::string device_ip_str;    // Device IP
    std::string mac_addr_str;     // Device MAC Address
    std::string gateway_ip_str;   // Gateway IP

    /// \brief constructor
    device_util();

    /// \brief parameterized constructor
    device_util(std::string device_ip_str, std::string mac_addr_str,
                    std::string gateway_ip_str);

    /// \brief destructor
    ~device_util();

    /// \brief Create device
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void);

    /// \brief Read device
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_device_info_t *info);

    /// \brief Update device
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void);

    /// \brief Delete device
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t del(void);

    /// \brief Validate device
    ///
    /// validates the test parameters of this object against info
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t validate(pds_device_info_t *info);
};

}    // namespace api_test

#endif    // __TEST_UTILS_DEVICE_HPP__
