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
#include "nic/apollo/api/include/pds_device.hpp"

namespace api_test {

#define DEVICE_CREATE(seed)                                 \
    device_util obj_c(seed);                                \
    ASSERT_TRUE(obj_c.create() == sdk::SDK_RET_OK)

#define DEVICE_READ(seed, expected_res)                     \
    device_util obj_r(seed);                                \
    pds_device_info_t info;                                 \
    ASSERT_TRUE(obj_r.read(&info, expected_res)             \
                == sdk::SDK_RET_OK)

#define DEVICE_UPDATE(seed)                                 \
    device_util obj_u(seed);                                \
    ASSERT_TRUE(obj_u.update() == sdk::SDK_RET_OK)

#define DEVICE_DELETE(seed)                                \
    device_util obj_d(seed);                               \
    ASSERT_TRUE(obj_d.del() == sdk::SDK_RET_OK)

#define DEVICE_SEED_INIT device_util::stepper_seed_init

typedef struct device_stepper_seed_s {
    std::string device_ip_str;
    std::string mac_addr_str;
    std::string gateway_ip_str;
} device_stepper_seed_t;

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

    /// \brief parameterized constructor
    device_util(device_stepper_seed_t *seed);

    /// \brief destructor
    ~device_util();

    /// \brief Create device
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t create(void) const;

    /// \brief Read device
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t read(pds_device_info_t *info,
                   sdk::sdk_ret_t expected_res = sdk::SDK_RET_OK) const;

    /// \brief Update device
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t update(void) const;

    /// \brief Delete device
    ///
    /// \returns #SDK_RET_OK on success, failure status code on error
    sdk_ret_t del(void) const;

    // \brief Create many devices
    ///
    /// \param[in] seed    device seed
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_create(device_stepper_seed_t *seed);

    /// \brief Read many devices
    ///
    /// \param[in] seed device seed
    /// \param[in] expected_res expected result after read operation
    /// \return #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_read(device_stepper_seed_t *seed,
                               sdk::sdk_ret_t expected_res = sdk::SDK_RET_OK);

    /// \brief Update multiple devices
    ///
    /// \param[in] seed device seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_update(device_stepper_seed_t *seed);

    /// \brief Delete multiple devices
    ///
    /// \param[in] seed device seed
    /// \returns #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t many_delete(device_stepper_seed_t *seed);

    /// \brief Initialize the seed for device
    ///
    /// \param[out] seed device seed
    /// \param[in] device_ip_str device ip address
    /// \param[in] mac_addr_str device mac address
    /// \param[in] gateway_ip_str gateway ip address
    static void stepper_seed_init(device_stepper_seed_t *seed,
                                  std::string device_ip_str,
                                  std::string mac_addr_str,
                                  std::string gateway_ip_str);

    /// \brief Indicates whether device is stateful
    ///
    /// \returns FALSE for device which is stateless
    static bool is_stateful(void) { return true; }

private:
    void __init();
};

}    // namespace api_test

#endif    // __TEST_UTILS_DEVICE_HPP__
