//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains tep object utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_TEP_UTILS_HPP__
#define __API_TEP_UTILS_HPP__

#include <iostream>
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/include/pds_tep.hpp"

inline std::ostream&
operator<<(std::ostream& os, const pds_tep_key_t *key) {
    os << " ID: " << key->id;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_tep_spec_t *spec) {
    os << &spec->key
       << " vpc: " << spec->vpc.id
       << " type: " << spec->type
       << " remote ip: " << spec->remote_ip
       << " DIPi: " << spec->ip_addr
       << " dmac: " << macaddr2str(spec->mac)
       << " nat: " << spec->nat
       << " encap: " << pds_encap2str(&spec->encap)
       << " remote svc: " << spec->remote_svc
       << " remote svc encap: " << pds_encap2str(&spec->remote_svc_encap)
       << " remote svc public ip: " << spec->remote_svc_public_ip
       << " nh type: " << spec->nh_type;
    switch (spec->nh_type) {
    case PDS_NH_TYPE_UNDERLAY_ECMP:
        os << " nh group id: " << spec->nh_group.id;
        break;
    case PDS_NH_TYPE_UNDERLAY:
        os << " nh id: " << spec->nh.id;
        break;
    default:
        break;
    }
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_tep_status_t *status) {
    os << " HW id: " << status->hw_id
       << " NH id: " << status->nh_id
       << " DMAC: " << macaddr2str(status->dmac);
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_tep_info_t *obj) {
    os << "TEP info =>"
       << &obj->spec
       << &obj->status
       << std::endl;
    return os;
}

#endif    // __API_TEP_UTILS_HPP__
