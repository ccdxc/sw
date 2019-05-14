//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __CORE_TUNNEL_HPP__
#define __CORE_TUNNEL_HPP__

#include "nic/apollo/api/include/pds_route.hpp"

namespace core {

sdk_ret_t route_table_create(pds_route_table_key_t *key, pds_route_table_spec_t *spec);
sdk_ret_t route_table_update(pds_route_table_key_t *key, pds_route_table_spec_t *spec);
sdk_ret_t route_table_delete(pds_route_table_key_t *key);

}    // namespace core

#endif    // __CORE_TUNNEL_HPP__
