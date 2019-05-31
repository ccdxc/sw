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
#include "nic/apollo/agent/svc/specs.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientContext;
using pds::BatchSpec;
using pds::BatchStatus;
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

static void
pds_encap_to_proto_encap (types::Encap *encap_spec, pds_encap_t *encap)
{
    switch (encap->type) {
    case PDS_ENCAP_TYPE_NONE:
        encap_spec->set_type(types::ENCAP_TYPE_NONE);
        break;

    case PDS_ENCAP_TYPE_DOT1Q:
        encap_spec->set_type(types::ENCAP_TYPE_DOT1Q);
        encap_spec->mutable_value()->set_vlanid(encap->val.vlan_tag);
        break;

    case PDS_ENCAP_TYPE_QINQ:
        encap_spec->set_type(types::ENCAP_TYPE_QINQ);
        encap_spec->mutable_value()->mutable_qinqtag()->
            set_ctag(encap->val.qinq_tag.c_tag);
        encap_spec->mutable_value()->mutable_qinqtag()->
            set_stag(encap->val.qinq_tag.s_tag);
        break;

    case PDS_ENCAP_TYPE_MPLSoUDP:
        encap_spec->set_type(types::ENCAP_TYPE_MPLSoUDP);
        encap_spec->mutable_value()->set_mplstag(encap->val.mpls_tag);
        break;

    case PDS_ENCAP_TYPE_VXLAN:
        encap_spec->set_type(types::ENCAP_TYPE_VXLAN);
        encap_spec->mutable_value()->set_vnid(encap->val.vnid);
        break;

    default:
        break;
    }
}

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
        ip_pfx_to_spec(route->mutable_prefix(), &rt->routes[i].prefix);
        if (rt->routes[i].nh_type == PDS_NH_TYPE_PEER_VPC) {
            route->set_vpcid(rt->routes[i].vpc.id);
        } else if (rt->routes[i].nh_type == PDS_NH_TYPE_TEP) {
            ip_addr_to_spec(route->mutable_nexthop(), &rt->routes[i].nh_ip);
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
    spec->mutable_id()->set_vpcid(local_spec->key.vpc.id);
    ip_addr_to_spec(spec->mutable_id()->mutable_ipaddr(),
                    &local_spec->key.ip_addr);
    spec->set_subnetid(local_spec->subnet.id);
    // Set tunnel id 0 for local mapping
    spec->set_tunnelid(0);
    spec->set_macaddr(MAC_TO_UINT64(local_spec->vnic_mac));
    pds_encap_to_proto_encap(spec->mutable_encap(), &local_spec->fabric_encap);
    spec->set_vnicid(local_spec->vnic.id);
    if (local_spec->public_ip_valid) {
        ip_addr_to_spec(spec->mutable_publicip(), &local_spec->public_ip);
    }
    if (local_spec->provider_ip_valid) {
        ip_addr_to_spec(spec->mutable_providerip(), &local_spec->provider_ip);
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
    spec->mutable_id()->set_vpcid(remote_spec->key.vpc.id);
    ip_addr_to_spec(spec->mutable_id()->mutable_ipaddr(),
                    &remote_spec->key.ip_addr);
    spec->set_subnetid(remote_spec->subnet.id);
    spec->set_tunnelid(remote_spec->tep.ip_addr.addr.v4_addr);
    spec->set_macaddr(MAC_TO_UINT64(remote_spec->vnic_mac));
    pds_encap_to_proto_encap(spec->mutable_encap(), &remote_spec->fabric_encap);
    return;
}

static void
populate_subnet_request (SubnetRequest *req, pds_subnet_spec_t *subnet)
{
    if (!subnet || !req) {
        return;
    }

    SubnetSpec *spec = req->add_request();
    ipv4_pfx_to_spec(spec->mutable_v4prefix(), &subnet->v4_pfx);
    ip_pfx_to_spec(spec->mutable_v6prefix(), &subnet->v6_pfx);
    spec->set_ipv4virtualrouterip(subnet->v4_vr_ip);
    spec->set_ipv6virtualrouterip(subnet->v6_vr_ip.addr.v6_addr.addr8, IP6_ADDR8_LEN);
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
    ipv4_pfx_to_spec(spec->mutable_v4prefix(), &vpc->v4_pfx);
    ip_pfx_to_spec(spec->mutable_v6prefix(), &vpc->v6_pfx);
    spec->set_id(vpc->key.id);
    if (vpc->type == PDS_VPC_TYPE_TENANT) {
        spec->set_type(pds::VPC_TYPE_TENANT);
    } else if (vpc->type == PDS_VPC_TYPE_SUBSTRATE) {
        spec->set_type(pds::VPC_TYPE_SUBSTRATE);
    }

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
    spec->set_id(tep_id);
    tep_api_spec_to_proto_spec(spec, tep);
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
    ipv4_addr_to_spec(spec->mutable_ipaddr(), &device->device_ip_addr);
    ipv4_addr_to_spec(spec->mutable_gatewayip(), &device->gateway_ip_addr);
    spec->set_macaddr(MAC_TO_UINT64(device->device_mac_addr));
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
populate_batch_spec (BatchSpec *spec, pds_batch_params_t *batch)
{
    if (!batch || !spec) {
        return;
    }
    spec->set_epoch(batch->epoch);
    spec->set_rollback_on_failure(false);
    return;
}

#endif    // __TEST_APP_UTILS_HPP__
