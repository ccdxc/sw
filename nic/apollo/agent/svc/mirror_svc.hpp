//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for mirror object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_MIRROR_SVC_HPP__
#define __AGENT_SVC_MIRROR_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/mirror.hpp"

static inline pds::ERSpanType
pds_mirror_erspan_type_api_spec_to_proto (pds_erspan_type_t type)
{
    switch (type) {
    case PDS_ERSPAN_TYPE_1:
        return pds::ERSPAN_TYPE_1;
    case PDS_ERSPAN_TYPE_2:
        return pds::ERSPAN_TYPE_2;
    case PDS_ERSPAN_TYPE_3:
        return pds::ERSPAN_TYPE_3;
    default:
        break;
    }
    return pds::ERSPAN_TYPE_NONE;
}

static inline pds_erspan_type_t
pds_mirror_erspan_type_proto_to_api_spec (pds::ERSpanType type)
{
    switch (type) {
    case pds::ERSPAN_TYPE_1:
        return PDS_ERSPAN_TYPE_1;
    case pds::ERSPAN_TYPE_2:
        return PDS_ERSPAN_TYPE_2;
    case pds::ERSPAN_TYPE_3:
        return PDS_ERSPAN_TYPE_3;
    default:
        break;
    }
    return PDS_ERSPAN_TYPE_NONE;
}

// populate proto buf spec from mirror session API spec
static inline sdk_ret_t
pds_mirror_session_api_spec_to_proto (pds::MirrorSessionSpec *proto_spec,
                                      const pds_mirror_session_spec_t *api_spec)
{
    proto_spec->set_id(api_spec->key.id, PDS_MAX_KEY_LEN);
    proto_spec->set_snaplen(api_spec->snap_len);
    switch (api_spec->type) {
    case PDS_MIRROR_SESSION_TYPE_RSPAN:
        {
            pds::RSpanSpec *proto_rspan = proto_spec->mutable_rspanspec();
            pds_encap_to_proto_encap(proto_rspan->mutable_encap(),
                                     &api_spec->rspan_spec.encap);
            proto_rspan->set_interface(api_spec->rspan_spec.interface.id,
                                       PDS_MAX_KEY_LEN);
        }
        break;

    case PDS_MIRROR_SESSION_TYPE_ERSPAN:
        {
            pds::ERSpanSpec *proto_erspan = proto_spec->mutable_erspanspec();
            proto_erspan->set_type(pds_mirror_erspan_type_api_spec_to_proto(api_spec->erspan_spec.type));
            proto_erspan->set_vpcid(api_spec->erspan_spec.vpc.id,
                                    PDS_MAX_KEY_LEN);
            switch (api_spec->erspan_spec.dst_type) {
            case PDS_ERSPAN_DST_TYPE_TEP:
                proto_erspan->set_tunnelid(api_spec->erspan_spec.tep.id,
                                           PDS_MAX_KEY_LEN);
                break;
            case PDS_ERSPAN_DST_TYPE_IP:
                ipaddr_api_spec_to_proto_spec(proto_erspan->mutable_dstip(),
                                              &api_spec->erspan_spec.ip_addr);
                break;
            default:
                return SDK_RET_INVALID_ARG;
                break;
            }
            proto_erspan->set_dscp(api_spec->erspan_spec.dscp);
            proto_erspan->set_spanid(api_spec->erspan_spec.span_id);
        }
        break;

    default:
        PDS_TRACE_ERR("Unknown mirror session type {}", api_spec->type);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

// populate proto buf status from mirror session API status
static inline void
pds_mirror_session_api_status_to_proto (pds::MirrorSessionStatus *proto_status,
                                        const pds_mirror_session_status_t *api_status)
{
}

// populate proto buf stats from mirror session API stats
static inline void
pds_mirror_session_api_stats_to_proto (pds::MirrorSessionStats *proto_stats,
                                       const pds_mirror_session_stats_t *api_stats)
{
}

// populate proto buf from mirror session API info
static inline void
pds_mirror_session_api_info_to_proto (const pds_mirror_session_info_t *api_info,
                                      void *ctxt)
{
    pds::MirrorSessionGetResponse *proto_rsp =
        (pds::MirrorSessionGetResponse *)ctxt;
    auto mirror_session = proto_rsp->add_response();
    pds::MirrorSessionSpec *proto_spec = mirror_session->mutable_spec();
    pds::MirrorSessionStatus *proto_status = mirror_session->mutable_status();
    pds::MirrorSessionStats *proto_stats = mirror_session->mutable_stats();

    pds_mirror_session_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_mirror_session_api_status_to_proto(proto_status, &api_info->status);
    pds_mirror_session_api_stats_to_proto(proto_stats, &api_info->stats);
}

// build mirror session API spec from protobuf spec
static inline sdk_ret_t
pds_mirror_session_proto_to_api_spec (pds_mirror_session_spec_t *api_spec,
                                      const pds::MirrorSessionSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&api_spec->key, proto_spec.id());
    api_spec->snap_len = proto_spec.snaplen();
    if (proto_spec.has_rspanspec()) {
        api_spec->type = PDS_MIRROR_SESSION_TYPE_RSPAN;
        api_spec->rspan_spec.encap =
            proto_encap_to_pds_encap(proto_spec.rspanspec().encap());
        if (api_spec->rspan_spec.encap.type != PDS_ENCAP_TYPE_DOT1Q) {
            PDS_TRACE_ERR("Invalid encap type {} in RSPAN mirror session {} "
                          "spec, only PDS_ENCAP_TYPE_DOT1Q encap type is valid",
                          api_spec->rspan_spec.encap.type, api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        if (api_spec->rspan_spec.encap.val.vlan_tag == 0) {
            PDS_TRACE_ERR("Invalid vlan tag 0 in RSPAN mirror session {} spec",
                          api_spec->key.id);
            return SDK_RET_INVALID_ARG;
        }
        pds_obj_key_proto_to_api_spec(&api_spec->rspan_spec.interface,
                                      proto_spec.rspanspec().interface());
    } else if (proto_spec.has_erspanspec()) {
        api_spec->type = PDS_MIRROR_SESSION_TYPE_ERSPAN;
        api_spec->erspan_spec.type =
            pds_mirror_erspan_type_proto_to_api_spec(proto_spec.erspanspec().type());
        pds_obj_key_proto_to_api_spec(&api_spec->erspan_spec.vpc,
                                      proto_spec.erspanspec().vpcid());
        if (api_spec->erspan_spec.dst_type == PDS_ERSPAN_DST_TYPE_TEP) {
            pds_obj_key_proto_to_api_spec(&api_spec->erspan_spec.tep,
                                          proto_spec.erspanspec().tunnelid());
        } else if (api_spec->erspan_spec.dst_type == PDS_ERSPAN_DST_TYPE_IP) {
            ipaddr_proto_spec_to_api_spec(&api_spec->erspan_spec.ip_addr,
                                          proto_spec.erspanspec().dstip());
        }
        api_spec->erspan_spec.dscp = proto_spec.erspanspec().dscp();
        api_spec->erspan_spec.span_id = proto_spec.erspanspec().spanid();
    } else {
        PDS_TRACE_ERR("rspan & erspan config missing in mirror session {} spec",
                      api_spec->key.id);
        return SDK_RET_INVALID_ARG;
    }
    return SDK_RET_OK;
}

#endif    //__AGENT_SVC_MIRROR_SVC_HPP__
