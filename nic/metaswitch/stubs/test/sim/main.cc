//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This is the main entry point for the SIM that runs Metasswitch stack with the
//  pds stub integration code and mocks the PDS HAL
//----------------------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/if.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_init.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/bgp_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/evpn_gen.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/metaswitch/stubs/test/hals/test_params.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_util.hpp"
#include "nic/metaswitch/stubs/pds_ms_stubs_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_subnet.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_vpc.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_interface.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_bgp_utils_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pds_ms_evpn_utils_gen.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/metaswitch/stubs/test/common/test_config.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_mai.hpp"

namespace pds_ms_test {
test_params_t* test_params() {
    static test_params_t  g_test_params;
    return &g_test_params;
}

} // End namespace pds_ms_test
using namespace pds_ms_test;
using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using namespace pds_ms;

static sdk::lib::thread *g_routing_thread;
std::string g_grpc_server_addr;
#define GRPC_API_PORT   50057

static int g_vpc_uuid = 10;
static int g_underlay_rttbl_uuid = 30;
static int g_underlay_vpc_uuid = 100;
static int g_overlay_rttbl_uuid = 130;
static int g_bgp_uuid = 50;
static int g_subnet_uuid = 300;
static int g_l3_if_uuid = 400;
static int g_lo_if_uuid = 401;

