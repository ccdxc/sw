//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This module defines protobuf API for mapping object
///
//----------------------------------------------------------------------------

#ifndef __AGENT_SVC_MAPPING_SVC_HPP__
#define __AGENT_SVC_MAPPING_SVC_HPP__

#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/apollo/agent/svc/mapping.hpp"

static inline sdk_ret_t
pds_remote_mapping_proto_to_api_spec (pds_remote_mapping_spec_t *remote_spec,
                                      const pds::MappingSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&remote_spec->key, proto_spec.id());
    switch (proto_spec.mac_or_ip_case()) {
    case pds::MappingSpec::kMACKey:
        remote_spec->skey.type = PDS_MAPPING_TYPE_L2;
        pds_obj_key_proto_to_api_spec(&remote_spec->skey.subnet,
                                      proto_spec.mackey().subnetid());
        MAC_UINT64_TO_ADDR(remote_spec->skey.mac_addr,
                           proto_spec.mackey().macaddr());
        remote_spec->subnet = remote_spec->skey.subnet;
        break;

    case pds::MappingSpec::kIPKey:
        remote_spec->skey.type = PDS_MAPPING_TYPE_L3;
        pds_obj_key_proto_to_api_spec(&remote_spec->skey.vpc,
                                      proto_spec.ipkey().vpcid());
        ipaddr_proto_spec_to_api_spec(&remote_spec->skey.ip_addr,
                                      proto_spec.ipkey().ipaddr());
        pds_obj_key_proto_to_api_spec(&remote_spec->subnet,
                                      proto_spec.subnetid());
        break;

    default:
        return SDK_RET_INVALID_ARG;
    }

    switch (proto_spec.dstinfo_case()) {
    case pds::MappingSpec::kTunnelId:
        remote_spec->nh_type = PDS_NH_TYPE_OVERLAY;
        pds_obj_key_proto_to_api_spec(&remote_spec->tep,
                                      proto_spec.tunnelid());
        break;

    case pds::MappingSpec::kNexthopGroupId:
        remote_spec->nh_type = PDS_NH_TYPE_OVERLAY_ECMP;
        pds_obj_key_proto_to_api_spec(&remote_spec->nh_group,
                                      proto_spec.nexthopgroupid());
        break;

    case pds::MappingSpec::kVnicId:
    default:
        PDS_TRACE_ERR("Usage of vnic attribute is invalid for remote mappings");
        return SDK_RET_INVALID_ARG;
    }
    MAC_UINT64_TO_ADDR(remote_spec->vnic_mac, proto_spec.macaddr());
    remote_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.encap());
    if (proto_spec.has_providerip()) {
        if (proto_spec.providerip().af() == types::IP_AF_INET ||
            proto_spec.providerip().af() == types::IP_AF_INET6) {
            remote_spec->provider_ip_valid = true;
            ipaddr_proto_spec_to_api_spec(&remote_spec->provider_ip,
                                          proto_spec.providerip());
        }
    }
    remote_spec->num_tags = proto_spec.tags_size();
    if (remote_spec->num_tags > PDS_MAX_TAGS_PER_MAPPING) {
        PDS_TRACE_ERR("No. of tags {} on remote IP mapping exceeded max. "
                      "supported {}", remote_spec->num_tags,
                      PDS_MAX_TAGS_PER_MAPPING);
        return SDK_RET_INVALID_ARG;
    }
    for (uint32_t i = 0; i < remote_spec->num_tags; i++) {
        remote_spec->tags[i] = proto_spec.tags(i);
    }
    return SDK_RET_OK;
}

