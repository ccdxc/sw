//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the device test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/device.hpp"

namespace api_test {

device_util::device_util() {
    this->device_ip_str = "0.0.0.0";
    this->mac_addr_str = "00:00:00:00:00:00";
    this->gateway_ip_str = "0.0.0.0";
}

device_util::device_util(std::string device_ip_str,
                         std::string mac_addr_str,
                         std::string gateway_ip_str) {
    this->device_ip_str = device_ip_str;
    this->mac_addr_str = mac_addr_str;
    this->gateway_ip_str = gateway_ip_str;
}

device_util::~device_util() {}

sdk::sdk_ret_t
device_util::create() {
    pds_device_spec_t spec;
    ip_prefix_t device_ip_pfx, gateway_ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)this->device_ip_str.c_str(),
                           &device_ip_pfx) == 0);
    SDK_ASSERT(str2ipv4pfx((char *)this->gateway_ip_str.c_str(),
                           &gateway_ip_pfx) == 0);
    memset(&spec, 0, sizeof(pds_device_spec_t));
    spec.device_ip_addr = device_ip_pfx.addr.addr.v4_addr;
    spec.gateway_ip_addr = gateway_ip_pfx.addr.addr.v4_addr;
    mac_str_to_addr((char *)this->mac_addr_str.c_str(),
                    spec.device_mac_addr);
    return (pds_device_create(&spec));
}

sdk::sdk_ret_t
device_util::read(pds_device_info_t *info,  bool compare_spec) {
    sdk_ret_t rv;

    memset(info, 0, sizeof(pds_device_info_t));
    rv = pds_device_read(info);
    if (rv != SDK_RET_OK) {
        return rv;
    }

    if (compare_spec) {
        if (this->device_ip_str.compare(ipv4addr2str(info->spec.device_ip_addr)) != 0) {
            return SDK_RET_ERR;
        }

        if (this->mac_addr_str.compare(macaddr2str(info->spec.device_mac_addr)) !=0) {
            return SDK_RET_ERR;
        }

        if (this->gateway_ip_str.compare(ipv4addr2str(info->spec.gateway_ip_addr)) != 0) {
            return SDK_RET_ERR;
        }
    }
    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
device_util::del() {
    return (pds_device_delete());
}

}    // namespace api_test
