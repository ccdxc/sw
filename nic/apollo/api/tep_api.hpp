//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains tep object utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_TEP_API_HPP__
#define __API_TEP_API_HPP__

#include<iostream>
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/tep.hpp"

inline std::ostream&
operator<<(std::ostream& os, const pds_tep_key_t *key) {
    os << " IP: " << key->ip_addr;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_tep_spec_t *spec) {
    os << &spec->key
       << " type: " << spec->type
       << " DIPi: " << spec->ip_addr
       << " dmac: " << macaddr2str(spec->mac)
       << " nat: " << spec->nat
       << " encap: " << pdsencap2str(spec->encap);
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
    os << "TEP info => "
       << &obj->spec
       << &obj->status
       << std::endl;
    return os;
}

#endif    // __API_TEP_API_HPP__