static inline void
pds_remote_mapping_api_spec_to_proto (pds::MappingSpec *proto_spec,
                                      const pds_remote_mapping_spec_t *remote_spec)
{
    if (!proto_spec || !remote_spec) {
        return;
    }

    proto_spec->set_id(remote_spec->key.id, PDS_MAX_KEY_LEN);
    switch (remote_spec->skey.type) {
    case PDS_MAPPING_TYPE_L2:
        {
            auto key = proto_spec->mutable_mackey();
            key->set_macaddr(MAC_TO_UINT64(remote_spec->skey.mac_addr));
            key->set_subnetid(remote_spec->skey.subnet.id, PDS_MAX_KEY_LEN);
        }
        break;
    case PDS_MAPPING_TYPE_L3:
        {
            auto key = proto_spec->mutable_ipkey();
            ipaddr_api_spec_to_proto_spec(key->mutable_ipaddr(),
                                          &remote_spec->skey.ip_addr);
            key->set_vpcid(remote_spec->skey.vpc.id, PDS_MAX_KEY_LEN);
        }
        break;
    default:
        return;
    }
    switch (remote_spec->nh_type) {
    case PDS_NH_TYPE_OVERLAY:
        proto_spec->set_tunnelid(remote_spec->tep.id, PDS_MAX_KEY_LEN);
        break;
    case PDS_NH_TYPE_OVERLAY_ECMP:
        proto_spec->set_nexthopgroupid(remote_spec->nh_group.id,
                                       PDS_MAX_KEY_LEN);
        break;
    default:
        return;
    }
    proto_spec->set_subnetid(remote_spec->subnet.id, PDS_MAX_KEY_LEN);
    proto_spec->set_macaddr(MAC_TO_UINT64(remote_spec->vnic_mac));
    pds_encap_to_proto_encap(proto_spec->mutable_encap(),
                             &remote_spec->fabric_encap);
    if (remote_spec->provider_ip_valid) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_providerip(),
                                      &remote_spec->provider_ip);
    }
    for (uint32_t i = 0; i < remote_spec->num_tags; i++) {
        proto_spec->add_tags(remote_spec->tags[i]);
    }
}

// build API spec from protobuf spec
static inline sdk_ret_t
pds_local_mapping_proto_to_api_spec (pds_local_mapping_spec_t *local_spec,
                                     const pds::MappingSpec &proto_spec)
{
    pds_obj_key_proto_to_api_spec(&local_spec->key, proto_spec.id());
    switch (proto_spec.mac_or_ip_case()) {
    case pds::MappingSpec::kIPKey:
        local_spec->skey.type = PDS_MAPPING_TYPE_L3;
        pds_obj_key_proto_to_api_spec(&local_spec->skey.vpc,
                                      proto_spec.ipkey().vpcid());
        ipaddr_proto_spec_to_api_spec(&local_spec->skey.ip_addr,
                                      proto_spec.ipkey().ipaddr());
        pds_obj_key_proto_to_api_spec(&local_spec->subnet,
                                      proto_spec.subnetid());
        break;

    case pds::MappingSpec::kMACKey:
    default:
        PDS_TRACE_ERR("Unsupported local mapping key type %u, local mappings "
                      "can only be L3 mappings", proto_spec.mac_or_ip_case());
        return SDK_RET_INVALID_ARG;
    }
    if (proto_spec.dstinfo_case() != pds::MappingSpec::kVnicId) {
        PDS_TRACE_ERR("Mandatory vnic attribute not set for local mapping");
        return SDK_RET_INVALID_ARG;
    }
    pds_obj_key_proto_to_api_spec(&local_spec->vnic, proto_spec.vnicid());
    if (proto_spec.has_publicip()) {
        if (proto_spec.publicip().af() == types::IP_AF_INET ||
            proto_spec.publicip().af() == types::IP_AF_INET6) {
            local_spec->public_ip_valid = true;
            ipaddr_proto_spec_to_api_spec(&local_spec->public_ip,
                                          proto_spec.publicip());
        }
    }
    if (proto_spec.has_providerip()) {
        if (proto_spec.providerip().af() == types::IP_AF_INET ||
            proto_spec.providerip().af() == types::IP_AF_INET6) {
            local_spec->provider_ip_valid = true;
            ipaddr_proto_spec_to_api_spec(&local_spec->provider_ip,
                                          proto_spec.providerip());
        }
    }
    MAC_UINT64_TO_ADDR(local_spec->vnic_mac, proto_spec.macaddr());
    local_spec->fabric_encap = proto_encap_to_pds_encap(proto_spec.encap());
    local_spec->num_tags = proto_spec.tags_size();
    if (local_spec->num_tags > PDS_MAX_TAGS_PER_MAPPING) {
        PDS_TRACE_ERR("No. of tags {} on local IP mapping exceeded max. "
                      "supported {}", local_spec->num_tags,
                      PDS_MAX_TAGS_PER_MAPPING);
        return SDK_RET_INVALID_ARG;
    }
    for (uint32_t i = 0; i < local_spec->num_tags; i++) {
        local_spec->tags[i] = proto_spec.tags(i);
    }
    return SDK_RET_OK;
}

