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
#include "gen/proto/subnet.grpc.pb.h"
#include "gen/proto/device.grpc.pb.h"
#include "gen/proto/tunnel.grpc.pb.h"
#include "gen/proto/vnic.grpc.pb.h"
#include "gen/proto/types.grpc.pb.h"
#include "nic/apollo/api/include/pds_tep.hpp"
#include "nic/apollo/api/include/pds_vcn.hpp"
#include "nic/apollo/api/include/pds_subnet.hpp"
#include "nic/apollo/api/include/pds_vnic.hpp"
#include "nic/apollo/api/include/pds_mapping.hpp"
#include "nic/apollo/api/include/pds_policy.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/api/include/pds_route.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"

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

//----------------------------------------------------------------------------
// convert HAL IP address to spec
//----------------------------------------------------------------------------
static void
ip_addr_to_spec (types::IPAddress *ip_addr_spec,
                 ip_addr_t *ip_addr)
{
    if (ip_addr->af == IP_AF_IPV4) {
        ip_addr_spec->set_af(types::IP_AF_INET);
        ip_addr_spec->set_v4addr(ip_addr->addr.v4_addr);
    } else if (ip_addr->af == IP_AF_IPV6) {
        ip_addr_spec->set_af(types::IP_AF_INET6);
        ip_addr_spec->set_v6addr(ip_addr->addr.v6_addr.addr8, IP6_ADDR8_LEN);
    }
}

//----------------------------------------------------------------------------
// convert IP prefix to IPPrefix proto spec
//----------------------------------------------------------------------------
static void
ip_pfx_to_spec (types::IPPrefix *ip_pfx_spec,
                ip_prefix_t *ip_pfx)
{
    ip_pfx_spec->set_len(ip_pfx->len);
    ip_addr_to_spec(ip_pfx_spec->mutable_addr(), &ip_pfx->addr);
}

//----------------------------------------------------------------------------
// convert HAL IPv4 address to spec
//----------------------------------------------------------------------------
static void
ipv4_addr_to_spec (types::IPAddress *ip_addr_spec,
                   ipv4_addr_t *ipv4_addr)
{
    ip_addr_spec->set_af(types::IP_AF_INET);
    ip_addr_spec->set_v4addr(*ipv4_addr);
}

//----------------------------------------------------------------------------
// convert IPv4 prefix to IPPrefix proto spec
//----------------------------------------------------------------------------
static void
ipv4_pfx_to_spec (types::IPPrefix *ip_pfx_spec,
                ipv4_prefix_t *ip_pfx)
{
    ip_pfx_spec->set_len(ip_pfx->len);
    ipv4_addr_to_spec(ip_pfx_spec->mutable_addr(), &ip_pfx->v4_addr);
}

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
    if (!rt || !req)
        return;

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
        if (rt->routes[i].nh_type == PDS_NH_TYPE_PEER_VCN) {
            route->set_vpcid(rt->routes[i].vcn.id);
        } else if (rt->routes[i].nh_type == PDS_NH_TYPE_TEP) {
            ip_addr_to_spec(route->mutable_nexthop(), &rt->routes[i].nh_ip);
        }
    }

    return;
}

static void
populate_mapping_request (MappingRequest *req, pds_mapping_spec_t *mapping)
{
    if (!mapping || !req)
        return;

    MappingSpec *spec = req->add_request();
    spec->mutable_id()->set_vpcid(mapping->key.vcn.id);
    ip_addr_to_spec(spec->mutable_id()->mutable_ipaddr(),
                    &mapping->key.ip_addr);
    spec->set_subnetid(mapping->subnet.id);
    spec->set_tunnelid(mapping->tep.ip_addr);
    spec->set_macaddr(MAC_TO_UINT64(mapping->overlay_mac));
    pds_encap_to_proto_encap(spec->mutable_encap(), &mapping->fabric_encap);
    spec->set_vnicid(mapping->vnic.id);
    if (mapping->public_ip_valid) {
        ip_addr_to_spec(spec->mutable_publicip(), &mapping->public_ip);
    }
    return;
}

static void
populate_vnic_request (VnicRequest *req, pds_vnic_spec_t *vnic)
{
    if (!vnic || !req)
        return;

    VnicSpec *spec = req->add_request();
    spec->set_vnicid(vnic->vcn.id);
    spec->set_subnetid(vnic->subnet.id);
    spec->set_vpcid(vnic->key.id);
    spec->set_wirevlan(vnic->wire_vlan);
    spec->set_macaddress(MAC_TO_UINT64(vnic->mac_addr));
    spec->set_resourcepoolid(vnic->rsc_pool_id);
    spec->set_sourceguardenable(vnic->src_dst_check);
    pds_encap_to_proto_encap(spec->mutable_encap(), &vnic->fabric_encap);
    return;
}

static void
populate_subnet_request (SubnetRequest *req, pds_subnet_spec_t *subnet)
{
    if (!subnet || !req)
        return;

    SubnetSpec *spec = req->add_request();
    ipv4_pfx_to_spec(spec->mutable_v4prefix(), &subnet->v4_pfx);
    ip_pfx_to_spec(spec->mutable_v6prefix(), &subnet->v6_pfx);
    spec->set_ipv4virtualrouterip(subnet->v4_vr_ip);
    spec->set_ipv6virtualrouterip(subnet->v6_vr_ip.addr.v6_addr.addr8, IP6_ADDR8_LEN);
    spec->set_id(subnet->key.id);
    spec->set_vpcid(subnet->vcn.id);
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
populate_vpc_request (VPCRequest *req, pds_vcn_spec_t *vcn)
{
    if (!vcn || !req)
        return;

    VPCSpec *spec = req->add_request();
    ipv4_pfx_to_spec(spec->mutable_v4prefix(), &vcn->v4_pfx);
    ip_pfx_to_spec(spec->mutable_v6prefix(), &vcn->v6_pfx);
    spec->set_id(vcn->key.id);
    if (vcn->type == PDS_VCN_TYPE_TENANT) {
        spec->set_type(pds::VPC_TYPE_TENANT);
    } else if (vcn->type == PDS_VCN_TYPE_SUBSTRATE) {
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
    auto encap = spec->mutable_encap();

    spec->set_id(tep_id);
    // TODO: Only filling up remote-ip for now
    ipv4_addr_to_spec(spec->mutable_remoteip(), &tep->key.ip_addr);
    switch (tep->type) {
    case PDS_TEP_TYPE_NONE:
        spec->set_type(pds::TUNNEL_TYPE_NONE);
        break;
    case PDS_TEP_TYPE_IGW:
        spec->set_type(pds::TUNNEL_TYPE_IGW);
        break;
    case PDS_TEP_TYPE_WORKLOAD:
        spec->set_type(pds::TUNNEL_TYPE_WORKLOAD);
        break;
    default:
        break;
    }
    switch (tep->encap.type) {
       case PDS_ENCAP_TYPE_VXLAN:
           encap->set_type(types::ENCAP_TYPE_VXLAN);
           encap->mutable_value()->set_vnid(tep->encap.val.vnid);
           break;
       case PDS_ENCAP_TYPE_MPLSoUDP:
           encap->set_type(types::ENCAP_TYPE_MPLSoUDP);
           encap->mutable_value()->set_mplstag(tep->encap.val.mpls_tag);
           break;
       default:
           break;
    }

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
