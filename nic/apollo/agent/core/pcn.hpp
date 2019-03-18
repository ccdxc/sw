//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_PCN_HPP__
#define __CORE_PCN_HPP__

#include "nic/apollo/api/include/pds_vcn.hpp"

namespace core {

sdk_ret_t pcn_create(pds_vcn_key_t *key, pds_vcn_spec_t *spec);
sdk_ret_t pcn_delete(pds_vcn_key_t *key);

}    // namespace core

#endif    // __CORE_PCN_HPP__
