//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains route_table object utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_ROUTE_TABLE_UTILS_HPP__
#define __API_ROUTE_TABLE_UTILS_HPP__

#include <iostream>

inline std::ostream&
operator<<(std::ostream& os, const pds_route_t *route) {
    os << " pfx: " << ippfx2str(&route->prefix)
       << " nh type: " << route->nh_type;
    switch (route->nh_type) {
    case PDS_NH_TYPE_OVERLAY:
        os << " TEP id: " << route->tep.id;
        break;
    case PDS_NH_TYPE_IP:
        os << " NH id: " << route->nh.id;
        break;
    case PDS_NH_TYPE_PEER_VPC:
        os << " vpc id: " << route->vpc.id;
        break;
    default:
        break;
    }
    os << std::endl;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_route_table_key_t *key) {
    os << " id: " << key->id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_route_table_spec_t *spec) {
    os << &spec->key
       << " af: " << +spec->af
       << " num routes: " << spec->num_routes << std::endl;
    for (uint32_t i = 0; i < spec->num_routes; i++) {
        os << &spec->routes[i];
    }

    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_route_table_status_t *status) {
    os << " Base address: " << status->route_table_base_addr;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_route_table_stats_t *stats) {
    os << "  ";
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_route_table_info_t *obj) {
    os << " Route Table info =>"
       << &obj->spec
       << &obj->status
       << &obj->stats
       << std::endl;
    return os;
}

#endif    // __API_ROUTE_TABLE_UTILS_HPP__
