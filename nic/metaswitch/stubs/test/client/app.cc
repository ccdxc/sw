//----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//----------------------------------------------------------------------------
#include <iostream>
#include <memory>
#include <string>
#include <grpc++/grpc++.h>
#include <gen/proto/bgp.pb.h>
#include <gen/proto/bgp.grpc.pb.h>
#include <chrono>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/test/common/test_config.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "gen/proto/device.grpc.pb.h"
#include "gen/proto/interface.grpc.pb.h"
#include <gen/proto/vpc.grpc.pb.h>
#include <gen/proto/subnet.grpc.pb.h>
#include <gen/proto/bgp.grpc.pb.h>
#include <gen/proto/evpn.grpc.pb.h>
#include <gen/proto/staticroute.grpc.pb.h>
#include "nic/apollo/agent/svc/specs.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using namespace pds_ms_test;
using namespace std;
using namespace pds;

static test_config_t g_test_conf_;
static unique_ptr<pds::DeviceSvc::Stub> g_device_stub_;
static unique_ptr<pds::IfSvc::Stub>     g_if_stub_;
static unique_ptr<pds::BGPSvc::Stub>    g_bgp_stub_;
static unique_ptr<pds::EvpnSvc::Stub>    g_evpn_stub_;
static unique_ptr<pds::SubnetSvc::Stub> g_subnet_stub_;
static unique_ptr<pds::VPCSvc::Stub>    g_vpc_stub_;
static unique_ptr<pds::StaticRouteSvc::Stub>    g_route_stub_;

static void create_device_proto_grpc () {
    ClientContext   context;
    DeviceRequest   request;
    DeviceResponse  response;
    Status          ret_status;
    pds_device_spec_t device_spec = {0};

    device_spec.overlay_routing_en = TRUE;
    device_spec.device_ip_addr.af  = types::IP_AF_INET;
    device_spec.device_ip_addr.addr.v4_addr = (htonl(g_test_conf_.local_ip_addr));
    device_spec.gateway_ip_addr.af  = types::IP_AF_INET;
    device_spec.gateway_ip_addr.addr.v4_addr = (htonl(g_test_conf_.local_gwip_addr));

    pds_device_api_spec_to_proto (request.mutable_request(), &device_spec);
    ret_status = g_device_stub_->DeviceCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n", 
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
    printf("Sent Device proto\n");
}

static void create_l3_intf_proto_grpc () {
    InterfaceRequest    request;
    InterfaceResponse   response;
    ClientContext       context;
    Status              ret_status;
    pds_if_spec_t       pds_if = {0};

    pds_if.key.id = g_test_conf_.eth_if_index;
    pds_if.type = PDS_IF_TYPE_L3;
    pds_if.admin_state = PDS_IF_STATE_UP;
    pds_if.l3_if_info.vpc.id = 1;
    pds_if.l3_if_info.ip_prefix.addr.af = IP_AF_IPV4;
    pds_if.l3_if_info.ip_prefix.addr.addr.v4_addr = htonl(g_test_conf_.local_ip_addr);
    pds_if.l3_if_info.ip_prefix.len = 16;
    pds_if.l3_if_info.port_num = 0;

    pds_if_api_spec_to_proto (request.add_request(), &pds_if);
    ret_status = g_if_stub_->InterfaceCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n", 
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
    printf("Sent Interface proto\n");

}

static void create_bgp_global_proto_grpc () {
    BGPRequest      request;
    BGPResponse     response;
    ClientContext   context;
    Status          ret_status;

    auto proto_spec = request.add_request();
    proto_spec->set_vrfid (PDSA_BGP_RM_ENT_INDEX);
    proto_spec->set_localasn (g_test_conf_.local_asn);
    proto_spec->set_routerid(g_test_conf_.local_ip_addr);

    ret_status = g_bgp_stub_->BGPGlobalSpecCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n", 
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
    printf("Sent BGP Global proto\n");
}

static void create_evpn_evi_proto_grpc () {
    EvpnEviRequest  request;
    EvpnResponse    response;
    ClientContext   context;
    Status          ret_status;

    auto proto_spec = request.add_request ();
    proto_spec->set_eviid (1);
    proto_spec->set_autord (pds::EVPN_CFG_AUTO);
    proto_spec->set_autort (pds::EVPN_CFG_AUTO);
    proto_spec->set_rttype (pds::EVPN_RT_IMPORT_EXPORT);
    proto_spec->set_encap (pds::EVPN_ENCAP_VXLAN);

    ret_status = g_evpn_stub_->EvpnEviSpecCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n", 
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
    printf("Sent EVPN Evi proto\n");
}

