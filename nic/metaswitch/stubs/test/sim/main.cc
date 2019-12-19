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
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_init.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/bgp_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/evpn_gen.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/metaswitch/stubs/test/hals/test_params.hpp"
#include "nic/metaswitch/stubs/common/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/pdsa_stubs_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_utils.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_subnet.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_vpc.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_interface.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pdsa_bgp_utils_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/mgmt/pdsa_evpn_utils_gen.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/metaswitch/stubs/test/common/test_config.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_l2f_mai.hpp"

namespace pdsa_test {
test_params_t* test_params() {    
    static test_params_t  g_test_params;
    return &g_test_params;
}

} // End namespace pdsa_test
using namespace pds_ms_test;
using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

static sdk::lib::thread *g_routing_thread;
std::string g_grpc_server_addr;
#define GRPC_API_PORT   50057

namespace pds_ms_test {

static test_config_t  g_test_conf;
static NBB_VOID
pdsa_sim_test_mac_ip()
{

    ip_addr_t ip_addr;
    auto host_ifindex = pds_ms::pds_to_ms_ifindex (g_test_conf.lif_if_index, IF_TYPE_LIF);

    // Start CTM
    PDSA_START_TXN (PDSA_CTM_GRPC_CORRELATOR);

    pdsa_convert_long_to_pdsa_ipv4_addr (g_test_conf.local_mai_ip, &ip_addr);
    pdsa_test_row_update_l2f_mac_ip_cfg (ip_addr, host_ifindex);

    // End CTM transaction
    PDSA_END_TXN (PDSA_CTM_GRPC_CORRELATOR);

    // Wait for MS response
    pds_ms::mgmt_state_t::ms_response_wait();
}
static NBB_VOID
pdsa_sim_test_bgp_update ()
{
    // Start CTM 
    PDSA_START_TXN (PDSA_CTM_GRPC_CORRELATOR);

    // BGP Global Spec
    pds::BGPGlobalSpec bgp_global_spec;
    bgp_global_spec.set_localasn (g_test_conf.local_asn);
    bgp_global_spec.set_vrfid (PDSA_BGP_RM_ENT_INDEX);
    bgp_global_spec.set_routerid (g_test_conf.local_ip_addr);
    pdsa_set_amb_bgp_rm_ent (bgp_global_spec, AMB_ROW_ACTIVE, PDSA_CTM_GRPC_CORRELATOR);

    //BGP PeerTable
    pds::BGPPeerSpec bgp_peer_spec;
    bgp_peer_spec.set_localasn (g_test_conf.local_asn);
    bgp_peer_spec.set_remoteasn (g_test_conf.remote_asn);
    auto peeraddr = bgp_peer_spec.mutable_peeraddr();
    peeraddr->set_af(types::IP_AF_INET);
    peeraddr->set_v4addr(htonl(g_test_conf.remote_ip_addr));
    bgp_peer_spec.set_vrfid(1);
    bgp_peer_spec.set_adminen(pds::ADMIN_UP);
    bgp_peer_spec.set_peerport(0);
    auto localaddr = bgp_peer_spec.mutable_localaddr();
    localaddr->set_af(types::IP_AF_INET);
    localaddr->set_v4addr(htonl(g_test_conf.local_ip_addr));
    bgp_peer_spec.set_localport(0);
    bgp_peer_spec.set_ifid(0);
    bgp_peer_spec.set_connectretry(10);
    bgp_peer_spec.set_sendcomm(pds::BOOL_TRUE);
    bgp_peer_spec.set_sendextcomm(pds::BOOL_TRUE);
    pdsa_set_amb_bgp_peer (bgp_peer_spec, AMB_ROW_ACTIVE, PDSA_CTM_GRPC_CORRELATOR); 

    // End CTM transaction
    PDSA_END_TXN (PDSA_CTM_GRPC_CORRELATOR);

    // Wait for MS response
    pds_ms::mgmt_state_t::ms_response_wait();
}

static NBB_VOID
pdsa_sim_test_evpn_evi_update ()
{
    // Start CTM
    PDSA_START_TXN (PDSA_CTM_GRPC_CORRELATOR);

    // EvpnEviTable
    pds::EvpnEviSpec evpn_evi_spec;
    evpn_evi_spec.set_eviid (1);
    evpn_evi_spec.set_autord(pds::EVPN_CFG_AUTO);
    evpn_evi_spec.set_autort(pds::EVPN_CFG_AUTO);
    evpn_evi_spec.set_rttype(pds::EVPN_RT_IMPORT_EXPORT);
    evpn_evi_spec.set_encap(pds::EVPN_ENCAP_VXLAN);
    pdsa_set_amb_evpn_evi (evpn_evi_spec, AMB_ROW_ACTIVE, PDSA_CTM_GRPC_CORRELATOR);
    // End CTM transaction
    PDSA_END_TXN (PDSA_CTM_GRPC_CORRELATOR);

    // Wait for MS response
    pds_ms::mgmt_state_t::ms_response_wait();
}

static NBB_VOID
pdsa_sim_test_config ()
{
    cout << "Config thread is waiting for Nbase....\n";
    while (!g_routing_thread->ready()) {
         pthread_yield();
    }    
    cout << "Nbase is ready!\n";

    // Create L3 interface
    pds_if_spec_t l3_if_spec = {0};
    l3_if_spec.l3_if_info.ip_prefix.len = 24;
    l3_if_spec.key.id = g_test_conf.eth_if_index;
    pdsa_convert_long_to_pdsa_ipv4_addr (g_test_conf.local_ip_addr, 
                                         &l3_if_spec.l3_if_info.ip_prefix.addr);
    pds_ms::interface_create (&l3_if_spec, 0);
    cout << "Config thread: L3 Interface Config is done!\n";
    
    // BGP Update
    pdsa_sim_test_bgp_update();
    cout << "Config thread: BGP Proto is done!\n";

    // VPC update
    pds_vpc_spec_t vpc_spec = {0};
    vpc_spec.key.id = 1;
    vpc_spec.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
    vpc_spec.fabric_encap.val.vnid = g_test_conf.vni;
    pds_ms::vpc_create (&vpc_spec, 0);
    cout << "Config thread: VPC Proto is done!\n";

    // Subnet update
    pds_subnet_spec_t subnet_spec = {0};
    subnet_spec.key.id = 1;
    subnet_spec.vpc.id = 1;
    subnet_spec.fabric_encap.type = PDS_ENCAP_TYPE_VXLAN;
    subnet_spec.fabric_encap.val.vnid = g_test_conf.vni;
    subnet_spec.host_ifindex = g_test_conf.lif_if_index;
    pds_ms::subnet_create (&subnet_spec, 0);
    cout << "Config thread: Subnet Proto is done!\n";

    // Evpn Evi Update
    pdsa_sim_test_evpn_evi_update();

    // Push MAC-IP
    pdsa_sim_test_mac_ip();
    cout << "Config thread: pushed a mac-ip entry to l2fMacIpCfgTable\n";
    sleep(40);

    // Simulate MAC IP learn
    if (g_node_id == 1) {
        std::cout << " Simulate MAC and IP learn on Node " << g_node_id << std::endl;
        ip_addr_t   ip;
        mac_addr_t  mac;
        str2ipaddr((char*) "99.0.0.1", &ip);
        mac_str_to_addr((char*) "00:11:11:11:11:22", mac);
        pds_ms::l2f_local_mac_ip_add (1, ip, mac, g_test_conf.lif_if_index);
        str2ipaddr((char*) "99.0.0.2", &ip);
        mac_str_to_addr((char*) "00:11:11:11:11:22", mac);
        pds_ms::l2f_local_mac_ip_add (1, ip, mac, g_test_conf.lif_if_index);
        str2ipaddr((char*) "99.0.0.5", &ip);
        mac_str_to_addr((char*) "00:11:11:11:11:22", mac);
        pds_ms::l2f_local_mac_ip_add (1, ip, mac, g_test_conf.lif_if_index);
    }
    sleep(10);
    if (g_node_id == 2) {
        std::cout << " Simulate MAC move to Node " << g_node_id << std::endl;
        ip_addr_t   ip;
        str2ipaddr((char*) "0.0.0.0", &ip);
        mac_addr_t  mac;
        mac_str_to_addr((char*) "00:11:11:11:11:22", mac);
        pds_ms::l2f_local_mac_ip_add (1, ip, mac, g_test_conf.lif_if_index);
    }
    sleep(10);
    if (g_node_id == 2) {
        std::cout << " Simulate MAC age on Node " << g_node_id << std::endl;
        ip_addr_t   ip;
        str2ipaddr((char*) "0.0.0.0", &ip);
        mac_addr_t  mac;
        mac_str_to_addr((char*) "00:11:11:11:11:22", mac);
        pds_ms::l2f_local_mac_ip_del (1, ip, mac);
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
            0x0, &pdsa_stub::pdsa_thread_init,
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            false);
    SDK_ASSERT_TRACE_RETURN((g_routing_thread != NULL), SDK_RET_ERR,
                            "Routing thread create failure");
    g_routing_thread->start(g_routing_thread);
    // Push the test config, this will wait for nbase init to complete
    pds_ms_test::pdsa_sim_test_config();
    svc_reg();
    return 0;
}
