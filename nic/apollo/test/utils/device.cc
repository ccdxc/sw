//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the device test utility routines implementation
///
//----------------------------------------------------------------------------

#include <iostream>
#include "nic/apollo/test/utils/utils.hpp"
#include "nic/apollo/test/utils/device.hpp"

namespace api_test {

device_util::device_util() {
    __init();
    this->device_ip_str = "0.0.0.0";
    this->mac_addr_str = "00:00:00:00:00:00";
    this->gateway_ip_str = "0.0.0.0";
}

device_util::device_util(std::string device_ip_str,
                         std::string mac_addr_str,
                         std::string gateway_ip_str) {
    __init();
    this->device_ip_str = device_ip_str;
    this->mac_addr_str = mac_addr_str;
    this->gateway_ip_str = gateway_ip_str;
}

device_util::device_util(device_stepper_seed_t *seed) {
    __init();
    this->device_ip_str = seed->device_ip_str;
    this->mac_addr_str = seed->mac_addr_str;
    this->gateway_ip_str = seed->gateway_ip_str;
}

device_util::~device_util() {}

void
device_util::stepper_seed_init(device_stepper_seed_t *seed,
                               std::string device_ip_str,
                               std::string mac_addr_str,
                               std::string gateway_ip_str) {
    seed->device_ip_str = device_ip_str;
    seed->mac_addr_str = mac_addr_str;
    seed->gateway_ip_str = gateway_ip_str;
}

sdk_ret_t
device_util::create(void) const {
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

sdk_ret_t
device_util::read(pds_device_info_t *info,
                  sdk::sdk_ret_t expected_res) const {
    sdk_ret_t rv;

    memset(info, 0, sizeof(pds_device_info_t));
    rv = pds_device_read(info);
    if (rv != expected_res)
        return rv;

    if (capri_mock_mode())
        return sdk::SDK_RET_OK;

    std::cout << "Device IP : " << ipv4addr2str(info->spec.device_ip_addr)
              << " Device MAC : " << macaddr2str(info->spec.device_mac_addr)
              << " Gateway IP : " << ipv4addr2str(info->spec.gateway_ip_addr)
              << std::endl;

    if (this->device_ip_str.compare(
            ipv4addr2str(info->spec.device_ip_addr)) != 0)
        return SDK_RET_ERR;

    if (this->mac_addr_str.compare(
            macaddr2str(info->spec.device_mac_addr)) !=0)
        return SDK_RET_ERR;

    if (this->gateway_ip_str.compare(
            ipv4addr2str(info->spec.gateway_ip_addr)) != 0)
        return SDK_RET_ERR;

    return sdk::SDK_RET_OK;
}

sdk_ret_t
device_util::update(void) const {
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
    return (pds_device_update(&spec));
}

sdk_ret_t
device_util::del(void) const {
    return (pds_device_delete());
}

void device_util::__init() {
    this->device_ip_str = "0.0.0.0";
    this->mac_addr_str = "00:00:00:00:00:00";
    this->gateway_ip_str = "0.0.0.0";
}

sdk_ret_t
device_util::many_create(device_stepper_seed_t *seed) {
    device_util obj_c(seed);
    return obj_c.create();
}

sdk_ret_t
device_util::many_read(device_stepper_seed_t *seed,
                       sdk::sdk_ret_t expected_res) {
    pds_device_info_t info;
    device_util obj_r(seed);
    return obj_r.read(&info, expected_res);
}

sdk_ret_t
device_util::many_update(device_stepper_seed_t *seed) {
    device_util obj_u(seed);
    return obj_u.update();
}

sdk_ret_t
device_util::many_delete(device_stepper_seed_t *seed) {
    device_util obj_d(seed);
    return obj_d.del();
}

}    // namespace api_test