namespace pds_ms_test {
static test_config_t  g_test_conf;

static NBB_VOID
pds_ms_sim_test_loopback ()
{
    // Loopback interface
    pds_if_spec_t lo_if_spec = {0};
    lo_if_spec.type = PDS_IF_TYPE_LOOPBACK;
    lo_if_spec.key = msidx2pdsobjkey(g_lo_if_uuid);
    lo_if_spec.loopback_if_info.ip_prefix.len = 32;
    lo_if_spec.loopback_if_info.ip_prefix.addr.addr.v4_addr = g_test_conf.local_lo_ip_addr;
    lo_if_spec.loopback_if_info.ip_prefix.addr.af = IP_AF_IPV4;
    pds_ms::interface_create (&lo_if_spec, 0);
}

static NBB_VOID
pds_ms_sim_test_bgp_update ()
{
    // Start CTM
    PDS_MS_START_TXN (PDS_MS_CTM_GRPC_CORRELATOR);

    // BGP Global Spec
    pds::BGPGlobalSpec bgp_global_spec;
    bgp_global_spec.set_localasn (g_test_conf.local_asn);
    bgp_global_spec.set_id (msidx2pdsobjkey(g_bgp_uuid).id, PDS_MAX_KEY_LEN);
    // Router ID should be in host order
    bgp_global_spec.set_routerid (ntohl(g_test_conf.local_lo_ip_addr));
    pds_ms_set_amb_bgp_rm_ent (bgp_global_spec, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);

    //BGP PeerTable
    pds::BGPPeerSpec bgp_peer_spec;
    bgp_peer_spec.set_remoteasn (g_test_conf.remote_asn);
    bgp_peer_spec.set_id(msidx2pdsobjkey(g_bgp_uuid).id, PDS_MAX_KEY_LEN);
    bgp_peer_spec.set_adminen(pds::ADMIN_UP);

    auto peeraddr = bgp_peer_spec.mutable_peeraddr();
    peeraddr->set_af(types::IP_AF_INET);
    peeraddr->set_v4addr(g_test_conf.remote_ip_addr);

    auto localaddr = bgp_peer_spec.mutable_localaddr();
    localaddr->set_af(types::IP_AF_INET);
    localaddr->set_v4addr(0);
    bgp_peer_spec.set_ifid(0);

    bgp_peer_spec.set_connectretry(10);
    bgp_peer_spec.set_sendcomm(pds::BOOL_TRUE);
    bgp_peer_spec.set_sendextcomm(pds::BOOL_TRUE);
    bgp_peer_spec.set_password ("test");
    pds_ms_pre_set_amb_bgp_peer(bgp_peer_spec, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);
    pds_ms_set_amb_bgp_peer (bgp_peer_spec, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);

    pds::BGPPeerAfSpec bgp_peer_af;
    bgp_peer_af.set_id(msidx2pdsobjkey(g_bgp_uuid).id, PDS_MAX_KEY_LEN);

    peeraddr = bgp_peer_af.mutable_peeraddr();
    peeraddr->set_af(types::IP_AF_INET);
    peeraddr->set_v4addr(g_test_conf.remote_ip_addr);

    localaddr = bgp_peer_af.mutable_localaddr();
    localaddr->set_af(types::IP_AF_INET);
    localaddr->set_v4addr(0);
    bgp_peer_af.set_ifid(0);
    bgp_peer_af.set_afi(pds::BGP_AFI_L2VPN);
    bgp_peer_af.set_safi(pds::BGP_SAFI_EVPN);
    bgp_peer_af.set_disable(pds::BOOL_TRUE);
    bgp_peer_af.set_nhself(pds::BOOL_FALSE);
    bgp_peer_af.set_defaultorig(pds::BOOL_FALSE);

    pds_ms_pre_set_amb_bgp_peer_afi_safi(bgp_peer_af, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);
    pds_ms_set_amb_bgp_peer_afi_safi(bgp_peer_af, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);

    // End CTM transaction
    PDS_MS_END_TXN (PDS_MS_CTM_GRPC_CORRELATOR);

    // Wait for MS response
    pds_ms::mgmt_state_t::ms_response_wait();
}

static NBB_VOID
pds_ms_sim_test_overlay_bgp_update ()
{
    // Start CTM
    PDS_MS_START_TXN (PDS_MS_CTM_GRPC_CORRELATOR);

    //BGP PeerTable
    pds::BGPPeerSpec bgp_peer_spec;
    bgp_peer_spec.set_remoteasn (g_test_conf.remote_asn);
    bgp_peer_spec.set_id(msidx2pdsobjkey(g_bgp_uuid).id, PDS_MAX_KEY_LEN);
    bgp_peer_spec.set_adminen(pds::ADMIN_UP);

    auto peeraddr = bgp_peer_spec.mutable_peeraddr();
    peeraddr->set_af(types::IP_AF_INET);
    peeraddr->set_v4addr(g_test_conf.remote_lo_ip_addr);

    auto localaddr = bgp_peer_spec.mutable_localaddr();
    localaddr->set_af(types::IP_AF_INET);
    localaddr->set_v4addr(g_test_conf.local_lo_ip_addr);
    bgp_peer_spec.set_ifid(0);

    bgp_peer_spec.set_connectretry(10);
    bgp_peer_spec.set_sendcomm(pds::BOOL_TRUE);
    bgp_peer_spec.set_sendextcomm(pds::BOOL_TRUE);
    bgp_peer_spec.set_password ("test");
    pds_ms_pre_set_amb_bgp_peer(bgp_peer_spec, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);
    pds_ms_set_amb_bgp_peer(bgp_peer_spec, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);

    pds::BGPPeerAfSpec bgp_peer_af;
    bgp_peer_af.set_id(msidx2pdsobjkey(g_bgp_uuid).id, PDS_MAX_KEY_LEN);

    peeraddr = bgp_peer_af.mutable_peeraddr();
    peeraddr->set_af(types::IP_AF_INET);
    peeraddr->set_v4addr(g_test_conf.remote_lo_ip_addr);

    localaddr = bgp_peer_af.mutable_localaddr();
    localaddr->set_af(types::IP_AF_INET);
    localaddr->set_v4addr(g_test_conf.local_lo_ip_addr);
    bgp_peer_af.set_ifid(0);
    bgp_peer_af.set_afi(pds::BGP_AFI_IPV4);
    bgp_peer_af.set_safi(pds::BGP_SAFI_UNICAST);
    bgp_peer_af.set_disable(pds::BOOL_TRUE);
    bgp_peer_af.set_nhself(pds::BOOL_FALSE);
    bgp_peer_af.set_defaultorig(pds::BOOL_FALSE);

    pds_ms_pre_set_amb_bgp_peer_afi_safi(bgp_peer_af, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);
    pds_ms_set_amb_bgp_peer_afi_safi(bgp_peer_af, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);

    // End CTM transaction
    PDS_MS_END_TXN (PDS_MS_CTM_GRPC_CORRELATOR);

    // Wait for MS response
    pds_ms::mgmt_state_t::ms_response_wait();
}

static NBB_VOID
pds_ms_sim_test_evpn_evi_update ()
{
    // Start CTM
    PDS_MS_START_TXN (PDS_MS_CTM_GRPC_CORRELATOR);

    // EvpnEviTable
    pds::EvpnEviSpec evpn_evi_spec;
    evpn_evi_spec.set_id (msidx2pdsobjkey(g_subnet_uuid).id, PDS_MAX_KEY_LEN); // evi rt UUID is same as subnet UUID
    evpn_evi_spec.set_subnetid (msidx2pdsobjkey(g_subnet_uuid).id, PDS_MAX_KEY_LEN);
    evpn_evi_spec.set_autord(pds::EVPN_CFG_AUTO);
    evpn_evi_spec.set_autort(pds::EVPN_CFG_AUTO);
    evpn_evi_spec.set_rttype(pds::EVPN_RT_IMPORT_EXPORT);
    evpn_evi_spec.set_encap(pds::EVPN_ENCAP_VXLAN);
    pds_ms_pre_set_amb_evpn_evi (evpn_evi_spec, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);
    pds_ms_set_amb_evpn_evi (evpn_evi_spec, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);
    // End CTM transaction
    PDS_MS_END_TXN (PDS_MS_CTM_GRPC_CORRELATOR);

    // Wait for MS response
    pds_ms::mgmt_state_t::ms_response_wait();
}

static NBB_VOID
pds_ms_sim_test_config ()
{
    cout << "Config thread is waiting for Nbase....\n";
    while (!g_routing_thread->ready()) {
         pthread_yield();
    }
    cout << "Nbase is ready!\n";

    // Underlay VPC update
    pds_vpc_spec_t u_vpc_spec = {0};
    u_vpc_spec.key = msidx2pdsobjkey(g_underlay_vpc_uuid);
    u_vpc_spec.v4_route_table = msidx2pdsobjkey (g_underlay_rttbl_uuid);
    u_vpc_spec.type = PDS_VPC_TYPE_UNDERLAY;
    pds_ms::vpc_create (&u_vpc_spec, 0);
    cout << "Config thread: VPC Proto is done!\n";

    // Create L3 interface
    pds_if_spec_t l3_if_spec = {0};
    l3_if_spec.type = PDS_IF_TYPE_L3;
    l3_if_spec.key = msidx2pdsobjkey(g_l3_if_uuid);
    l3_if_spec.l3_if_info.ip_prefix.len = 24;
    l3_if_spec.l3_if_info.eth_ifindex = g_test_conf.eth_if_index;
    l3_if_spec.l3_if_info.ip_prefix.addr.addr.v4_addr = g_test_conf.local_ip_addr;
    l3_if_spec.l3_if_info.ip_prefix.addr.af = IP_AF_IPV4;
    pds_ms::interface_create (&l3_if_spec, 0);
    cout << "Config thread: L3 Interface Config is done!\n";

    // Loopback Update
    pds_ms_sim_test_loopback();
    cout << "Config thread: Loopback Proto is done!\n";

    // BGP Update
    pds_ms_sim_test_bgp_update();
    cout << "Config thread: BGP Proto is done!\n";
    cout << "Config thread: Waiting for BGP underlay convergence before configuring overlay!\n";

    sleep(5);

    // BGP Overlay Update
    pds_ms_sim_test_overlay_bgp_update();
    cout << "Config thread: BGP Proto is done!\n";

    // VPC update
    pds_vpc_spec_t vpc_spec = {0};
    vpc_spec.key = msidx2pdsobjkey(g_vpc_uuid);
    vpc_spec.v4_route_table = msidx2pdsobjkey (g_overlay_rttbl_uuid);
    u_vpc_spec.type = PDS_VPC_TYPE_TENANT;
    vpc_spec.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
    vpc_spec.fabric_encap.val.vnid = g_test_conf.vni;
    pds_ms::vpc_create (&vpc_spec, 0);
    cout << "Config thread: VPC Proto is done!\n";

    // Start CTM
    PDS_MS_START_TXN (PDS_MS_CTM_GRPC_CORRELATOR);

    pds::EvpnIpVrfSpec evpn_ip_vrf_spec;
    evpn_ip_vrf_spec.set_id (msidx2pdsobjkey(g_vpc_uuid).id, PDS_MAX_KEY_LEN);
    evpn_ip_vrf_spec.set_vpcid (msidx2pdsobjkey(g_vpc_uuid).id, PDS_MAX_KEY_LEN);
    evpn_ip_vrf_spec.set_vni(200);
    NBB_BYTE rd[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    evpn_ip_vrf_spec.set_rd((char*)rd,8);
    pds_ms_set_amb_evpn_ip_vrf (evpn_ip_vrf_spec, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);

    pds::EvpnIpVrfRtSpec evpn_ip_vrf_rt_spec;
    evpn_ip_vrf_rt_spec.set_id (msidx2pdsobjkey(g_vpc_uuid).id, PDS_MAX_KEY_LEN);
    evpn_ip_vrf_rt_spec.set_vpcid (msidx2pdsobjkey(g_vpc_uuid).id, PDS_MAX_KEY_LEN);
    NBB_BYTE rt[] = {0x00,0x02,0x00,0x00,0x00,0x00,0x00,0xc8};
    evpn_ip_vrf_rt_spec.set_rt(rt,8);
    evpn_ip_vrf_rt_spec.set_rttype(pds::EVPN_RT_IMPORT_EXPORT);
    pds_ms_set_amb_evpn_ip_vrf_rt (evpn_ip_vrf_rt_spec, AMB_ROW_ACTIVE, PDS_MS_CTM_GRPC_CORRELATOR);

    // End CTM transaction
    PDS_MS_END_TXN (PDS_MS_CTM_GRPC_CORRELATOR);

    // Wait for MS response
    pds_ms::mgmt_state_t::ms_response_wait();
    cout << "Config thread: EVPN IP VRF & RT Proto is done!\n";

    // Subnet update
    pds_subnet_spec_t subnet_spec = {0};
    subnet_spec.key = pds_ms::msidx2pdsobjkey(g_subnet_uuid);
    subnet_spec.vpc = pds_ms::msidx2pdsobjkey(g_vpc_uuid);
    subnet_spec.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
    subnet_spec.fabric_encap.val.vnid = g_test_conf.vni;
    subnet_spec.host_if = api::uuid_from_objid(g_test_conf.lif_if_index);
    subnet_spec.v4_prefix.len = 24;
    subnet_spec.v4_prefix.v4_addr = g_test_conf.local_gwip_addr;
    pds_ms::subnet_create (&subnet_spec, 0);
    cout << "Config thread: Subnet Proto is done!\n";

    // Evpn Evi Update
    pds_ms_sim_test_evpn_evi_update();

    sleep(2);

    // Simulate MAC IP learn
    if (g_node_id == 1) {
        std::cout << " Simulate MAC and IP learn on Node " << g_node_id << std::endl;
        ip_addr_t   ip;
        mac_addr_t  mac;
        str2ipaddr((char*) "10.17.0.1", &ip);
        mac_str_to_addr((char*) "00:11:11:11:11:22", mac);
        pds_ms::l2f_local_mac_ip_add (pds_ms::msidx2pdsobjkey(g_subnet_uuid), ip, mac, g_test_conf.lif_if_index);
        str2ipaddr((char*) "10.17.0.2", &ip);
        mac_str_to_addr((char*) "00:11:11:11:11:22", mac);
        pds_ms::l2f_local_mac_ip_add (pds_ms::msidx2pdsobjkey(g_subnet_uuid), ip, mac, g_test_conf.lif_if_index);
        str2ipaddr((char*) "10.17.0.5", &ip);
        mac_str_to_addr((char*) "00:11:11:11:11:22", mac);
        pds_ms::l2f_local_mac_ip_add (pds_ms::msidx2pdsobjkey(g_subnet_uuid), ip, mac, g_test_conf.lif_if_index);
    }
    sleep(10);
    if (g_node_id == 2) {
        std::cout << " Simulate MAC move to Node " << g_node_id << std::endl;
        ip_addr_t   ip;
        str2ipaddr((char*) "0.0.0.0", &ip);
        mac_addr_t  mac;
        mac_str_to_addr((char*) "00:11:11:11:11:22", mac);
        pds_ms::l2f_local_mac_ip_add (pds_ms::msidx2pdsobjkey(g_subnet_uuid), ip, mac, g_test_conf.lif_if_index);
    }
    sleep(10);
    if (g_node_id == 2) {
        std::cout << " Simulate MAC age on Node " << g_node_id << std::endl;
        ip_addr_t   ip;
        str2ipaddr((char*) "0.0.0.0", &ip);
        mac_addr_t  mac;
        mac_str_to_addr((char*) "00:11:11:11:11:22", mac);
        pds_ms::l2f_local_mac_ip_del (pds_ms::msidx2pdsobjkey(g_subnet_uuid), ip, mac);
    }
}
} // End of pds_ms_test  namespace

static void
svc_reg (void)
{
    ServerBuilder         *server_builder;
    BGPSvcImpl            bgp_svc;
    EvpnSvcImpl           evpn_svc;

    grpc_init();
    g_grpc_server_addr =
        std::string("0.0.0.0:") + std::to_string(GRPC_API_PORT);
    server_builder = new ServerBuilder();
    server_builder->SetMaxReceiveMessageSize(INT_MAX);
    server_builder->SetMaxSendMessageSize(INT_MAX);
    server_builder->AddListeningPort(g_grpc_server_addr,
                                     grpc::InsecureServerCredentials());

    server_builder->RegisterService(&bgp_svc);
    server_builder->RegisterService(&evpn_svc);

    std::unique_ptr<Server> server(server_builder->BuildAndStart());
    server->Wait();
}

int
main (int argc, char **argv)
{
    // Call the mock pds init
    pds_init(nullptr);

    // parse json config file
    if (parse_json_config(&g_test_conf, 1) < 0) {
        cout << "Config file not found! Check CONFIG_PATH env var\n";
        exit(1);
    }
    // This will start nbase
    g_routing_thread =
        sdk::lib::thread::factory(
            "routing", 0, sdk::lib::THREAD_ROLE_CONTROL,
            0x0, &pds_ms::pds_ms_thread_init,
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            false);
    SDK_ASSERT_TRACE_RETURN((g_routing_thread != NULL), SDK_RET_ERR,
                            "Routing thread create failure");
    g_routing_thread->start(g_routing_thread);
    // Push the test config, this will wait for nbase init to complete
    pds_ms_test::pds_ms_sim_test_config();
    svc_reg();
    return 0;
}
