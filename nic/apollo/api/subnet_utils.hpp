//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains subnet object utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_SUBNET_UTILS_HPP__
#define __API_SUBNET_UTILS_HPP__

#include <iostream>
#include "nic/apollo/api/subnet.hpp"
#include "nic/sdk/include/sdk/eth.hpp"

inline std::ostream&
operator<<(std::ostream& os, const pds_subnet_spec_t *spec) {
    os  << " id: " << spec->key.id
        << " vpc: " << spec->vpc.id
        << " cidr_str: " << ipv4pfx2str(&spec->v4_prefix)
        << " vr_ip: " << ipv4addr2str(spec->v4_vr_ip)
        << " vr_mac: " << macaddr2str(spec->vr_mac)
        << " v4_rt: " << spec->v4_route_table.id
        << " v6_rt: " << spec->v6_route_table.id
        << " v4_in_pol: " << spec->ing_v4_policy.id
        << " v6_in_pol: " << spec->ing_v6_policy.id
        << " v4_eg_pol: " << spec->egr_v4_policy.id
        << " v6_eg_pol: " << spec->egr_v6_policy.id
        << " fabric encap: " << pds_encap2str(&spec->fabric_encap);
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_subnet_info_t *obj) {
    os << "Subnet info =>" << &obj->spec << std::endl;
    return os;
}

#endif    // __API_SUBNET_UTILS_HPP__
