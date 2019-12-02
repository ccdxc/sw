//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/device.hpp"
#include "nic/apollo/test/utils/utils.hpp"

namespace api_test {

//----------------------------------------------------------------------------
// Device feeder class routines
//----------------------------------------------------------------------------

void
device_feeder::init(std::string device_ip_str, std::string mac_addr_str,
                    std::string gw_ip_str, int num_device) {
    this->device_ip_str = device_ip_str;
    this->mac_addr_str = mac_addr_str;
    this->gw_ip_str = gw_ip_str;
    num_obj = num_device;
}

void
device_feeder::spec_build(pds_device_spec_t *spec) const {
    ip_prefix_t device_ip_pfx, gw_ip_pfx;

    memset(spec, 0, sizeof(pds_device_spec_t));
    
    extract_ip_pfx(device_ip_str.c_str(), &device_ip_pfx);
    if (device_ip_pfx.addr.af == IP_AF_IPV6) {
        spec->device_ip_addr.af = IP_AF_IPV6;
        memcpy(spec->device_ip_addr.addr.v6_addr.addr8,
               device_ip_pfx.addr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
    } else {
        spec->device_ip_addr.af = IP_AF_IPV4;
        spec->device_ip_addr.addr.v4_addr = device_ip_pfx.addr.addr.v4_addr;
    }

    extract_ip_pfx(gw_ip_str.c_str(), &gw_ip_pfx);
    if (gw_ip_pfx.addr.af == IP_AF_IPV6) {
        spec->gateway_ip_addr.af = IP_AF_IPV6;
        memcpy(spec->gateway_ip_addr.addr.v6_addr.addr8,
               gw_ip_pfx.addr.addr.v6_addr.addr8, IP6_ADDR8_LEN);
    } else {
        spec->gateway_ip_addr.af = IP_AF_IPV4;
        spec->gateway_ip_addr.addr.v4_addr = gw_ip_pfx.addr.addr.v4_addr;
    }

    mac_str_to_addr((char *)mac_addr_str.c_str(), spec->device_mac_addr);

    if (apulu()) {
        spec->dev_oper_mode = PDS_DEV_OPER_MODE_HOST;
    } else {
        spec->dev_oper_mode = PDS_DEV_OPER_MODE_BITW;
    }
}

bool
device_feeder::spec_compare(const pds_device_spec_t *spec) const {
    ip_prefix_t device_ip_pfx, gw_ip_pfx;
    mac_addr_t  device_mac_addr;

    extract_ip_pfx(device_ip_str.c_str(), &device_ip_pfx);
    extract_ip_pfx(gw_ip_str.c_str(), &gw_ip_pfx);

    if (!IPADDR_EQ(&device_ip_pfx.addr, &spec->device_ip_addr)) {
        return false;
    }
    mac_str_to_addr((char *)mac_addr_str.c_str(), device_mac_addr);
    if (memcmp(device_mac_addr, spec->device_mac_addr, ETH_ADDR_LEN)) {
        return false;
    }
    if (!IPADDR_EQ(&gw_ip_pfx.addr, &spec->gateway_ip_addr)) {
        return false;
    }
    return true;
}

//----------------------------------------------------------------------------
// Misc routines
//----------------------------------------------------------------------------

// do not modify these sample values as rest of system is sync with these
std::string k_device_ip("91.0.0.1");
static device_feeder k_device_feeder;

void sample_device_setup(pds_batch_ctxt_t bctxt) {
    // setup and teardown parameters should be in sync
    k_device_feeder.init(k_device_ip, "00:00:01:02:0a:0b", "90.0.0.2");
    many_create(bctxt, k_device_feeder);
}

void sample_device_setup_validate() {
    k_device_feeder.init(k_device_ip, "00:00:01:02:0a:0b", "90.0.0.2");
    many_read(k_device_feeder);
}

void sample_device_teardown(pds_batch_ctxt_t bctxt) {
    // this feeder base values doesn't matter in case of deletes
    k_device_feeder.init(k_device_ip, "00:00:01:02:0a:0b", "90.0.0.2");
    many_delete(bctxt, k_device_feeder);
}

}    // namespace api_test
