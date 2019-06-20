//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains vpc object utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_VPC_UTILS_HPP__
#define __API_VPC_UTILS_HPP__

#include <iostream>
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/encap_utils.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"

inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_key_t *key) {
    os << " id: " << key->id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_spec_t *spec) {
    os << &spec->key
       << " type: " << spec->type
       << " v4 cidr: " << ipv4pfx2str(&spec->v4_prefix)
       << " v6 cidr: " << ippfx2str(&spec->v6_prefix)
       << " vr mac: " << macaddr2str(spec->vr_mac)
       << " fabric encap: " << pds_encap2str(spec->fabric_encap)
       << " v4 rt table: " << spec->v4_route_table.id
       << " v6 rt table: " << spec->v6_route_table.id
       << " nat46 pfx: " << ippfx2str(&spec->nat46_prefix);
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_status_t *status) {
    os << " HW id: " << status->hw_id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_vpc_info_t *obj) {
    os << "VPC info =>"
       << &obj->spec
       << &obj->status
       << std::endl;
    return os;
}

#endif    // __API_VPC_API_HPP__
