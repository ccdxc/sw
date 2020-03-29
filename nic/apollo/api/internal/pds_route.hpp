//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file defines route APIs for internal module interactions
///
//----------------------------------------------------------------------------

#ifndef __INTERNAL_PDS_ROUTE_HPP__
#define __INTERNAL_PDS_ROUTE_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/types.hpp"
#include "nic/apollo/api/include/pds.hpp"

namespace api {

sdk_ret_t pds_underlay_route_update(_In_ pds_route_spec_t *spec);
sdk_ret_t pds_underlay_route_delete(_In_ ip_prefix_t *prefix);
sdk_ret_t pds_underlay_nexthop(_In_ ipv4_addr_t ip_addr,
                               _Out_ pds_nh_type_t *nh_type,
                               _Out_ pds_obj_key_t *nh);

}    // namespace api

#endif    // __INTERNAL_PDS_ROUTE_HPP__
