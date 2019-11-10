//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains nexthop group object utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_NEXTHOP_GROUP_UTILS_HPP__
#define __API_NEXTHOP_GROUP_UTILS_HPP__

#include<iostream>
#include "nic/apollo/api/nexthop_group.hpp"

inline std::ostream&
operator<<(std::ostream& os, const pds_nexthop_group_key_t *key) {
    os << " id " << key->id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_nexthop_group_spec_t *spec) {
    os << &spec->key
        << " type " << spec->type
        << " num nexthops " << unsigned(spec->num_nexthops);
        for (uint8_t i = 0; i < spec->num_nexthops; i++) {
            os << "nexthop " << unsigned(i + 1)
               << &spec->nexthops[i] << std::endl;
        }
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_nexthop_group_status_t *status) {
    os << " HW id " << status->hw_id
        << "nh base idx " << status->nh_base_idx;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_nexthop_group_info_t *obj) {
    os << " NH group info =>"
        << &obj->spec
        << &obj->status
        << &obj->stats
        << std::endl;
    return os;
}

#endif    // __API_NEXTHOP_GROUP_UTILS_HPP__
