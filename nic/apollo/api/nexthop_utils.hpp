//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains nexthop object utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_NEXTHOP_UTILS_HPP__
#define __API_NEXTHOP_UTILS_HPP__

#include<iostream>
#include "nic/apollo/api/nexthop.hpp"

inline std::ostream&
operator<<(std::ostream& os, const pds_nexthop_key_t *key) {
    os << " id: " << key->id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_nexthop_spec_t *spec) {
    os << &spec->key
       << " type: " << spec->type;
    if (spec->type == PDS_NH_TYPE_IP) {
        os << " ip: " << spec->ip
           << " mac: " << macaddr2str(spec->mac)
           << " vlan: " << spec->vlan
           << " vpc: " << spec->vpc.id;
    } else if (spec->type == PDS_NH_TYPE_UNDERLAY) {
        os << " underlay mac:" << macaddr2str(spec->underlay_mac)
           << " l3 if: " << std::hex << spec->l3_if.id;
    } else if (spec->type == PDS_NH_TYPE_OVERLAY) {
        os << " tep: " << spec->tep.id;
    }
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_nexthop_status_t *status) {
    os << " HW id: " << status->hw_id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_nexthop_info_t *obj) {
    os << " NH info =>"
       << &obj->spec
       << &obj->status
       << &obj->stats
       << std::endl;
    return os;
}

#endif    // __API_NEXTHOP_UTILS_HPP__
