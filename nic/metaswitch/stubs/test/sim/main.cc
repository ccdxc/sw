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
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_init.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/bgp_gen.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/metaswitch/stubs/test/hals/test_params.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_test_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_config.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_evpn_utils.hpp"
#include "nic/metaswitch/stubs/hals/pds_ms_ifindex.hpp"
#include "nic/metaswitch/stubs/pdsa_stubs_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <thread>

using boost::property_tree::ptree;

namespace pdsa_test {
test_params_t* test_params() {    
    static test_params_t  g_test_params;
    return &g_test_params;
}

} // End namespace pdsa_test

using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

static sdk::lib::thread *g_routing_thread;
std::string g_grpc_server_addr;
#define GRPC_API_PORT   50057

namespace pdsa_stub {


static NBB_VOID
pdsa_sim_test_config (pdsa_config_t& conf)
{
    cout << "Config thread is waiting for Nbase....\n";
    while (!g_routing_thread->ready()) {
         pthread_yield();
    }    
    cout << "Nbase is ready!\n";

    // Create thread context
    NBB_CREATE_THREAD_CONTEXT
    NBB_TRC_ENTRY ("pdsa_sim_test_config");

    /***************************************************************************/
    /* Get the lock for the SHARED LOCAL data.                                 */
    /***************************************************************************/
    NBS_ENTER_SHARED_CONTEXT(sms_our_pid);
    NBS_GET_SHARED_DATA();

    NBB_TRC_FLOW ((NBB_FORMAT "Start CTM Transaction"));
    pdsa_ctm_send_transaction_start (PDSA_CTM_CORRELATOR);

    NBB_TRC_FLOW ((NBB_FORMAT "ROW UPDATES"));

    conf.correlator = PDSA_CTM_CORRELATOR;

    // limInterfaceCfgTable - L3 interface for physical port
    // NetAgent will provide the HAL L3 IfIndex.
    // Mgmt Stub has to convert it to MS IfIndex
    auto ms_ifindex = pdsa_stub::pds_to_ms_ifindex(conf.g_evpn_if_index);
    SDK_TRACE_INFO ("Configuring uplink port %ld L3 IfIndex 0x%lx MS IfIndex 0x%lx",
                    ETH_IFINDEX_TO_PARENT_PORT(conf.g_evpn_if_index), conf.g_evpn_if_index, ms_ifindex);
    pdsa_test_row_update_lim_if_cfg (&conf,
                                     ms_ifindex,
                                     AMB_TRISTATE_TRUE,
                                     AMB_TRISTATE_TRUE,
                                     AMB_TRISTATE_TRUE,
                                     AMB_TRISTATE_TRUE,
                                     AMB_LIM_FWD_MODE_L3);
    // limL3InterfaceAddressTable
    pdsa_test_row_update_lim_if_addr (&conf);

    // limInterfaceCfgTable -NODE_A_AC_IF_INDEX
    pdsa_test_row_update_lim_if_cfg (&conf,
                                     7,
                                     AMB_TRISTATE_TRUE,
                                     AMB_TRISTATE_FALSE,
                                     AMB_TRISTATE_FALSE,
                                     AMB_TRISTATE_FALSE,
                                     AMB_LIM_FWD_MODE_DEFAULT);

    // l2fMacIpCfgTable
    pdsa_test_row_update_l2f_mac_ip_cfg(&conf);

   // bgpRmEntTable
   pdsa_test_row_update_bgp_rm (&conf);

    // bgpRmAfmJoinTable - AMB_BGP_AFI_IPV4
    pdsa_row_update_bgp_rm_afm_join (&conf,
                                     1,
                                     AMB_BGP_AFI_IPV4,
                                     AMB_BGP_UNICAST);

    // bgpRmAfmJoinTable - AMB_BGP_AFI_L2VPN
    pdsa_row_update_bgp_rm_afm_join (&conf,
                                     2,
                                     AMB_BGP_AFI_L2VPN,
                                     AMB_BGP_EVPN);

   // bgpRmAfiSafiTable
   pdsa_test_row_update_bgp_rm_afi_safi (&conf);

    // bgpNmListenTable
    pdsa_test_row_update_bgp_nm_listen (&conf);

    // bgpPeerTable
    pdsa_test_row_update_bgp_peer (&conf);

    // evpnEviTable
    pdsa_row_update_evpn_evi (1, PDS_ENCAP_TYPE_VXLAN, AMB_ROW_ACTIVE, PDSA_CTM_CORRELATOR);

    // evpnBdTable
    pdsa_row_update_evpn_bd (1, 100, AMB_ROW_ACTIVE, PDSA_CTM_CORRELATOR);

    // evpnIfBindCfgTable
    pdsa_row_update_evpn_if_bind_cfg (1, 7, AMB_ROW_ACTIVE, PDSA_CTM_CORRELATOR);

    NBB_TRC_FLOW ((NBB_FORMAT "End CTM Transaction"));
    pdsa_ctm_send_transaction_end (PDSA_CTM_CORRELATOR);

    /***************************************************************************/
    /* Release the lock on the SHARED LOCAL data.                              */
    /***************************************************************************/
    NBS_RELEASE_SHARED_DATA();
    NBS_EXIT_SHARED_CONTEXT();

    NBB_TRC_EXIT();
    SDK_TRACE_INFO ("Test Config Completed");

    // Destory the thread context
    NBB_DESTROY_THREAD_CONTEXT;


}

static int
parse_json_config (pdsa_config_t *conf) {
    ptree       pt;
    uint32_t    test_config = 0;
    std::string file, cfg_path, value;

    if (!std::getenv("CONFIG_PATH")) {
        fprintf(stderr, "CONFIG_PATH env var is not set!\n");
        return -1;
    }
    // form the full path to the config directory
    cfg_path = std::string(std::getenv("CONFIG_PATH"));
    if (cfg_path.empty()) {
        cfg_path = std::string("./");
    } else {
        cfg_path += "/";
    }

    // make sure the cfg file exists
    file = cfg_path + "/" + std::string("evpn.json");
    if (access(file.c_str(), R_OK) < 0) {
        fprintf(stderr, "Config file %s doesn't exist or not accessible\n",
                file.c_str());
        return -1;
    }
    std::ifstream json_cfg (file.c_str());
    if (!json_cfg)
    {
        fprintf(stderr, "Config file %s doesn't exist or not accessible\n",
                file.c_str());
        return -1;
    }

    // read config
    read_json (json_cfg, pt);
    try
    {
        // if there is test-config string, read it
        test_config = pt.get <uint32_t>("test-config", 0);
    }
    catch (std::exception const&  ex)
    {
        // Otherwise, test config is not required
        fprintf(stderr, "Config file %s doesn't have test-config enabled!\n",
                file.c_str());
    }

    if (!test_config)
    {
        return -1;
    }

    value           = pt.get <std::string>("local.ip","");
    conf->g_node_a_ip     = inet_network (value.c_str());
    value           = pt.get <std::string>("local.ac-ip","");
    conf->g_node_a_ac_ip  = inet_network (value.c_str());
    value           = pt.get <std::string>("remote.ip","");
    conf->g_node_b_ip     = inet_network (value.c_str());
    value           = pt.get <std::string>("remote.ac-ip","");
    conf->g_node_b_ac_ip  = inet_network (value.c_str());
    value           = pt.get <std::string>("if-index","");
    conf->g_evpn_if_index = strtol (value.c_str(),NULL, 0);

    return 0;
}

} // End of pdsa_stub namespace

static void
svc_reg (void)
{
    ServerBuilder         *server_builder;
    BGPSvcImpl            bgp_svc;

    grpc_init();
    g_grpc_server_addr =
        std::string("0.0.0.0:") + std::to_string(GRPC_API_PORT);
    server_builder = new ServerBuilder();
    server_builder->SetMaxReceiveMessageSize(INT_MAX);
    server_builder->SetMaxSendMessageSize(INT_MAX);
    server_builder->AddListeningPort(g_grpc_server_addr,
                                     grpc::InsecureServerCredentials());

    server_builder->RegisterService(&bgp_svc);

    std::unique_ptr<Server> server(server_builder->BuildAndStart());
    server->Wait();
}

int
main (int argc, char **argv)
{
    // Local variables
    pdsa_stub::pdsa_config_t   conf = {0};

    // Call the mock pds init
    pds_init(nullptr);

    // parse json config file
    if (parse_json_config(&conf) < 0) {
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
    pdsa_stub::pdsa_sim_test_config(conf);
    svc_reg();
}
