//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains device object utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_DEVICE_API_HPP__
#define __API_DEVICE_API_HPP__

#include <iostream>
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/device.hpp"

inline std::ostream&
operator<<(std::ostream& os, const pds_device_spec_t *spec) {
    os << " ip: " << ipv4addr2str(spec->device_ip_addr)
       << " mac: " << macaddr2str(spec->device_mac_addr)
       << " gw ip: " << ipv4addr2str(spec->gateway_ip_addr);
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_device_info_t *obj) {
    os << "Device info =>"
       << &obj->spec
       << std::endl;
    return os;
}

#endif    // __API_DEVICE_API_HPP__
