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

#include "grpc++/grpc++.h"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "gen/proto/types.pb.h"
#include "gen/proto/nh.grpc.pb.h"
#include "gen/proto/mapping.grpc.pb.h"

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

// build obj key from protobuf spec
static inline sdk_ret_t
pds_obj_key_proto_to_api_spec (pds_obj_key_t *api_spec,
                               const ::std::string& proto_key)
{
    if (proto_key.length() > PDS_MAX_KEY_LEN) {
        return SDK_RET_INVALID_ARG;
    }
    // set all the key bytes to 0 1st and hash on the full key can't include
    // uninitialized memory
    api_spec->reset();
    // set the key bytes
    memcpy(api_spec->id, proto_key.data(),
           MIN(proto_key.length(), PDS_MAX_KEY_LEN));
    return SDK_RET_OK;
}

// build nh API spec from protobuf spec
static inline void
pds_nh_proto_to_api_spec (pds_nexthop_spec_t *api_spec,
                          const pds::NexthopSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
}

// build nh group API spec from protobuf spec
static inline sdk_ret_t
pds_nh_group_proto_to_api_spec (pds_nexthop_group_spec_t *api_spec,
                                const pds::NhGroupSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    api_spec->num_nexthops = proto_spec.members_size();

    switch (proto_spec.type()) {
    case pds::NEXTHOP_GROUP_TYPE_OVERLAY_ECMP:
        if (api_spec->num_nexthops > PDS_MAX_OVERLAY_ECMP_TEP) {
            return SDK_RET_INVALID_ARG;
        }
        api_spec->type = PDS_NHGROUP_TYPE_OVERLAY_ECMP;
        break;

    case pds::NEXTHOP_GROUP_TYPE_UNDERLAY_ECMP:
        if (api_spec->num_nexthops > PDS_MAX_ECMP_NEXTHOP) {
            return SDK_RET_INVALID_ARG;
        }
        api_spec->type = PDS_NHGROUP_TYPE_UNDERLAY_ECMP;
        break;

    default:
        return SDK_RET_INVALID_ARG;
    }
    for (uint32_t i = 0; i < api_spec->num_nexthops; i++) {
        pds_nh_proto_to_api_spec(&api_spec->nexthops[i],
                                 proto_spec.members(i));
    }
    return SDK_RET_OK;
}

// populate API status from nh group proto status
static inline sdk_ret_t
pds_nh_group_proto_to_api_status (pds_nexthop_group_status_t *api_status,
                                  const pds::NhGroupStatus &proto_status)
{
    pds::NexthopStatus nh_proto_status = proto_status.members(0);
    api_status->hw_id = proto_status.hwid();
    api_status->nh_base_idx = nh_proto_status.hwid();
    return SDK_RET_OK;
}

