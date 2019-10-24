//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#ifndef __TEST_APP_UTILS_HPP__
#define __TEST_APP_UTILS_HPP__

#include <grpc++/grpc++.h>
#include "gen/proto/batch.grpc.pb.h"
#include "gen/proto/gogo.grpc.pb.h"
#include "gen/proto/mapping.grpc.pb.h"
#include "gen/proto/vpc.grpc.pb.h"
#include "gen/proto/route.grpc.pb.h"
#include "gen/proto/policy.grpc.pb.h"
#include "gen/proto/subnet.grpc.pb.h"
#include "gen/proto/device.grpc.pb.h"
#include "gen/proto/tunnel.grpc.pb.h"
#include "gen/proto/vnic.grpc.pb.h"
#include "gen/proto/mirror.grpc.pb.h"
#include "gen/proto/meter.grpc.pb.h"
#include "gen/proto/tags.grpc.pb.h"
#include "gen/proto/nh.grpc.pb.h"
#include "gen/proto/types.grpc.pb.h"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vpc.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_mirror.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_meter.hpp"
#include "nic/apollo/api/include/pds_tag.hpp"
#include "nic/apollo/api/include/pds_nexthop.hpp"
#include "nic/apollo/agent/svc/specs.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientContext;
using pds::BatchSpec;
using pds::BatchStatus;
using types::BatchCtxt;
using pds::Batch;
using pds::MappingKey;
using pds::MappingRequest;
using pds::MappingSpec;
using pds::MappingResponse;
using pds::Mapping;
using pds::VPCRequest;
using pds::VPCSpec;
using pds::VPCResponse;
using pds::VPC;
using pds::VPCPeerRequest;
using pds::VPCPeerSpec;
using pds::VPCPeerResponse;
using pds::VPCDeleteRequest;
using pds::VPCDeleteResponse;
using pds::VPCGetRequest;
using pds::VPCGetResponse;
using pds::Route;
using pds::RouteTableRequest;
using pds::RouteTableSpec;
using pds::RouteTableResponse;
using pds::RouteTable;
using pds::SubnetRequest;
using pds::SubnetSpec;
using pds::SubnetResponse;
using pds::Subnet;
using pds::DeviceRequest;
using pds::DeviceSpec;
using pds::DeviceResponse;
using pds::Device;
using pds::TunnelRequest;
using pds::TunnelSpec;
using pds::TunnelResponse;
using pds::Tunnel;
using pds::VnicRequest;
using pds::VnicSpec;
using pds::VnicResponse;
using pds::Vnic;
using pds::SecurityPolicyRequest;
using pds::SecurityPolicySpec;
using pds::SecurityPolicyResponse;
using pds::SecurityPolicy;
using pds::SecurityRule;
using pds::MirrorSession;
using pds::MirrorSessionRequest;
using pds::MirrorSessionResponse;
using pds::MirrorSessionSpec;
using pds::MeterRequest;
using pds::MeterResponse;
using pds::MeterSpec;
using pds::MeterRuleSpec;
using pds::TagRequest;
using pds::TagResponse;
using pds::TagSpec;
using pds::NexthopRequest;
using pds::NexthopResponse;
using pds::NexthopSpec;

static void
populate_route_table_request (RouteTableRequest *req,
                              pds_route_table_spec_t *rt)
{
    if (!rt || !req) {
        return;
    }

    RouteTableSpec *spec = req->add_request();

    if (rt->af == IP_AF_IPV4) {
        spec->set_af(types::IP_AF_INET);
    } else if (rt->af == IP_AF_IPV6) {
        spec->set_af(types::IP_AF_INET6);
    }
    spec->set_id(rt->key.id);

    for (uint32_t i = 0; i < rt->num_routes; i++) {
        Route *route = spec->add_routes();
        ippfx_api_spec_to_proto_spec(route->mutable_prefix(),
                                     &rt->routes[i].prefix);
        if (rt->routes[i].nh_type == PDS_NH_TYPE_PEER_VPC) {
            route->set_vpcid(rt->routes[i].vpc.id);
        } else if (rt->routes[i].nh_type == PDS_NH_TYPE_OVERLAY) {
            route->set_tunnelid(rt->routes[i].tep.id);
        } else if (rt->routes[i].nh_type == PDS_NH_TYPE_IP) {
            route->set_nexthopid(rt->routes[i].nh.id);
        }
    }

    return;
}

static void
populate_local_mapping_request (MappingRequest *req,
                                pds_local_mapping_spec_t *local_spec)
{
    if (!local_spec || !req) {
        return;
    }

    MappingSpec *spec = req->add_request();
    spec->mutable_id()->mutable_ipkey()->set_vpcid(local_spec->key.vpc.id);
    ipaddr_api_spec_to_proto_spec(
        spec->mutable_id()->mutable_ipkey()->mutable_ipaddr(),
        &local_spec->key.ip_addr);
    spec->set_subnetid(local_spec->subnet.id);
    spec->set_macaddr(MAC_TO_UINT64(local_spec->vnic_mac));
    pds_encap_to_proto_encap(spec->mutable_encap(), &local_spec->fabric_encap);
    spec->set_vnicid(local_spec->vnic.id);
    if (local_spec->public_ip_valid) {
        ipaddr_api_spec_to_proto_spec(spec->mutable_publicip(),
                                      &local_spec->public_ip);
    }
    if (local_spec->provider_ip_valid) {
        ipaddr_api_spec_to_proto_spec(spec->mutable_providerip(),
                                      &local_spec->provider_ip);
    }
    spec->set_servicetag(local_spec->svc_tag);
    return;
}


