//------------------------------------------------------------------------------
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------
///
/// \file
/// apulu pipeline api info to proto and vice versa implementation for mapping
///
//------------------------------------------------------------------------------

#ifndef __APULU_SVC_MAPPING_SVC_HPP__
#define __APULU_SVC_MAPPING_SVC_HPP__

#include "grpc++/grpc++.h"
#include "nic/apollo/core/trace.hpp"
#include "nic/apollo/api/mapping.hpp"
#include "nic/apollo/api/impl/apulu/svc/specs.hpp"
#include "gen/proto/mapping.grpc.pb.h"

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

#endif    // __APULU_SVC_MAPPING_SVC_HPP__