// populate API info from nh group proto response
static inline sdk_ret_t
pds_nh_group_proto_to_api_info (pds_nexthop_group_info_t *api_info,
                                pds::NhGroupGetResponse *proto_rsp)
{
    pds_nexthop_group_spec_t *api_spec;
    pds_nexthop_group_status_t *api_status;
    pds::NhGroupSpec proto_spec;
    pds::NhGroupStatus proto_status;
    sdk_ret_t ret;

    SDK_ASSERT(proto_rsp->response_size() == 1);
    api_spec = &api_info->spec;
    api_status = &api_info->status;
    proto_spec = proto_rsp->mutable_response(0)->spec();
    proto_status = proto_rsp->mutable_response(0)->status();
    ret = pds_nh_group_proto_to_api_spec(api_spec, proto_spec);
    if (ret == SDK_RET_OK) {
        ret = pds_nh_group_proto_to_api_status(api_status, proto_status);
    } else {
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return ret;
}

//----------------------------------------------------------------------------
// convert ip_addr_t to IP address proto spec
//----------------------------------------------------------------------------
static inline sdk_ret_t
ipaddr_api_spec_to_proto_spec (types::IPAddress *out_ipaddr,
                               const ip_addr_t *in_ipaddr)
{
    if (in_ipaddr->af == IP_AF_IPV4) {
        out_ipaddr->set_af(types::IP_AF_INET);
        out_ipaddr->set_v4addr(in_ipaddr->addr.v4_addr);
    } else if (in_ipaddr->af == IP_AF_IPV6) {
        out_ipaddr->set_af(types::IP_AF_INET6);
        out_ipaddr->set_v6addr(
                    std::string((const char *)&in_ipaddr->addr.v6_addr.addr8,
                                IP6_ADDR8_LEN));
    } else {
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline void
pds_mapping_api_spec_to_proto (pds::MappingSpec *proto_spec,
                               const pds_mapping_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    switch (api_spec->skey.type) {
    case PDS_MAPPING_TYPE_L3:
        {
            auto key = proto_spec->mutable_ipkey();
            ipaddr_api_spec_to_proto_spec(key->mutable_ipaddr(),
                                          &api_spec->skey.ip_addr);
            key->set_vpcid(api_spec->skey.vpc.id, PDS_MAX_KEY_LEN);
        }
        break;

    default:
        SDK_ASSERT(0);
    }
    // TODO: how to mark 'local' in mapping proto spec
}

static inline void
pds_mapping_api_status_to_proto (pds::MappingStatus *proto_status,
                                 const pds_mapping_status_t *api_status)
{
    proto_status->set_publicipnatidx(api_status->public_ip_nat_idx);
    proto_status->set_privateipnatidx(api_status->overlay_ip_nat_idx);
}

// populate proto buf spec from mapping API spec
static inline void
pds_mapping_api_info_to_proto (pds_mapping_info_t *api_info, void *ctxt)
{
    pds::MappingGetResponse *proto_rsp = (pds::MappingGetResponse *)ctxt;
    auto mapping = proto_rsp->add_response();
    pds::MappingSpec *proto_spec = mapping->mutable_spec();
    pds::MappingStatus *proto_status = mapping->mutable_status();

    pds_mapping_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_mapping_api_status_to_proto(proto_status, &api_info->status);
}

//----------------------------------------------------------------------------
// convert IP address spec in proto to ip_addr
//----------------------------------------------------------------------------
static inline sdk_ret_t
ipaddr_proto_spec_to_api_spec (ip_addr_t *out_ipaddr,
                               const types::IPAddress &in_ipaddr)
{
    memset(out_ipaddr, 0, sizeof(ip_addr_t));
    if (in_ipaddr.af() == types::IP_AF_INET) {
        out_ipaddr->af = IP_AF_IPV4;
        out_ipaddr->addr.v4_addr = in_ipaddr.v4addr();
    } else if (in_ipaddr.af() == types::IP_AF_INET6) {
        out_ipaddr->af = IP_AF_IPV6;
        memcpy(out_ipaddr->addr.v6_addr.addr8,
               in_ipaddr.v6addr().c_str(),
               IP6_ADDR8_LEN);
    } else {
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

// populate API spec from mapping protobuf spec
static inline sdk_ret_t
pds_mapping_proto_to_api_spec (pds_mapping_spec_t *api_spec,
                               const pds::MappingSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    switch (proto_spec.mac_or_ip_case()) {
    case pds::MappingSpec::kIPKey:
        api_spec->skey.type = PDS_MAPPING_TYPE_L3;
        pds_obj_key_proto_to_api_spec(&api_spec->skey.vpc,
                                      proto_spec.ipkey().vpcid());
        ipaddr_proto_spec_to_api_spec(&api_spec->skey.ip_addr,
                                      proto_spec.ipkey().ipaddr());
        break;

    default:
        PDS_TRACE_ERR("Unsupported local mapping key type %u, local mappings "
                      "can only be L3 mappings", proto_spec.mac_or_ip_case());
        return SDK_RET_INVALID_ARG;
    }

    // mark 'local' and 'public_ip_valid' as we are sure about it
    api_spec->is_local = true;
    api_spec->public_ip_valid = true;
    return SDK_RET_OK;
}

// populate API status from mapping proto status
static inline sdk_ret_t
pds_mapping_proto_to_api_status (pds_mapping_status_t *api_status,
                                 const pds::MappingStatus &proto_status)
{
    api_status->public_ip_nat_idx = proto_status.publicipnatidx();
    api_status->overlay_ip_nat_idx = proto_status.privateipnatidx();
    return SDK_RET_OK;
}

// populate API info from mapping proto response
static inline sdk_ret_t
pds_mapping_proto_to_api_info (pds_mapping_info_t *api_info,
                               pds::MappingGetResponse *proto_rsp)
{
    pds_mapping_spec_t *api_spec;
    pds_mapping_status_t *api_status;
    pds::MappingSpec proto_spec;
    pds::MappingStatus proto_status;
    sdk_ret_t ret;

    SDK_ASSERT(proto_rsp->response_size() == 1);
    api_spec = &api_info->spec;
    api_status = &api_info->status;
    proto_spec = proto_rsp->mutable_response(0)->spec();
    proto_status = proto_rsp->mutable_response(0)->status();
    ret = pds_mapping_proto_to_api_spec(api_spec, proto_spec);
    if (ret == SDK_RET_OK) {
        ret = pds_mapping_proto_to_api_status(api_status, proto_status);
    } else {
        SDK_ASSERT(ret == SDK_RET_OK);
    }
    return ret;
}

#endif    // __SPECS_IMPL_HPP__
