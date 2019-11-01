//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// -----------------------------------------------------------------------------

#ifndef __PDSA_SVC_SPECS_HPP__
#define __PDSA_SVC_SPECS_HPP__

#include "nic/metaswitch/svc/bgp.hpp"
#include "nic/metaswitch/include/bgp_api.hpp"
#include "gen/proto/types.pb.h"

// build VPC API spec from protobuf spec
static inline void
pds_bgp_global_proto_to_api_spec (bgp_global_spec_t *api_spec,
                           const pds::BGPGlobalSpec &proto_spec)
{
    return;
}

#endif    // __PDSA_SVC_SPECS_HPP__