static inline void
pds_local_mapping_api_spec_to_proto (pds::MappingSpec *proto_spec,
                                     const pds_local_mapping_spec_t *local_spec)
{
    if (!proto_spec || !local_spec) {
        return;
    }

    proto_spec->set_id(local_spec->key.id, PDS_MAX_KEY_LEN);
    switch (local_spec->skey.type) {
    case PDS_MAPPING_TYPE_L2:
        {
            auto key = proto_spec->mutable_mackey();
            key->set_macaddr(MAC_TO_UINT64(local_spec->skey.mac_addr));
            key->set_subnetid(local_spec->skey.subnet.id, PDS_MAX_KEY_LEN);
        }
        break;
    case PDS_MAPPING_TYPE_L3:
        {
            auto key = proto_spec->mutable_ipkey();
            ipaddr_api_spec_to_proto_spec(key->mutable_ipaddr(),
                                          &local_spec->skey.ip_addr);
            key->set_vpcid(local_spec->skey.vpc.id, PDS_MAX_KEY_LEN);
        }
        break;
    default:
        return;
    }
    proto_spec->set_subnetid(local_spec->subnet.id, PDS_MAX_KEY_LEN);
    proto_spec->set_macaddr(MAC_TO_UINT64(local_spec->vnic_mac));
    pds_encap_to_proto_encap(proto_spec->mutable_encap(),
                             &local_spec->fabric_encap);
    proto_spec->set_vnicid(local_spec->vnic.id, PDS_MAX_KEY_LEN);
    if (local_spec->public_ip_valid) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_publicip(),
                                      &local_spec->public_ip);
    }
    if (local_spec->provider_ip_valid) {
        ipaddr_api_spec_to_proto_spec(proto_spec->mutable_providerip(),
                                      &local_spec->provider_ip);
    }
    for (uint32_t i = 0; i < local_spec->num_tags; i++) {
        proto_spec->add_tags(local_spec->tags[i]);
    }
}

// populate proto buf status from mapping API status
static inline void
pds_mapping_api_status_to_proto (pds::MappingStatus *proto_status,
                                 const pds_mapping_status_t *api_status)
{
}

// populate proto buf stats from mapping API stats
static inline void
pds_mapping_api_stats_to_proto (pds::MappingStats *proto_stats,
                                const pds_mapping_stats_t *api_stats)
{
}

// populate proto buf from local mapping API info
static inline void
pds_local_mapping_api_info_to_proto (void *info, void *ctxt)
{
    pds_local_mapping_info_t *api_info = (pds_local_mapping_info_t *)info;
    pds::MappingGetResponse *proto_rsp = (pds::MappingGetResponse *)ctxt;
    auto mapping = proto_rsp->add_response();
    pds::MappingSpec *proto_spec = mapping->mutable_spec();
    pds::MappingStatus *proto_status = mapping->mutable_status();
    pds::MappingStats *proto_stats = mapping->mutable_stats();

    pds_local_mapping_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_mapping_api_status_to_proto(proto_status, &api_info->status);
    pds_mapping_api_stats_to_proto(proto_stats, &api_info->stats);
}

// populate proto buf from remote mapping API info
static inline void
pds_remote_mapping_api_info_to_proto (void *info, void *ctxt)
{
    pds_remote_mapping_info_t *api_info = (pds_remote_mapping_info_t *)info;
    pds::MappingGetResponse *proto_rsp = (pds::MappingGetResponse *)ctxt;
    auto mapping = proto_rsp->add_response();
    pds::MappingSpec *proto_spec = mapping->mutable_spec();
    pds::MappingStatus *proto_status = mapping->mutable_status();
    pds::MappingStats *proto_stats = mapping->mutable_stats();

    pds_remote_mapping_api_spec_to_proto(proto_spec, &api_info->spec);
    pds_mapping_api_status_to_proto(proto_status, &api_info->status);
    pds_mapping_api_stats_to_proto(proto_stats, &api_info->stats);
}

#endif    //__AGENT_SVC_MAPPING_SVC_HPP__
