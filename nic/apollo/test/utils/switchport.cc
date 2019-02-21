//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the switchport test utility routines implementation
///
//----------------------------------------------------------------------------

#include "nic/apollo/test/utils/switchport.hpp"

namespace api_test {

switchport_util::switchport_util() {
    this->switchport_ip_str = "0.0.0.0";
    this->mac_addr_str = "00:00:00:00:00:00";
    this->gateway_ip_str = "0.0.0.0";
}

switchport_util::switchport_util(std::string switchport_ip_str,
                                 std::string mac_addr_str,
                                 std::string gateway_ip_str) {
    this->switchport_ip_str = switchport_ip_str;
    this->mac_addr_str = mac_addr_str;
    this->gateway_ip_str = gateway_ip_str;
}

switchport_util::~switchport_util() {}

sdk::sdk_ret_t
switchport_util::create() {
    oci_switchport_spec_t oci_switchport;
    ip_prefix_t switchport_ip_pfx, gateway_ip_pfx;

    SDK_ASSERT(str2ipv4pfx((char *)this->switchport_ip_str.c_str(),
                           &switchport_ip_pfx) == 0);
    SDK_ASSERT(str2ipv4pfx((char *)this->gateway_ip_str.c_str(),
                           &gateway_ip_pfx) == 0);
    memset(&oci_switchport, 0, sizeof(oci_switchport_spec_t));
    oci_switchport.switch_ip_addr = switchport_ip_pfx.addr.addr.v4_addr;
    oci_switchport.gateway_ip_addr = gateway_ip_pfx.addr.addr.v4_addr;
    mac_str_to_addr((char *)this->mac_addr_str.c_str(),
                    oci_switchport.switch_mac_addr);
    return (oci_switchport_create(&oci_switchport));
}

sdk::sdk_ret_t
switchport_util::validate(oci_switchport_info_t *info) {
    if (this->switchport_ip_str.compare(
            ipv4addr2str(info->spec.switch_ip_addr)) != 0) {
        return SDK_RET_ERR;
    }

    if (this->mac_addr_str.compare(macaddr2str(info->spec.switch_mac_addr)) !=
        0) {
        return SDK_RET_ERR;
    }

    if (this->gateway_ip_str.compare(
            ipv4addr2str(info->spec.gateway_ip_addr)) != 0) {
        return SDK_RET_ERR;
    }

    return sdk::SDK_RET_OK;
}

sdk::sdk_ret_t
switchport_util::read(oci_switchport_info_t *info) {
    memset(info, 0, sizeof(oci_switchport_info_t));
    return (oci_switchport_read(info));
}

sdk::sdk_ret_t
switchport_util::del() {
    return (oci_switchport_delete());
}

}    // namespace api_test
