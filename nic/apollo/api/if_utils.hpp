//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains interface object utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_IF_UTILS_HPP__
#define __API_IF_UTILS_HPP__

#include <iostream>
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/apollo/api/if.hpp"

inline std::ostream&
operator<<(std::ostream& os, const pds_if_spec_t *spec) {
    os << " key: " << spec->key.id
       << " type: " << spec->type;

    if (spec->type == PDS_IF_TYPE_L3) {
        os << " port num: " << (uint32_t)spec->l3_if_info.port_num
           << " ip pfx: " << ippfx2str(&spec->l3_if_info.ip_prefix)
           << " mac addr: " << macaddr2str(spec->l3_if_info.mac_addr);
    }

    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_if_info_t *obj) {
    os << "Interface info =>" << &obj->spec << std::endl;
    return os;
}

#endif    // __API_IF_UTILS_HPP__
