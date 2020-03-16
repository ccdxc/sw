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
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/test/common/test_config.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/apollo/api/include/pds_if.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/test/base/utils.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "gen/proto/device.grpc.pb.h"
#include "gen/proto/interface.grpc.pb.h"
#include <gen/proto/vpc.grpc.pb.h>
#include <gen/proto/subnet.grpc.pb.h>
#include <gen/proto/bgp.grpc.pb.h>
#include <gen/proto/evpn.grpc.pb.h>
#include <gen/proto/cp_route.grpc.pb.h>
#include <gen/proto/cp_test.grpc.pb.h>
#include "nic/apollo/agent/svc/specs.hpp"

/*****************************************************

        --- EVPN -----  Pegasus  --- EVPN ------
       |                 10.3                  |
       |                                       |
       |       10.1 -----IPv4----- 10.2        |
    (1.1.1.1) PDSA 1              PDSA 2  (2.2.2.2)
              11.1 ------IPv4----- 11.2


*****************************************************/

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using namespace pds_ms_test;
using namespace std;
using namespace pds;
using namespace types;

static test_config_t g_test_conf_;
static unique_ptr<pds::DeviceSvc::Stub> g_device_stub_;
static unique_ptr<pds::IfSvc::Stub>     g_if_stub_;
static unique_ptr<pds::BGPSvc::Stub>    g_bgp_stub_;
static unique_ptr<pds::EvpnSvc::Stub>    g_evpn_stub_;
static unique_ptr<pds::SubnetSvc::Stub> g_subnet_stub_;
static unique_ptr<pds::VPCSvc::Stub>    g_vpc_stub_;
static unique_ptr<pds::CPRouteSvc::Stub> g_route_stub_;
static unique_ptr<pds_ms::CPTestSvc::Stub>  g_cp_test_stub_;

static unique_ptr<pds::DeviceSvc::Stub> g_rr_device_stub_;
static unique_ptr<pds::IfSvc::Stub>     g_rr_if_stub_;
static unique_ptr<pds::BGPSvc::Stub>    g_rr_bgp_stub_;
static unique_ptr<pds::CPRouteSvc::Stub> g_rr_route_stub_;

// Simulate random UUIDs
static constexpr int k_underlay_vpc_id = 10;
static constexpr int k_underlay_rttbl_id = 30;
static constexpr int k_vpc_id = 200;
static constexpr int k_overlay_rttbl_id = 230;
static constexpr int k_subnet_id = 300;
static constexpr int k_bgp_id = 50;
static constexpr int k_l3_if_id  = 400;
static constexpr int k_l3_if_id_2  = 410;
static constexpr int k_lo_if_id  = 401;

