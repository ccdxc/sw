//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains policy object utility routines
///
//----------------------------------------------------------------------------

#ifndef __API_POLICY_UTILS_HPP__
#define __API_POLICY_UTILS_HPP__

#include <iostream>
#include "nic/apollo/api/policy.hpp"

inline std::ostream&
operator<<(std::ostream& os, const pds_policy_spec_t *spec) {
    os << " id: " << spec->key.id
       << " dir: " << spec->direction
       << " af: " << (uint32_t)spec->af
       << " num rules: " << spec->num_rules;
    return os;
}

inline std::ostream&
operator<<(std::ostream& os, const pds_policy_info_t *obj) {
    os << "Policy info =>" << &obj->spec << std::endl;
    return os;
}

#endif    // __API_POLICY_UTILS_HPP__
