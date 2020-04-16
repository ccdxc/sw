
//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for service object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_SERVICE_SVC_HPP__
#define __AGENT_SVC_SERVICE_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"

// populate proto buf spec from service API spec
static inline void
pds_service_api_spec_to_proto (pds::SvcMappingSpec *proto_spec,
                               const pds_svc_mapping_spec_t *api_spec)
{
    if (!proto_spec || !api_spec) {
        return;
    }

    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    auto proto_key = proto_spec->mutable_key();
    proto_key->set_vpcid(api_spec->skey.vpc.id, PDS_MAX_KEY_LEN);
    proto_key->set_backendport(api_spec->skey.backend_port);
    ipaddr_api_spec_to_proto_spec(
                proto_key->mutable_backendip(), &api_spec->skey.backend_ip);
    ipaddr_api_spec_to_proto_spec(
                proto_spec->mutable_ipaddr(), &api_spec->vip);
    // provider IP is optional
    if (api_spec->backend_provider_ip.af != IP_AF_NIL) {
        ipaddr_api_spec_to_proto_spec(
                    proto_spec->mutable_providerip(),
                    &api_spec->backend_provider_ip);
    }
    proto_spec->set_svcport(api_spec->svc_port);
}

// populate proto buf status from service API status
static inline void
pds_service_api_status_to_proto (pds::SvcMappingStatus *proto_status,
                                 const pds_svc_mapping_status_t *api_status)
{
}

// populate proto buf stats from service API stats
static inline void
pds_service_api_stats_to_proto (pds::SvcMappingStats *proto_stats,
                                const pds_svc_mapping_stats_t *api_stats)
{
}

// populate proto buf from service API info
static inline void
pds_service_api_info_to_proto (pds_svc_mapping_info_t *api_info,
                               void *ctxt)
{
    pds::SvcMappingGetResponse *proto_rsp = (pds::SvcMappingGetResponse *)ctxt;
    auto service = proto_rsp->add_response();
    pds::SvcMappingSpec *proto_spec = service->mutable_spec();
    pds::SvcMappingStatus *proto_status = service->mutable_status();
    pds::SvcMappingStats *proto_stats = service->mutable_stats();

    pds_service_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_service_api_status_to_proto(proto_status, &api_info->status);
    pds_service_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build service API spec from proto buf spec
static inline void
pds_service_proto_to_api_spec (pds_svc_mapping_spec_t *api_spec,
                               const pds::SvcMappingSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    pds_obj_key_proto_to_api_spec(&api_spec->skey.vpc,
                                  proto_spec.key().vpcid());
    api_spec->skey.backend_port = proto_spec.key().backendport();
    ipaddr_proto_spec_to_api_spec(&api_spec->skey.backend_ip,
                                  proto_spec.key().backendip());
    ipaddr_proto_spec_to_api_spec(&api_spec->vip,
                                  proto_spec.ipaddr());
    ipaddr_proto_spec_to_api_spec(&api_spec->backend_provider_ip,
                                  proto_spec.providerip());
    api_spec->svc_port = proto_spec.svcport();
}

#endif    //__AGENT_SVC_SERVICE_SVC_HPP__