static void create_device_proto_grpc () {
    ClientContext   context;
    DeviceRequest   request;
    DeviceResponse  response;
    Status          ret_status;
    pds_device_spec_t device_spec = {0};

    device_spec.overlay_routing_en = TRUE;
    device_spec.device_ip_addr.af  = types::IP_AF_INET;
    device_spec.device_ip_addr.addr.v4_addr = (g_test_conf_.local_lo_ip_addr);
    device_spec.gateway_ip_addr.af          = types::IP_AF_INET;
    device_spec.gateway_ip_addr.addr.v4_addr = g_test_conf_.local_lo_ip_addr;

    pds_device_api_spec_to_proto (request.mutable_request(), &device_spec);

    printf ("Pushing Device Proto...\n");
    if (g_node_id == 3) {
        ret_status = g_rr_device_stub_->DeviceCreate(&context, request, &response);
    } else {
        ret_status = g_device_stub_->DeviceCreate(&context, request, &response);
    }
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void create_intf_proto_grpc (bool lo=false, bool second=false, bool update=false, ipv4_addr_t ipv4 = 0) {
    InterfaceRequest    request;
    InterfaceResponse   response;
    ClientContext       context;
    Status              ret_status;
    pds_if_spec_t       pds_if = {0};

    request.mutable_batchctxt()->set_batchcookie(1);

    if (lo) {
        pds_if.key = pds_ms::msidx2pdsobjkey(k_lo_if_id);
        pds_if.type = PDS_IF_TYPE_LOOPBACK;
        if (g_node_id !=3) {
            pds_if.loopback_if_info.ip_prefix.addr.af = IP_AF_IPV4;
            if (update) {
                pds_if.loopback_if_info.ip_prefix.addr.addr.v4_addr = ipv4;
            } else {
                pds_if.loopback_if_info.ip_prefix.addr.addr.v4_addr = g_test_conf_.local_lo_ip_addr;
            }
            pds_if.loopback_if_info.ip_prefix.len = 32;
        }
    } else {
        if (second) {
            pds_if.key = test::uuid_from_objid(k_l3_if_id_2);
            pds_if.l3_if_info.ip_prefix.addr.addr.v4_addr = g_test_conf_.local_ip_addr_2;
            pds_if.l3_if_info.port = test::uuid_from_objid(g_test_conf_.eth_if_index_2);
        } else {
            pds_if.key = test::uuid_from_objid(k_l3_if_id);
            pds_if.l3_if_info.ip_prefix.addr.addr.v4_addr = g_test_conf_.local_ip_addr;
            pds_if.l3_if_info.port = test::uuid_from_objid(g_test_conf_.eth_if_index);
        }
        pds_if.type = PDS_IF_TYPE_L3;
        pds_if.admin_state = PDS_IF_STATE_UP;
        pds_if.l3_if_info.vpc = pds_ms::msidx2pdsobjkey(k_underlay_vpc_id);
        pds_if.l3_if_info.ip_prefix.addr.af = IP_AF_IPV4;
        pds_if.l3_if_info.ip_prefix.len = 16;
    }

    pds_if_api_spec_to_proto (request.add_request(), &pds_if);

    printf ("Pushing %sInterface Proto...\n", lo?"lo ":"");
    if (g_node_id == 3) {
        ret_status = g_rr_if_stub_->InterfaceCreate(&context, request, &response);
    } else {
        if (update) {
            ret_status = g_if_stub_->InterfaceUpdate(&context, request, &response);
        } else {
            ret_status = g_if_stub_->InterfaceCreate(&context, request, &response);
        }
    }
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void delete_lo_proto_grpc () {
    InterfaceDeleteRequest  request;
    InterfaceDeleteResponse response;
    ClientContext       context;
    Status              ret_status;

    request.mutable_batchctxt()->set_batchcookie(1);
    request.add_id(pds_ms::msidx2pdsobjkey(k_lo_if_id).id, PDS_MAX_KEY_LEN);

    printf ("Pushing Loopback Interface Delete Proto...\n");
    if (g_node_id == 3) {
        ret_status = g_rr_if_stub_->InterfaceDelete(&context, request, &response);
    } else {
        ret_status = g_if_stub_->InterfaceDelete(&context, request, &response);
    }
    if (!ret_status.ok() || (response.apistatus(0) != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus(0));
        exit(1);
    }
}

static void create_bgp_global_proto_grpc () {
    BGPRequest      request;
    BGPResponse     response;
    ClientContext   context;
    Status          ret_status;

    auto proto_spec = request.mutable_request();
    proto_spec->set_id (pds_ms::msidx2pdsobjkey(k_bgp_id).id);
    proto_spec->set_localasn (g_test_conf_.local_asn);
    proto_spec->set_routerid(ntohl(g_test_conf_.local_lo_ip_addr));

    printf ("Pushing BGP Global proto...\n");
    if (g_node_id == 3) {
        ret_status = g_rr_bgp_stub_->BGPCreate(&context, request, &response);
    } else {
        ret_status = g_bgp_stub_->BGPCreate(&context, request, &response);
    }
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void create_evpn_evi_proto_grpc () {
    EvpnEviRequest  request;
    EvpnEviResponse response;
    ClientContext   context;
    Status          ret_status;

    auto proto_spec = request.add_request ();
    proto_spec->set_id (pds_ms::msidx2pdsobjkey(k_subnet_id).id, PDS_MAX_KEY_LEN); // evi UUID is same as subnet UUID
    proto_spec->set_subnetid (pds_ms::msidx2pdsobjkey(k_subnet_id).id, PDS_MAX_KEY_LEN);
    if (g_test_conf_.manual_rd) {
        proto_spec->set_autord (pds::EVPN_CFG_MANUAL);
        proto_spec->set_rd((const char *)g_test_conf_.rd, 8);
    } else {
        proto_spec->set_autord (pds::EVPN_CFG_AUTO);
    }
    if (g_test_conf_.manual_rt) {
        proto_spec->set_autort (pds::EVPN_CFG_MANUAL);
    } else {
        proto_spec->set_autort (pds::EVPN_CFG_AUTO);
    }
    proto_spec->set_rttype (pds::EVPN_RT_IMPORT_EXPORT);

    printf ("Pushing EVPN Evi proto...\n");
    ret_status = g_evpn_stub_->EvpnEviCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void delete_evpn_evi_proto_grpc () {
    EvpnEviDeleteRequest  request;
    EvpnEviDeleteResponse response;
    ClientContext   context;
    Status          ret_status;

    auto keyh = request.add_request ();
    auto proto_spec = keyh->mutable_key();
    proto_spec->set_subnetid (pds_ms::msidx2pdsobjkey(k_subnet_id).id, PDS_MAX_KEY_LEN);

    printf ("Pushing EVPN Evi delete proto...\n");
    ret_status = g_evpn_stub_->EvpnEviDelete(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void create_route_proto_grpc (bool second=false) {
    CPStaticRouteRequest  request;
    CPStaticRouteResponse response;
    ClientContext         context;
    Status                ret_status;

    auto proto_spec = request.add_request ();
    proto_spec->set_routetableid(pds_ms::msidx2pdsobjkey(k_underlay_rttbl_id).id);
    auto dest_addr  = proto_spec->mutable_destaddr();
    dest_addr->set_af (types::IP_AF_INET);
    dest_addr->set_v4addr (0);
    proto_spec->set_prefixlen (0);
    auto next_hop   = proto_spec->mutable_nexthopaddr();
    next_hop->set_af (types::IP_AF_INET);
    if (g_node_id == 3) {
        proto_spec->set_interfaceid (pds_ms::msidx2pdsobjkey(k_lo_if_id).id, PDS_MAX_KEY_LEN);
    } else if (second) {
        next_hop->set_v4addr (g_test_conf_.remote_ip_addr_2);
    } else {
        next_hop->set_v4addr (g_test_conf_.remote_ip_addr);
    }
    proto_spec->set_state (ADMIN_STATE_ENABLE);
    proto_spec->set_override (true);
    proto_spec->set_admindist (250);

    printf ("Pushing Static Route proto...\n");
    if (g_node_id == 3) {
        ret_status = g_rr_route_stub_->CPStaticRouteCreate(&context, request, &response);
    } else {
        ret_status = g_route_stub_->CPStaticRouteCreate(&context, request, &response);
    }
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
    }
}

static void create_evpn_evi_rt_proto_grpc () {
    EvpnEviRtRequest    request;
    EvpnEviRtResponse   response;
    ClientContext       context;
    Status              ret_status;

    auto proto_spec = request.add_request ();
    proto_spec->set_id (pds_ms::msidx2pdsobjkey(k_subnet_id).id, PDS_MAX_KEY_LEN); // evi rt UUID is same as subnet UUID
    proto_spec->set_subnetid (pds_ms::msidx2pdsobjkey(k_subnet_id).id, PDS_MAX_KEY_LEN);
    proto_spec->set_rt((const char *)g_test_conf_.rt[0], 8);
    proto_spec->set_rttype (pds::EVPN_RT_IMPORT_EXPORT);

    printf ("Pushing EVPN Evi RT proto...\n");
    ret_status = g_evpn_stub_->EvpnEviRtCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void delete_evpn_evi_rt_proto_grpc () {
    EvpnEviRtDeleteRequest  request;
    EvpnEviRtDeleteResponse response;
    ClientContext           context;
    Status                  ret_status;

    auto proto_spec = request.add_request ();
    auto key   = proto_spec->mutable_key();
    key->set_subnetid (pds_ms::msidx2pdsobjkey(k_subnet_id).id, PDS_MAX_KEY_LEN); // evi rt UUID is same as subnet UUID
    key->set_rt((const char *)g_test_conf_.rt[0], 8);

    printf ("Pushing EVPN Evi RT Delete proto...\n");
    ret_status = g_evpn_stub_->EvpnEviRtDelete(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void create_l2f_test_mac_ip_proto_grpc (bool second=false) {
    pds_ms::CPL2fTestCreateSpec request;
    pds_ms::CPL2fTestResponse   response;
    ClientContext       context;
    Status              ret_status;

    auto proto_spec = &request;
    proto_spec->set_subnetid (pds_ms::msidx2pdsobjkey(k_subnet_id).id, PDS_MAX_KEY_LEN);
    if (!second) {
        auto ipaddr = proto_spec->mutable_ipaddr();
        ipaddr->set_af(types::IP_AF_INET);
        ipaddr->set_v4addr(g_test_conf_.local_mai_ip[0][0]);
    }
    char mac_addr[] = {0x00,0x12,0x23,0x45,0x67,0x8};
    if (second) {
        mac_addr[1] += 5;
        mac_addr[4] -= 7;
    }
    proto_spec->set_macaddr (mac_addr, 6);
    proto_spec->set_ifid (g_test_conf_.lif_if_index);

    printf ("Simulating EVPN MAC/IP learn...\n");
    ret_status = g_cp_test_stub_->CPL2fTestCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void delete_l2f_test_mac_ip_proto_grpc (bool second=false) {
    pds_ms::CPL2fTestDeleteSpec request;
    pds_ms::CPL2fTestResponse   response;
    ClientContext       context;
    Status              ret_status;

    auto proto_spec = &request;
    proto_spec->set_subnetid (pds_ms::msidx2pdsobjkey(k_subnet_id).id, PDS_MAX_KEY_LEN);
    if (!second) {
        auto ipaddr = proto_spec->mutable_ipaddr();
        ipaddr->set_af(types::IP_AF_INET);
        ipaddr->set_v4addr(g_test_conf_.local_mai_ip[0][0]);
    }
    char mac_addr[] = {0x00,0x12,0x23,0x45,0x67,0x8};
    if (second) {
        mac_addr[1] += 5;
        mac_addr[4] -= 7;
    }
    proto_spec->set_macaddr (mac_addr, 6);

    printf ("Simulating EVPN MAC/IP age...\n");
    ret_status = g_cp_test_stub_->CPL2fTestDelete(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void create_bgp_peer_proto_grpc (bool lo=false, bool second=false) {
    BGPPeerRequest  request;
    BGPPeerResponse response;
    ClientContext   context;
    Status          ret_status;

    auto proto_spec = request.add_request();
    auto peeraddr = proto_spec->mutable_peeraddr();
    peeraddr->set_af(types::IP_AF_INET);
    if (lo) {
        if (second) {
            peeraddr->set_v4addr(g_test_conf_.remote_lo_ip_addr_2);
        } else {
            peeraddr->set_v4addr(g_test_conf_.remote_lo_ip_addr);
        }
    } else if (second) {
        peeraddr->set_v4addr(g_test_conf_.remote_ip_addr_2);
    } else {
        peeraddr->set_v4addr(g_test_conf_.remote_ip_addr);
    }
    proto_spec->set_id(pds_ms::msidx2pdsobjkey(k_bgp_id).id);
    proto_spec->set_state(ADMIN_STATE_ENABLE);
    auto localaddr = proto_spec->mutable_localaddr();
    localaddr->set_af(types::IP_AF_INET);
    if (lo && g_node_id !=3) {
        localaddr->set_v4addr(g_test_conf_.local_lo_ip_addr);
    } else {
        localaddr->set_v4addr(0);
    }
    if (lo && g_node_id == 3) {
        proto_spec->set_rrclient(BGP_PEER_RR_CLIENT);
    }
    proto_spec->set_remoteasn(g_test_conf_.remote_asn);
    proto_spec->set_connectretry(5);
    proto_spec->set_sendcomm(true);
    proto_spec->set_sendextcomm(true);
    proto_spec->set_password("test");
    if (lo) {
    proto_spec->set_keepalive(10);
    proto_spec->set_holdtime(30);
    proto_spec->set_ttl(3);
    } else {
    proto_spec->set_keepalive(3);
    proto_spec->set_holdtime(9);
    }

    printf ("Pushing BGP %s Peer proto...\n", (lo) ? "Overlay" : "Underlay" );
    if (g_node_id == 3) {
        ret_status = g_rr_bgp_stub_->BGPPeerCreate(&context, request, &response);
    } else {
        ret_status = g_bgp_stub_->BGPPeerCreate(&context, request, &response);
    }
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void create_bgp_peer_af_proto_grpc (bool lo=false, bool second=false) {
    BGPPeerAfRequest  request;
    BGPPeerAfResponse response;
    ClientContext   context;
    Status          ret_status;

    auto proto_spec = request.add_request();
    auto peeraddr = proto_spec->mutable_peeraddr();
    peeraddr->set_af(types::IP_AF_INET);

    if (lo) {
        if (second) {
            peeraddr->set_v4addr(g_test_conf_.remote_lo_ip_addr_2);
        } else {
            peeraddr->set_v4addr(g_test_conf_.remote_lo_ip_addr);
        }
    } else if (second) {
        peeraddr->set_v4addr(g_test_conf_.remote_ip_addr_2);
    } else {
        peeraddr->set_v4addr(g_test_conf_.remote_ip_addr);
    }
    proto_spec->set_id(pds_ms::msidx2pdsobjkey(k_bgp_id).id);
    auto localaddr = proto_spec->mutable_localaddr();
    localaddr->set_af(types::IP_AF_INET);
    if (lo && g_node_id != 3) {
        localaddr->set_v4addr(g_test_conf_.local_lo_ip_addr);
    } else {
        localaddr->set_v4addr(0);
    }

    if (!lo) {
        // Enable IP
        proto_spec->set_afi(pds::BGP_AFI_IPV4);
        proto_spec->set_safi(pds::BGP_SAFI_UNICAST);
    } else {
        // Enable EVPN
        proto_spec->set_afi(pds::BGP_AFI_L2VPN);
        proto_spec->set_safi(pds::BGP_SAFI_EVPN);
    }
    if (g_node_id == 2 && !lo) {
        printf ("Enabling default originate in BGP on C2 to DUT for IPv4 AF\n");
        proto_spec->set_defaultorig(true);
    } else {
        proto_spec->set_defaultorig(false);
    }
    proto_spec->set_nexthopself(false);

    printf ("Pushing BGP %s Peer AF proto...\n", (lo) ? "Overlay" : "Underlay" );
    if (g_node_id == 3) {
        ret_status = g_rr_bgp_stub_->BGPPeerAfCreate(&context, request, &response);
    } else {
        ret_status = g_bgp_stub_->BGPPeerAfCreate(&context, request, &response);
    }
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void delete_bgp_peer_proto_grpc (bool lo=false, bool second=false) {
    BGPPeerDeleteRequest  request;
    BGPPeerDeleteResponse response;
    ClientContext         context;
    Status                ret_status;

    auto keyh= request.add_request();
    auto proto_spec = keyh->mutable_key();
    auto peeraddr = proto_spec->mutable_peeraddr();
    peeraddr->set_af(types::IP_AF_INET);
    if (lo) {
        if (second) {
            peeraddr->set_v4addr(g_test_conf_.remote_lo_ip_addr_2);
        } else {
            peeraddr->set_v4addr(g_test_conf_.remote_lo_ip_addr);
        }
    } else if (second) {
        peeraddr->set_v4addr(g_test_conf_.remote_ip_addr_2);
    } else {
        peeraddr->set_v4addr(g_test_conf_.remote_ip_addr);
    }
    auto localaddr = proto_spec->mutable_localaddr();
    localaddr->set_af(types::IP_AF_INET);
    if (lo && g_node_id !=3) {
        localaddr->set_v4addr(g_test_conf_.local_lo_ip_addr);
    } else {
        localaddr->set_v4addr(0);
    }

    printf ("Deleting BGP %s Peer proto...\n", (lo) ? "Overlay" : "Underlay" );
    if (g_node_id == 3) {
        ret_status = g_rr_bgp_stub_->BGPPeerDelete(&context, request, &response);
    } else {
        ret_status = g_bgp_stub_->BGPPeerDelete(&context, request, &response);
    }
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void create_subnet_proto_grpc (bool second=false) {
    SubnetRequest   request;
    SubnetResponse  response;
    ClientContext   context;
    Status          ret_status;

    request.mutable_batchctxt()->set_batchcookie(1);

    auto proto_spec = request.add_request();
    if (second) {
    proto_spec->set_id(pds_ms::msidx2pdsobjkey(k_subnet_id+1).id, PDS_MAX_KEY_LEN);
    } else {
    proto_spec->set_id(pds_ms::msidx2pdsobjkey(k_subnet_id).id, PDS_MAX_KEY_LEN);
    }

    proto_spec->set_vpcid(pds_ms::msidx2pdsobjkey(k_vpc_id).id, PDS_MAX_KEY_LEN);
    auto proto_encap = proto_spec->mutable_fabricencap();
    proto_encap->set_type(types::ENCAP_TYPE_VXLAN);
    auto v4_prefix = proto_spec->mutable_v4prefix();
    v4_prefix->set_len(24);
    if (second) {
    proto_encap->mutable_value()->set_vnid(g_test_conf_.vni[1]);
    proto_spec->set_ipv4virtualrouterip(g_test_conf_.local_gwip_addr[1]);
    v4_prefix->set_addr (g_test_conf_.local_gwip_addr[1]);
    } else {
    proto_encap->mutable_value()->set_vnid(g_test_conf_.vni[0]);
    proto_spec->set_ipv4virtualrouterip(g_test_conf_.local_gwip_addr[0]);
    v4_prefix->set_addr (g_test_conf_.local_gwip_addr[0]);
    }
    if (g_node_id == 2) {
    // TODO: Host IfIndex needs to refer to an actual LIF Index in HAL
    //       Else failure in non-mock PDS mode.
    proto_spec->set_hostif(test::uuid_from_objid(g_test_conf_.lif_if_index).id,
                           PDS_MAX_KEY_LEN);
    }
    proto_spec->set_virtualroutermac((uint64_t)0x001122334455);

    printf ("Pushing Subnet proto...\n");
    ret_status = g_subnet_stub_->SubnetCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void delete_subnet_proto_grpc (bool second=false) {
    SubnetDeleteRequest   request;
    SubnetDeleteResponse  response;
    ClientContext   context;
    Status          ret_status;

    request.mutable_batchctxt()->set_batchcookie(1);

    if (second) {
    request.add_id(pds_ms::msidx2pdsobjkey(k_subnet_id+1).id, PDS_MAX_KEY_LEN);
    } else {
    request.add_id(pds_ms::msidx2pdsobjkey(k_subnet_id).id, PDS_MAX_KEY_LEN);
    }
    printf ("Pushing Subnet Del proto...\n");
    ret_status = g_subnet_stub_->SubnetDelete(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus(0) != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus(0));
        exit(1);
    }
}

static void create_underlay_vpc_proto_grpc () {
    VPCRequest      request;
    VPCResponse     response;
    ClientContext   context;
    Status          ret_status;

    request.mutable_batchctxt()->set_batchcookie(1);

    auto proto_spec = request.add_request();
    proto_spec->set_id(pds_ms::msidx2pdsobjkey(k_underlay_vpc_id).id, PDS_MAX_KEY_LEN);
    pds_obj_key_t rttable_id = {0};
    proto_spec->set_v4routetableid(rttable_id.id, PDS_MAX_KEY_LEN);
    proto_spec->set_type(pds::VPC_TYPE_UNDERLAY);
    auto proto_encap = proto_spec->mutable_fabricencap();
    proto_encap->set_type(types::ENCAP_TYPE_NONE);

    printf ("Pushing Underlay VPC proto...\n");
    ret_status = g_vpc_stub_->VPCCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void create_vpc_proto_grpc () {
    VPCRequest      request;
    VPCResponse     response;
    ClientContext   context;
    Status          ret_status;

    request.mutable_batchctxt()->set_batchcookie(1);

    auto proto_spec = request.add_request();
    proto_spec->set_id(pds_ms::msidx2pdsobjkey(k_vpc_id).id, PDS_MAX_KEY_LEN);
    proto_spec->set_v4routetableid(pds_ms::msidx2pdsobjkey(k_overlay_rttbl_id).id, PDS_MAX_KEY_LEN);
    proto_spec->set_type(pds::VPC_TYPE_TENANT);
    auto proto_encap = proto_spec->mutable_fabricencap();
    proto_encap->set_type(types::ENCAP_TYPE_VXLAN);
    proto_encap->mutable_value()->set_vnid(200);

    printf ("Pushing VPC proto...\n");
    ret_status = g_vpc_stub_->VPCCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void create_evpn_ip_vrf_proto_grpc () {
    EvpnIpVrfRequest request;
    EvpnIpVrfResponse response;
    ClientContext   context;
    Status          ret_status;

    auto proto_spec = request.add_request();
    proto_spec->set_id (pds_ms::msidx2pdsobjkey(k_vpc_id).id, PDS_MAX_KEY_LEN);
    proto_spec->set_vpcid (pds_ms::msidx2pdsobjkey(k_vpc_id).id, PDS_MAX_KEY_LEN);
    proto_spec->set_vni(200);

    printf ("Pushing EVPN IP VRF proto...\n");
    ret_status = g_evpn_stub_->EvpnIpVrfCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void create_evpn_ip_vrf_rt_proto_grpc () {
    EvpnIpVrfRtRequest request;
    EvpnIpVrfRtResponse response;
    ClientContext   context;
    Status          ret_status;

    auto proto_spec = request.add_request();
    proto_spec->set_id (pds_ms::msidx2pdsobjkey(k_vpc_id).id, PDS_MAX_KEY_LEN);
    proto_spec->set_vpcid (pds_ms::msidx2pdsobjkey(k_vpc_id).id, PDS_MAX_KEY_LEN);
    NBB_BYTE rt[] = {0x00,0x02,0x00,0x00,0x00,0x00,0x00,0xc8};
    proto_spec->set_rt(rt,8);
    proto_spec->set_rttype(pds::EVPN_RT_IMPORT_EXPORT);

    printf ("Pushing EVPN IP VRF RT proto...\n");
    ret_status = g_evpn_stub_->EvpnIpVrfRtCreate(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void delete_vpc_proto_grpc () {
    VPCDeleteRequest      request;
    VPCDeleteResponse     response;
    ClientContext   context;
    Status          ret_status;

    request.mutable_batchctxt()->set_batchcookie(1);
    request.add_id(pds_ms::msidx2pdsobjkey(k_vpc_id).id, PDS_MAX_KEY_LEN);

    printf ("Pushing VPC Delete proto...\n");
    ret_status = g_vpc_stub_->VPCDelete(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus(0) != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus(0));
        exit(1);
    }
}

static void delete_evpn_ip_vrf_proto_grpc () {
    EvpnIpVrfDeleteRequest request;
    EvpnIpVrfDeleteResponse response;
    ClientContext   context;
    Status          ret_status;

    auto proto_spec = request.add_request();
    proto_spec->set_id (pds_ms::msidx2pdsobjkey(k_vpc_id).id, PDS_MAX_KEY_LEN);

    printf ("Pushing EVPN IP VRF Delete proto...\n");
    ret_status = g_evpn_stub_->EvpnIpVrfDelete(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void delete_evpn_ip_vrf_rt_proto_grpc () {
    EvpnIpVrfRtDeleteRequest request;
    EvpnIpVrfRtDeleteResponse response;
    ClientContext   context;
    Status          ret_status;

    auto keyh = request.add_request();
    auto proto_spec = keyh->mutable_key();
    proto_spec->set_vpcid (pds_ms::msidx2pdsobjkey(k_vpc_id).id, PDS_MAX_KEY_LEN);
    NBB_BYTE rt[] = {0x00,0x02,0x00,0x00,0x00,0x00,0x00,0xc8};
    proto_spec->set_rt(rt,8);

    printf ("Pushing EVPN IP VRF RT Delete proto...\n");
    ret_status = g_evpn_stub_->EvpnIpVrfRtDelete(&context, request, &response);
    if (!ret_status.ok() || (response.apistatus() != types::API_STATUS_OK)) {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
        exit(1);
    }
}

static void get_peer_status_all() {
    BGPPeerGetRequest       request;
    BGPPeerGetResponse   response;
    ClientContext        context;
    Status               ret_status;

    ret_status = g_bgp_stub_->BGPPeerGet(&context, request, &response);
    if (ret_status.ok()) {
        printf ("No of BGP Peer Status Table Entries: %d\n", response.response_size());
        for (int i=0; i<response.response_size(); i++) {
            auto resp = response.response(i).status();
            printf (" Entry :: %d\n", i+1);
            printf (" ===========\n");
            printf ("  VRF Id               : %d\n", 1); // TODO: how to convert UUID to VrfID.. auto-gen wont support fillFn in get
            printf ("  Status               : %d\n", resp.status());
            printf ("  Previous Status      : %d\n", resp.prevstatus());
            uint8_t ler[2];
            memcpy(ler, resp.lasterrorrcvd().c_str(), 2);
            printf  ("  last-error-rcvd::error      : %d\n", unsigned(ler[0]));;
            printf  ("  last-error-rcvd::suberror   : %d\n", unsigned(ler[1]));;
            uint8_t les[2];
            memcpy(les, resp.lasterrorsent().c_str(), 2);
            printf  ("  last-error-sent::error      : %d\n", unsigned(les[0]));;
            printf  ("  last-error-sent::suberror   : %d\n\n", unsigned(les[1]));;
        }
    } else {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
    }
}

static void get_evpn_mac_ip_all () {
    EvpnMacIpGetRequest request;
    EvpnMacIpGetResponse    response;
    ClientContext           context;
    Status                  ret_status;

    ret_status = g_evpn_stub_->EvpnMacIpGet(&context, request, &response);
    if (ret_status.ok()) {
        printf ("No of EVPN MAC IP Table Entries: %d\n", response.response_size());
        for (int i=0; i<response.response_size(); i++) {
            auto resp = response.response(i).status();
            printf (" Entry :: %d\n", i+1);
            printf (" ===========\n");
            printf ("  Source       : %s\n", (resp.source() == 2) ? "Remote" : "Local");
            printf ("  EVI Id       : %d\n", resp.eviid());
            uint8_t mac[6];
            memcpy(mac, resp.macaddress().c_str(),6);
            printf ("  MAC Address  : 0x%X 0x%X 0x%X 0x%X 0x%X 0x%X\n",
                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            auto ipaddr = resp.ipaddress().v4addr();
            struct in_addr ip_addr;
            ip_addr.s_addr = ipaddr;
            printf ("  IP Address   : %s\n\n", inet_ntoa(ip_addr));
        }
    } else {
        printf("%s failed! ret_status=%d (%s) response.status=%d\n",
                __FUNCTION__, ret_status.error_code(), ret_status.error_message().c_str(),
                response.apistatus());
    }
}

int main(int argc, char** argv)
{
    // parse json config file
    if (parse_json_config(&g_test_conf_, 1) < 0) {
        cout << "Config file not found! Check CONFIG_PATH env var\n";
        exit(1);
    }

    std::shared_ptr<Channel> channel = grpc::CreateChannel("localhost:50054",
            grpc::InsecureChannelCredentials());
    g_device_stub_  = DeviceSvc::NewStub (channel);
    g_if_stub_      = IfSvc::NewStub (channel);
    g_bgp_stub_     = BGPSvc::NewStub (channel);
    g_evpn_stub_    = EvpnSvc::NewStub (channel);
    g_vpc_stub_     = VPCSvc::NewStub (channel);
    g_subnet_stub_  = SubnetSvc::NewStub (channel);
    g_route_stub_   = CPRouteSvc::NewStub (channel);
    g_cp_test_stub_   = pds_ms::CPTestSvc::NewStub (channel);

    // TODO: Change channel port to 50057 when connecting to Pegasus
    std::shared_ptr<Channel> rr_channel = grpc::CreateChannel("localhost:50057",
            grpc::InsecureChannelCredentials());
    g_rr_device_stub_  = DeviceSvc::NewStub (rr_channel);
    g_rr_if_stub_      = IfSvc::NewStub (rr_channel);
    g_rr_bgp_stub_     = BGPSvc::NewStub (rr_channel);
    g_rr_route_stub_   = CPRouteSvc::NewStub (rr_channel);

    if (argc == 1)
    {
        // Send protos to grpc server
        if (g_node_id != 3) {
            create_device_proto_grpc();
            create_underlay_vpc_proto_grpc();
        }
        // Create loopback intf for TEP IP on PDSA
        // Create dummy interface in Pegasus as well to use as Nexthop for Static default route
        create_intf_proto_grpc(true /*loopback*/);
        if (g_node_id == 2) {
            // On C2, Delete the RTM redistribute rule to advertise specific TEP IP to DUT
            // Instead BGP default originate is setup on C2 to simulate default route
            // advertised from ToR to Naples
            auto fp = popen ("python /sw/nic/third-party/metaswitch/code/comn/tools/mibapi/metaswitch/cam/mib.py"
                             " set localhost rtmRedistTable rtmRedistFteIndex=1"
                             " rtmRedistEntryId=1 rtmRedistRowStatus=rowDestroy", "r");
            if (!fp) {
                std::cout << "ERROR deleting RTM Redist rule for loopback on container 2" << std::endl;
            }
        }
        if (g_node_id != 3) {
            create_intf_proto_grpc();
            create_intf_proto_grpc(false, true /* second interface */);
        }
        if (g_node_id == 1) {
            // Simulate the static route installed by NMD
            // with higher Admin Distance
            create_route_proto_grpc();
            create_route_proto_grpc(true);
        }
        create_bgp_global_proto_grpc();
        if (g_node_id != 3) {
            /*no IPv4 BGP sessions for Pegasus */
            create_bgp_peer_proto_grpc();
            create_bgp_peer_af_proto_grpc();
            create_bgp_peer_proto_grpc(false, true /* second peer */);
            create_bgp_peer_af_proto_grpc(false, true);
            sleep(5);
        }
        if (g_node_id == 3) {
            create_route_proto_grpc();
            // No direct Overlay BGP session between DUT1 and C2
            create_bgp_peer_proto_grpc(true /* loopback */);
            create_bgp_peer_af_proto_grpc(true /* loopback */);
        }
        create_bgp_peer_proto_grpc(true, true);
        create_bgp_peer_af_proto_grpc(true, true );
        if (g_node_id != 3) {
        create_vpc_proto_grpc();
        create_evpn_ip_vrf_proto_grpc();
        create_evpn_ip_vrf_rt_proto_grpc();
        create_subnet_proto_grpc();
        create_evpn_evi_proto_grpc();
        if (g_test_conf_.manual_rt) {
            create_evpn_evi_rt_proto_grpc();
        }
        }
        if (g_node_id == 2) {
            sleep(5);
            create_l2f_test_mac_ip_proto_grpc();
            create_l2f_test_mac_ip_proto_grpc(true);
        }
        printf ("Testapp Config Init is successful!\n");
        return 0;
    } else if (argc >= 2) {
        if (!strcmp(argv[1], "peer_status")) {
            get_peer_status_all();
            return 0;
        } else if (!strcmp (argv[1], "evpn_mac_ip")) {
            get_evpn_mac_ip_all();
            return 0;
        } else if (!strcmp(argv[1], "vpc-del")) {
            delete_evpn_ip_vrf_rt_proto_grpc();
            delete_evpn_ip_vrf_proto_grpc();
            delete_vpc_proto_grpc();
            return 0;
        } else if (!strcmp(argv[1], "vpc-create")) {
            create_vpc_proto_grpc();
            create_evpn_ip_vrf_proto_grpc();
            create_evpn_ip_vrf_rt_proto_grpc();
            return 0;
        } else if (!strcmp(argv[1], "subnet-del")) {
            delete_evpn_evi_rt_proto_grpc();
            delete_evpn_evi_proto_grpc();
            delete_subnet_proto_grpc();
            return 0;
        } else if (!strcmp(argv[1], "subnet-create")) {
            create_subnet_proto_grpc();
            create_evpn_evi_proto_grpc();
            if (g_test_conf_.manual_rt) {
                create_evpn_evi_rt_proto_grpc();
            }
            return 0;
        } else if (!strcmp(argv[1], "bgp-upeer-del")) {
            if (argc < 3 || (!strcmp(argv[2], "1"))) {
                // First underlay peer
                delete_bgp_peer_proto_grpc();
            } else {
                // Second underlay peer
                delete_bgp_peer_proto_grpc(false, true);
            }
            return 0;
        } else if (!strcmp(argv[1], "bgp-upeer-create")) {
            printf ("argc = %d argv[2] %s\n", argc, argv[2]);
            if (argc < 3 || (!strcmp(argv[2], "1"))) {
                // First underlay peer
                create_bgp_peer_proto_grpc();
                create_bgp_peer_af_proto_grpc();
            } else {
                // Second underlay peer
                create_bgp_peer_proto_grpc(false, true);
                create_bgp_peer_af_proto_grpc(false, true);
            } 
            return 0;
        } else if (!strcmp(argv[1], "bgp-opeer-del")) {
            // Overlay peer
            delete_bgp_peer_proto_grpc(true);
            return 0;
        } else if (!strcmp(argv[1], "bgp-opeer-create")) {
            // Overlay peer
            create_bgp_peer_proto_grpc(true);
            create_bgp_peer_af_proto_grpc(true);
            return 0;
        } else if (!strcmp(argv[1], "mac-ip-del")) {
            // MAC-IP
            delete_l2f_test_mac_ip_proto_grpc();
            return 0;
        } else if (!strcmp(argv[1], "mac-del")) {
            // MAC only
            delete_l2f_test_mac_ip_proto_grpc(true);
            return 0;
        } else if (!strcmp(argv[1], "mac-create")) {
            // MAC only
            create_l2f_test_mac_ip_proto_grpc(true);
            return 0;
        } else if (!strcmp(argv[1], "lo-delete")) {
            delete_lo_proto_grpc();
            return 0;
        } else if (!strcmp(argv[1], "lo-create")) {
            create_intf_proto_grpc(true);
            return 0;
        } else if (!strcmp(argv[1], "lo-update1")) {
            // Update
            create_intf_proto_grpc(true, false, true, 0x05050505);
            return 0;
        } else if (!strcmp(argv[1], "lo-update2")) {
            // Update
            create_intf_proto_grpc(true, false, true, g_test_conf_.local_lo_ip_addr);
            return 0;
        }
    }

    printf ("Invalid CLI Arguments!  Usage: \n"
            "no arguments : run test init config\n"
            "peer_status  : display Peer Status Table\n"
            "evpn_mac_ip  : display EVPN MAC IP Table\n");
    return 0;

}
