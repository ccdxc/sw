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
#include "nic/apollo/test/utils/feeder.hpp"
#include "nic/apollo/test/utils/api_base.hpp"

namespace api_test {

// Device test feeder class
class device_feeder : public feeder {
public:
    std::string device_ip_str;    // Device IP
    std::string mac_addr_str;     // Device MAC Address
    std::string gw_ip_str;        // Gateway IP

    // Constructor
    device_feeder() { };
    device_feeder(const device_feeder& feeder) {
        init(feeder.device_ip_str, feeder.mac_addr_str, feeder.gw_ip_str,
             feeder.num_obj);
    }

    // Initialize feeder with the base set of values
    void init(std::string device_ip_str, std::string mac_addr_str,
              std::string gw_ip_str, int num_device = 1);

    // Iterate helper routines
    void iter_next(int width = 1);

    // Build routines
    void spec_build(pds_device_spec_t *spec) const;

    // Compare routines
    bool spec_compare(const pds_device_spec_t *spec) const;
    sdk::sdk_ret_t info_compare(const pds_device_info_t *info) const;
};

// Device test CRUD routines

API_CREATE(device);

inline sdk::sdk_ret_t
read(device_feeder& feeder) {
    sdk_ret_t rv;
    pds_device_info_t info;

    memset(&info, 0, sizeof(pds_device_info_t));
    if ((rv = pds_device_read(&info)) != sdk::SDK_RET_OK)
        return rv;

    return (feeder.info_compare(&info));
}

API_UPDATE(device);

inline sdk::sdk_ret_t
del(device_feeder& feeder) {
    return (pds_device_delete());
}

// Export variables
extern std::string k_device_ip;

// Function prototypes
void sample_device_setup();
void sample_device_setup_validate();
void sample_device_teardown();

}    // namespace api_test

#endif    // __TEST_UTILS_DEVICE_HPP__
