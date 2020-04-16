//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#ifndef __TEST_API_UTILS_DEVICE_HPP__
#define __TEST_API_UTILS_DEVICE_HPP__

#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/test/api/utils/batch.hpp"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/test/api/utils/feeder.hpp"

namespace test {
namespace api {

enum device_attrs {
    DEVICE_ATTR_DEVICE_IP           =  bit(0),
    DEVICE_ATTR_DEVICE_MAC          =  bit(1),
    DEVICE_ATTR_GATEWAY_IP          =  bit(2),
    DEVICE_ATTR_BRIDGING_EN         =  bit(3),
    DEVICE_ATTR_LEARNING_EN         =  bit(4),
    DEVICE_ATTR_LEARN_AGE_TIME_OUT  =  bit(5),
    DEVICE_ATTR_OVERLAY_ROUTING_EN  =  bit(6),
    DEVICE_ATTR_DEVICE_PROFILE      =  bit(7),
    DEVICE_ATTR_MEMORY_PROFILE      =  bit(8),
    DEVICE_ATTR_DEV_OPER_MODE       =  bit(9),
};

// Device test feeder class
class device_feeder : public feeder {
public:
    pds_device_spec_t spec;

    // Constructor
    device_feeder() { };
    device_feeder(const device_feeder& feeder);

    // Initialize feeder with the base set of values
    void init(std::string device_ip_str, std::string mac_addr_str,
              std::string gw_ip_str, int num_device = 1);

    // Iterate helper routines
    void iter_next(int width = 1) { cur_iter_pos++; }

    bool read_unsupported(void) const {
        if (apulu()) { return false; }
        return (::asic_mock_mode() ? true : false);
    }

    // Build routines
    void spec_build(pds_device_spec_t *spec) const;

    // Compare routines
    bool spec_compare(const pds_device_spec_t *spec) const;
    bool status_compare(const pds_device_status_t *status1,
                        const pds_device_status_t *status2) const;
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
        << " device IP: " << ipaddr2str(&obj.spec.device_ip_addr)
        << " mac addr: " << macaddr2str(obj.spec.device_mac_addr)
        << " gw IP: " << ipaddr2str(&obj.spec.gateway_ip_addr) << " ";
    return os;
}

// CRUD prototypes
API_CREATE(device);
API_READ_SINGLETON(device);
API_UPDATE(device);
API_DELETE_SINGLETON(device);

// Export variables
extern std::string k_device_ip;

// Device crud helper prototypes
void device_create(device_feeder& feeder);
void device_read(device_feeder& feeder, sdk_ret_t exp_result = SDK_RET_OK);
void device_update(device_feeder& feeder, pds_device_spec_t *spec,
                   uint64_t chg_bmap, sdk_ret_t exp_result = SDK_RET_OK);
void device_delete(device_feeder& feeder);

// Function prototypes
void sample_device_setup(pds_batch_ctxt_t bctxt);
void sample_device_setup_validate();
void sample_device_teardown(pds_batch_ctxt_t bctxt);

}    // namespace api
}    // namespace test

#endif    // __TEST_API_UTILS_DEVICE_HPP__