static void
populate_remote_mapping_request (MappingRequest *req,
                                 pds_remote_mapping_spec_t *remote_spec)
{
    if (!remote_spec || !req) {
        return;
    }

    MappingSpec *spec = req->add_request();
    spec->mutable_id()->mutable_ipkey()->set_vpcid(remote_spec->key.vpc.id);
    ipaddr_api_spec_to_proto_spec(
        spec->mutable_id()->mutable_ipkey()->mutable_ipaddr(),
        &remote_spec->key.ip_addr);
    spec->set_subnetid(remote_spec->subnet.id);
    spec->set_tunnelid(remote_spec->tep.id);
    spec->set_macaddr(MAC_TO_UINT64(remote_spec->vnic_mac));
    pds_encap_to_proto_encap(spec->mutable_encap(), &remote_spec->fabric_encap);
    if (remote_spec->provider_ip_valid == true) {
        ipaddr_api_spec_to_proto_spec(spec->mutable_providerip(),
                                      &remote_spec->provider_ip);
    }
    return;
}

static void
populate_subnet_request (SubnetRequest *req, pds_subnet_spec_t *subnet)
{
    if (!subnet || !req) {
        return;
    }

    SubnetSpec *spec = req->add_request();
    ipv4pfx_api_spec_to_proto_spec(spec->mutable_v4prefix(),
                                   &subnet->v4_prefix);
    ippfx_api_spec_to_ipv6pfx_proto_spec(spec->mutable_v6prefix(),
                                         &subnet->v6_prefix);
    spec->set_ipv4virtualrouterip(subnet->v4_vr_ip);
    spec->set_ipv6virtualrouterip(subnet->v6_vr_ip.addr.v6_addr.addr8,
                                  IP6_ADDR8_LEN);
    spec->set_id(subnet->key.id);
    spec->set_vpcid(subnet->vpc.id);
    spec->set_virtualroutermac(MAC_TO_UINT64(subnet->vr_mac));
    spec->set_v4routetableid(subnet->v4_route_table.id);
    spec->set_v6routetableid(subnet->v6_route_table.id);
    spec->set_ingv4securitypolicyid(subnet->ing_v4_policy.id);
    spec->set_ingv6securitypolicyid(subnet->ing_v6_policy.id);
    spec->set_egv4securitypolicyid(subnet->egr_v4_policy.id);
    spec->set_egv6securitypolicyid(subnet->egr_v6_policy.id);

    return;
}

static void
populate_vpc_request (VPCRequest *req, pds_vpc_spec_t *vpc)
{
    if (!vpc || !req) {
        return;
    }

    VPCSpec *spec = req->add_request();
    pds_vpc_api_spec_to_proto(spec, vpc);
    return;
}

// TODO : Use single function template for delete and get
static void
populate_vpc_delete_request (VPCDeleteRequest *req, pds_vpc_key_t *key)
{
    if (!key || !req) {
        return;
    }
    req->add_id(key->id);

    return;
}

static void
populate_vpc_get_request (VPCGetRequest *req, pds_vpc_key_t *key)
{
    if (!key || !req) {
        return;
    }
    req->add_id(key->id);

    return;
}

static void
populate_tunnel_request (TunnelRequest *req,
                         uint32_t tep_id, pds_tep_spec_t *tep)
{
    types::IPAddress ip_addr_spec;
    TunnelSpec *spec;

    if (!tep || !req) {
        return;
    }
    spec = req->add_request();
    pds_tep_api_spec_to_proto(spec, tep);
    return;
}

static void
populate_device_request (DeviceRequest *req, pds_device_spec_t *device)
{
    DeviceSpec *spec;

    if (!device || !req) {
        return;
    }
    spec = req->mutable_request();
    pds_device_api_spec_to_proto(spec, device);
    return;
}

static void
populate_mirror_session_request (MirrorSessionRequest *req,
                                 pds_mirror_session_spec_t *ms)
{
    MirrorSessionSpec *spec;

    if (!req || !ms) {
        return;
    }
    spec = req->add_request();
    spec->set_id(ms->key.id);
    spec->set_snaplen(ms->snap_len);
    if (ms->type == PDS_MIRROR_SESSION_TYPE_RSPAN) {
        spec->mutable_rspanspec()->set_interfaceid(ms->rspan_spec.interface);
        pds_encap_to_proto_encap(spec->mutable_rspanspec()->mutable_encap(),
                                 &ms->rspan_spec.encap);
    } else {
    }
}

static void
populate_svc_mapping_request (SvcMappingRequest *req,
                              pds_svc_mapping_spec_t *svc_mapping)
{
    SvcMappingSpec *spec;

    if (!req || !svc_mapping) {
        return;
    }
    spec = req->add_request();
    spec->mutable_key()->set_vpcid(svc_mapping->key.vpc.id);
    ipaddr_api_spec_to_proto_spec(spec->mutable_key()->mutable_ipaddr(),
                                  &svc_mapping->key.vip);
    spec->mutable_key()->set_svcport(svc_mapping->key.svc_port);
    spec->set_vpcid(svc_mapping->vpc.id);
    ipaddr_api_spec_to_proto_spec(spec->mutable_privateip(),
                                  &svc_mapping->backend_ip);
    spec->set_port(svc_mapping->svc_port);
    ipaddr_api_spec_to_proto_spec(spec->mutable_providerip(),
                                  &svc_mapping->backend_provider_ip);

    return;
}

static void
populate_batch_spec (BatchSpec *spec, pds_batch_params_t *batch)
{
    if (!batch || !spec) {
        return;
    }
    spec->set_epoch(batch->epoch);
    return;
}

#endif    // __TEST_APP_UTILS_HPP__