static void create_route_proto_grpc () {
    StaticRouteRequest  request;
    StaticRouteResponse response;
    ClientContext       context;
    Status              ret_status;

    auto proto_spec = request.add_request ();
    auto dest_addr  = proto_spec->mutable_destaddr();
    dest_addr->set_af (types::IP_AF_INET);
    dest_addr->set_v4addr (htonl(g_test_conf_.route_dest_ip));
    proto_spec->set_prefixlen (g_test_conf_.route_prefix_len);
    auto next_hop   = proto_spec->mutable_nexthopaddr();
    next_hop->set_af (types::IP_AF_INET);
    next_hop->set_v4addr (htonl(g_test_conf_.route_nh_ip));
    proto_spec->set_adminstatus (ADMIN_UP);
    proto_spec->set_override (BOOL_TRUE);
    proto_spec->set_admindist (1);
    proto_spec->set_action (STRT_ACTION_FWD);

    ret_status = g_route_stub_->StaticRouteSpecCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n", 
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
    printf("Sent Static Route proto\n");
}

static void create_bgp_peer_proto_grpc () {
    BGPPeerRequest  request;
    BGPResponse     response;
    ClientContext   context;
    Status          ret_status;

    auto proto_spec = request.add_request();
    auto peeraddr = proto_spec->mutable_peeraddr();
    peeraddr->set_af(types::IP_AF_INET);
    peeraddr->set_v4addr(htonl(g_test_conf_.remote_ip_addr));
    proto_spec->set_vrfid(PDSA_BGP_RM_ENT_INDEX);
    proto_spec->set_adminen(pds::ADMIN_UP);
    proto_spec->set_peerport(0);
    auto localaddr = proto_spec->mutable_localaddr();
    localaddr->set_af(types::IP_AF_INET);
    localaddr->set_v4addr(htonl(g_test_conf_.local_ip_addr));
    proto_spec->set_localport(0);
    proto_spec->set_ifid(0);
    proto_spec->set_remoteasn(g_test_conf_.remote_asn);
    proto_spec->set_localasn(g_test_conf_.local_asn);
    proto_spec->set_connectretry(5);
    proto_spec->set_sendcomm(pds::BOOL_TRUE);
    proto_spec->set_sendextcomm(pds::BOOL_TRUE);

    ret_status = g_bgp_stub_->BGPPeerSpecCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n", 
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
    printf("Sent BGP Peer proto\n");
}

static void create_subnet_proto_grpc () {
    SubnetRequest   request;
    SubnetResponse  response;
    ClientContext   context;
    Status          ret_status;

    auto proto_spec = request.add_request();
    proto_spec->set_id(1);
    proto_spec->set_vpcid(1);
    auto proto_encap = proto_spec->mutable_fabricencap();
    proto_encap->set_type(types::ENCAP_TYPE_VXLAN);
    proto_encap->mutable_value()->set_vnid(g_test_conf_.vni);
    proto_spec->set_hostifindex(g_test_conf_.lif_if_index);
    proto_spec->set_ipv4virtualrouterip(htonl(0x01010101));
    proto_spec->set_virtualroutermac((uint64_t)0x001122334455);

    ret_status = g_subnet_stub_->SubnetCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
    printf("Sent Subnet proto\n");
}

static void create_vpc_proto_grpc () {
    VPCRequest      request;
    VPCResponse     response;
    ClientContext   context;
    Status          ret_status;

    auto proto_spec = request.add_request();
    proto_spec->set_id(1);
    proto_spec->set_type(pds::VPC_TYPE_TENANT);
    auto proto_encap = proto_spec->mutable_fabricencap();
    proto_encap->set_type(types::ENCAP_TYPE_VXLAN);
    proto_encap->mutable_value()->set_vnid(g_test_conf_.vni);

    ret_status = g_vpc_stub_->VPCCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n", 
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
    printf("Sent VPC proto\n");
}

int main(int argc, char** argv) 
{
    // parse json config file
    if (parse_json_config(&g_test_conf_, 1) < 0) {
        cout << "Config file not found! Check CONFIG_PATH env var\n";
        exit(1);
    }

    std::shared_ptr<Channel> channel = grpc::CreateChannel("localhost:9999",
                                       grpc::InsecureChannelCredentials());
    g_device_stub_  = DeviceSvc::NewStub (channel);
    g_if_stub_      = IfSvc::NewStub (channel);
    g_bgp_stub_     = BGPSvc::NewStub (channel);
    g_evpn_stub_    = EvpnSvc::NewStub (channel);
    g_vpc_stub_     = VPCSvc::NewStub (channel);
    g_subnet_stub_  = SubnetSvc::NewStub (channel);
    g_route_stub_   = StaticRouteSvc::NewStub (channel);

    // Send protos to grpc server
    create_device_proto_grpc();
    create_l3_intf_proto_grpc();
    create_bgp_global_proto_grpc();
    create_bgp_peer_proto_grpc();
    create_vpc_proto_grpc();
    create_evpn_evi_proto_grpc();
    create_subnet_proto_grpc();
    create_route_proto_grpc();

    return 0;
}
