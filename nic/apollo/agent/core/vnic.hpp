//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_TUNNEL_HPP__
#define __CORE_TUNNEL_HPP__

#include "nic/apollo/api/include/pds_vnic.hpp"

namespace core {

sdk_ret_t vnic_create(pds_vnic_key_t *key, pds_vnic_spec_t *spec);
sdk_ret_t vnic_delete(pds_vnic_key_t *key);

}    // namespace core

#endif    // __CORE_TUNNEL_HPP__
