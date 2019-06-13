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
    void spec_build(pds_device_spec_t *spec);

    // Compare routines
    bool spec_compare(pds_device_spec_t *spec);
    sdk::sdk_ret_t info_compare(pds_device_info_t *info);
};

// Export variables
extern std::string k_device_ip;

// Function prototypes
sdk::sdk_ret_t create(device_feeder& feeder);
sdk::sdk_ret_t read(device_feeder& feeder);
sdk::sdk_ret_t update(device_feeder& feeder);
sdk::sdk_ret_t del(device_feeder& feeder);

void sample_device_setup();
void sample_device_setup_validate();
void sample_device_teardown();

}    // namespace api_test

#endif    // __TEST_UTILS_DEVICE_HPP__
