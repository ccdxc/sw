//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for nat object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_NAT_SVC_HPP__
#define __AGENT_SVC_NAT_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"

static inline sdk_ret_t
pds_nat_port_block_proto_to_api_spec (pds_nat_port_block_spec_t *api_spec,
                                      const pds::NatPortBlockSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    pds_obj_key_proto_to_api_spec(&api_spec->vpc, proto_spec.vpcid());
    api_spec->ip_proto = proto_spec.protocol();
    if (proto_spec.nataddress().has_prefix()) {
        ipsubnet_proto_spec_to_ipvx_range(&api_spec->nat_ip_range,
                                          proto_spec.nataddress().prefix());
    } else if (proto_spec.nataddress().has_range()) {
        iprange_proto_spec_to_api_spec(&api_spec->nat_ip_range,
                                       proto_spec.nataddress().range());
    }
    api_spec->nat_port_range.port_lo = proto_spec.ports().portlow();
    api_spec->nat_port_range.port_hi = proto_spec.ports().porthigh();
    switch (proto_spec.addresstype()) {
    case types::ADDR_TYPE_PUBLIC:
        api_spec->address_type = ADDR_TYPE_PUBLIC;
        break;
    case types::ADDR_TYPE_SERVICE:
        api_spec->address_type = ADDR_TYPE_SERVICE;
        break;
    default:
        SDK_ASSERT(FALSE);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_nat_port_block_api_spec_to_proto (pds::NatPortBlockSpec *proto_spec,
                                      const pds_nat_port_block_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_vpcid(api_spec->vpc.id, PDS_MAX_KEY_LEN);
    proto_spec->set_protocol(api_spec->ip_proto);
    auto range_spec = proto_spec->mutable_nataddress()->mutable_range();
    iprange_api_spec_to_proto_spec(range_spec, &api_spec->nat_ip_range);
    proto_spec->mutable_ports()->set_portlow(api_spec->nat_port_range.port_lo);
    proto_spec->mutable_ports()->set_porthigh(api_spec->nat_port_range.port_hi);
    switch (api_spec->address_type) {
    case ADDR_TYPE_PUBLIC:
        proto_spec->set_addresstype(types::ADDR_TYPE_PUBLIC);
        break;
    case ADDR_TYPE_SERVICE:
        proto_spec->set_addresstype(types::ADDR_TYPE_SERVICE);
        break;
    default:
        SDK_ASSERT(FALSE);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_nat_port_block_api_status_to_proto (pds::NatPortBlockStatus *proto_status,
                                        const pds_nat_port_block_status_t *api_status)
{
    return SDK_RET_OK;
}

static inline sdk_ret_t
pds_nat_port_block_api_stats_to_proto (pds::NatPortBlockStats *proto_stats,
                                       const pds_nat_port_block_stats_t *api_stats)
{
    proto_stats->set_inusecount(api_stats->in_use_count);
    proto_stats->set_sessioncount(api_stats->session_count);
    return SDK_RET_OK;
}

// populate proto buf from route table API info
static inline void
pds_nat_port_block_api_info_to_proto (const pds_nat_port_block_info_t *api_info,
                                      void *ctxt)
{
    pds::NatPortBlockGetResponse *proto_rsp =
        (pds::NatPortBlockGetResponse *)ctxt;
    auto nat = proto_rsp->add_response();
    pds::NatPortBlockSpec *proto_spec = nat->mutable_spec();
    pds::NatPortBlockStatus *proto_status = nat->mutable_status();
    pds::NatPortBlockStats *proto_stats = nat->mutable_stats();

    pds_nat_port_block_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_nat_port_block_api_status_to_proto(proto_status, &api_info->status);
    pds_nat_port_block_api_stats_to_proto(proto_stats, &api_info->stats);
}

#endif    //__AGENT_SVC_NAT_SVC_HPP__
