//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_DEVICE_HPP__
#define __TEST_API_UTILS_DEVICE_HPP__

#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

// Device test feeder class
class device_feeder : public feeder {
public:
    // TODO: use spec_t instead of free form strings
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
    void iter_next(int width = 1) { cur_iter_pos++; }

    bool read_unsupported(void) const {
        if (apulu()) { return false; }
        return (::capri_mock_mode() ? true : false);
    }

    // Build routines
    void spec_build(pds_device_spec_t *spec) const;

    // Compare routines
    bool spec_compare(const pds_device_spec_t *spec) const;
};

// Dump prototypes
inline std::ostream&
operator<<(std::ostream& os, const pds_device_spec_t *spec) {
    os << " ip: " << ipaddr2str(&spec->device_ip_addr)
       << " mac: " << macaddr2str(spec->device_mac_addr)
       << " gw ip: " << ipaddr2str(&spec->gateway_ip_addr);
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_device_info_t *obj) {
    os << "Device info =>" << &obj->spec << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const device_feeder& obj) {
    os << "Device feeder =>"
        << " device IP: " << obj.device_ip_str
        << " mac addr: " << obj.mac_addr_str
        << " gw IP: " << obj.gw_ip_str << " ";
    return os;
}

// CRUD prototypes
API_CREATE(device);
API_READ_SINGLETON(device);
API_UPDATE(device);
API_DELETE_SINGLETON(device);

// Export variables
extern std::string k_device_ip;

// Function prototypes
void sample_device_setup(pds_batch_ctxt_t bctxt);
void sample_device_setup_validate();
void sample_device_teardown(pds_batch_ctxt_t bctxt);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_DEVICE_HPP__
