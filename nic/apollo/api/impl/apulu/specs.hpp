//------------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
///
/// \file
/// apulu pipeline api info to proto and vice versa implementation
///
//------------------------------------------------------------------------------

#ifndef __SPECS_IMPL_HPP__
#define __SPECS_IMPL_HPP__

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"

// populate proto buf spec from nh API spec
static inline void
pds_nh_api_spec_to_proto (pds::NexthopSpec *proto_spec,
                          const pds_nexthop_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
}

// populate proto buf status from nh API status
static inline void
pds_nh_api_status_to_proto (pds::NexthopStatus *proto_status,
                            const pds_nexthop_status_t *api_status,
                            const pds_nexthop_spec_t *api_spec)
{
    proto_status->set_hwid(api_status->hw_id);
}

// populate proto buf spec from nh group API spec
static inline sdk_ret_t
pds_nh_group_api_spec_to_proto (pds::NhGroupSpec *proto_spec,
                                const pds_nexthop_group_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    switch (api_spec->type) {
    case PDS_NHGROUP_TYPE_OVERLAY_ECMP:
        proto_spec->set_type(pds::NEXTHOP_GROUP_TYPE_OVERLAY_ECMP);
        break;

    case PDS_NHGROUP_TYPE_UNDERLAY_ECMP:
        proto_spec->set_type(pds::NEXTHOP_GROUP_TYPE_UNDERLAY_ECMP);
        break;

    default:
        return SDK_RET_INVALID_ARG;
    }
    for (uint32_t i = 0; i < api_spec->num_nexthops; i++) {
        pds_nh_api_spec_to_proto(proto_spec->add_members(),
                                 &api_spec->nexthops[i]);
    }
    return SDK_RET_OK;
}

// populate proto buf status from nh group API status
static inline void
pds_nh_group_api_status_to_proto (pds::NhGroupStatus *proto_status,
                                  const pds_nexthop_group_status_t *api_status,
                                  const pds_nexthop_group_spec_t *api_spec)
{
    proto_status->set_hwid(api_status->hw_id);

    for (uint32_t i = 0; i < api_spec->num_nexthops; i++) {
        pds_nh_api_status_to_proto(proto_status->add_members(),
                                   &api_status->nexthops[i],
                                   &api_spec->nexthops[i]);
    }
}

// populate proto buf from nh API info, only what is needed
static inline void
pds_nh_group_api_info_to_proto (pds_nexthop_group_info_t *api_info,
                                void *ctxt)
{
    pds::NhGroupGetResponse *proto_rsp = (pds::NhGroupGetResponse *)ctxt;
    auto nh = proto_rsp->add_response();
    pds::NhGroupSpec *proto_spec = nh->mutable_spec();
    pds::NhGroupStatus *proto_status = nh->mutable_status();

    pds_nh_group_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_nh_group_api_status_to_proto(proto_status, &api_info->status,
                                     &api_info->spec);
}

#endif    // __SPECS_IMPL_HPP__
