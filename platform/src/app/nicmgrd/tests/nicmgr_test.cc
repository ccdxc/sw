#include <gtest/gtest.h>
#include <stdio.h>

#include "nic/sdk/lib/device/device.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/eth_dev.hpp"
#include "platform/src/lib/nicmgr/include/eth_if.h"
#include "platform/src/lib/nicmgr/include/logger.hpp"
#include <grpc++/grpc++.h>
#include "gen/proto/interface.grpc.pb.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>
#include "nic/hal/test/utils/hal_test_utils.hpp"
// #include "devapi_types.hpp"
// #include "logger.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using intf::Interface;
using intf::InterfaceSpec;
using intf::InterfaceRequestMsg;
using intf::InterfaceResponse;
using intf::InterfaceResponseMsg;
using boost::multiprecision::uint512_t;
using boost::multiprecision::uint128_t;
using namespace std::chrono;

using namespace std;

namespace nicmgr {
// shared_ptr<nicmgr::NicMgrService> g_nicmgr_svc;
}

DeviceManager *devmgr = NULL;
sdk::lib::dev_forwarding_mode_t g_fwd_mode;

void
create_uplinks()
{
    InterfaceSpec           *spec;
    InterfaceRequestMsg     req_msg;
    InterfaceResponseMsg    rsp_msg;
    ClientContext           context;
    Status                  status;
    uint64_t                port_num[3] = {PORT_NUM_1, PORT_NUM_2, PORT_NUM_3};
    int                     num_uplinks = 3;
    uint64_t                if_id[3] = {UPLINK_IF_INDEX1, UPLINK_IF_INDEX2, UPLINK_IF_INDEX3};

    std::string  svc_endpoint;

     if (getenv("HAL_SOCK_PATH")) {
         svc_endpoint = std::string("unix:") + std::getenv("HAL_SOCK_PATH") + "halsock";
     } else if (getenv("HAL_GRPC_PORT")) {
         svc_endpoint = std::string("localhost:") + getenv("HAL_GRPC_PORT");
     } else {
         svc_endpoint = std::string("localhost:50054");
     }
    std::shared_ptr<Channel> channel = grpc::CreateChannel(svc_endpoint,
                                                           grpc::InsecureChannelCredentials());

    NIC_LOG_DEBUG("Waiting for HAL to be ready at: {}", svc_endpoint);
     auto state = channel->GetState(true);
     while (state != GRPC_CHANNEL_READY) {
         // Wait for State change or deadline
         channel->WaitForStateChange(state, gpr_time_from_seconds(1, GPR_TIMESPAN));
         state = channel->GetState(true);
        // cout << "[INFO] Connecting to HAL, channel status = " << channel->GetState(true) << endl;
     }

    std::unique_ptr<Interface::Stub> intf_stub_ = Interface::NewStub(channel);

    for (uint32_t i = 0; i < num_uplinks; i++) {
        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_interface_id(if_id[i]);
        spec->set_type(::intf::IfType::IF_TYPE_UPLINK);
        spec->set_admin_status(::intf::IfStatus::IF_STATUS_UP);
        spec->mutable_if_uplink_info()->set_port_num(port_num[i]);
    }
    status = intf_stub_->InterfaceCreate(&context, req_msg, &rsp_msg);
#if 0
    if (status.ok()) {
        for (uint32_t i = 0; i < num_uplinks; i++) {
            assert(rsp_msg.response(i).api_status() == types::API_STATUS_OK);
            std::cout << "Uplink interface create succeeded, handle = "
                << rsp_msg.response(i).status().if_handle()
                << std::endl;
        }
    } else {
        for (uint32_t i = 0; i < num_uplinks; i++) {
            std::cout << "Uplink interface create failed, error = "
                << rsp_msg.response(i).api_status()
                << std::endl;
        }
    }
#endif
}

static int
sdk_trace_cb (uint32_t mod_id, sdk_trace_level_e trace_level,
              const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    NIC_LOG_DEBUG("{}", logbuf);
    va_end(args);

    return 0;
}

static void
sdk_init (void)
{
    sdk::lib::logger::init(sdk_trace_cb);
}

void
nicmgr_init()
{
    devicemgr_cfg_t cfg;

    utils::logger::init(false);
    sdk_init();

    cfg.platform_type = platform_type_t::PLATFORM_TYPE_SIM;
    cfg.cfg_path = std::string(getenv("HAL_CONFIG_PATH"));
    cfg.device_conf_file = "../nic/conf/device.conf";
    cfg.fwd_mode = sdk::lib::FORWARDING_MODE_NONE;
    cfg.micro_seg_en = false;
    cfg.shm_mgr = NULL;
    cfg.EV_A = NULL;

    devmgr = new DeviceManager(&cfg);
    EXPECT_TRUE(devmgr != NULL);
    devmgr->Init(&cfg);
    devmgr->LoadProfile("../platform/src/app/nicmgrd/etc/eth.json", false);
    devmgr->HalEventHandler(true);
}

class nicmgr_test : public ::testing::Test {
protected:
  nicmgr_test() {
  }

  virtual ~nicmgr_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
      std::cout << "Creating Uplinks ........." << endl;
      create_uplinks();
      nicmgr_init();
  }

};

uint8_t *memrev(uint8_t *block, size_t elnum)
{
    uint8_t *s, *t, tmp;

    for (s = block, t = s + (elnum - 1); s < t; s++, t--) {
        tmp = *s;
        *s = *t;
        *t = tmp;
    }
    return block;
}

#if 0
TEST_F(nicmgr_test, test1)
{
    struct ionic_rx_filter_add_cmd rx_cmd = {0};
    uint64_t mac1;
    // Get eth device
    Eth *eth_dev = (Eth *)devmgr->GetDevice("eth0");
    assert(eth_dev != NULL);

    union ionic_dev_cmd d_cmd;
    union ionic_dev_cmd_comp d_comp;

    // RESET
    d_cmd = {0};
    d_cmd.cmd.opcode = IONIC_CMD_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // LIF_INIT
    struct ionic_lif_init_cmd init_cmd = {0};
    init_cmd.opcode = IONIC_CMD_LIF_INIT;
    memcpy(&d_cmd, &init_cmd, sizeof(init_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    rx_cmd = {0};
    // struct ionic_rx_filter_add_comp rx_comp;
    rx_cmd.opcode = IONIC_CMD_RX_FILTER_ADD;
    rx_cmd.match = IONIC_RX_FILTER_MATCH_VLAN;
    rx_cmd.vlan.vlan = 10;
    // printf("opcode: %d rx_cmd_size: %d\n", d_cmd.cmd.opcode, sizeof(rx_cmd));
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    // memcpy(&d_comp, &rx_comp, sizeof(rx_comp));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    rx_cmd = {0};
    rx_cmd.opcode = IONIC_CMD_RX_FILTER_ADD;
    rx_cmd.match = IONIC_RX_FILTER_MATCH_MAC;
    mac1 = 0x12345678ABCD;
    memcpy(rx_cmd.mac.addr, &mac1, 6);
    memrev(rx_cmd.mac.addr, 6);
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // Set modes
    struct ionic_rx_mode_set_cmd rx_mode_cmd = {0};
    rx_mode_cmd.opcode = IONIC_CMD_RX_MODE_SET;
    rx_mode_cmd.rx_mode = IONIC_RX_MODE_F_BROADCAST;
    memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // Set Promiscuous mode
    rx_mode_cmd = {0};
    rx_mode_cmd.opcode = IONIC_CMD_RX_MODE_SET;
    rx_mode_cmd.rx_mode = IONIC_RX_MODE_F_PROMISC;
    memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // RESET
    d_cmd = {0};
    d_cmd.cmd.opcode = IONIC_CMD_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // LIF_INIT
    init_cmd = {0};
    init_cmd.opcode = IONIC_CMD_LIF_INIT;
    memcpy(&d_cmd, &init_cmd, sizeof(init_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // Mac Filter
    rx_cmd = {0};
    rx_cmd.opcode = IONIC_CMD_RX_FILTER_ADD;
    rx_cmd.match = IONIC_RX_FILTER_MATCH_MAC;
    mac1 = 0x12345678ABCD;
    memcpy(rx_cmd.mac.addr, &mac1, 6);
    memrev(rx_cmd.mac.addr, 6);
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);


    // int num_vlan_filters = 2048;
    int num_vlan_filters = 1;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    for (int i = 1; i < num_vlan_filters; i++) {
        // Vlan Filter
        rx_cmd = {0};
        rx_cmd.opcode = IONIC_CMD_RX_FILTER_ADD;
        rx_cmd.match = IONIC_RX_FILTER_MATCH_VLAN;
        rx_cmd.vlan.vlan = i;
        // printf("opcode: %d rx_cmd_size: %d\n", d_cmd.cmd.opcode, sizeof(rx_cmd));
        memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
        // memcpy(&d_comp, &rx_comp, sizeof(rx_comp));
        eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>( t2 - t1 ).count();
    auto duration_secs = duration_cast<seconds>( t2 - t1 ).count();
    printf("Time taken to install %d vlan filters: %lu us, %lu secs\n",
           num_vlan_filters, duration, duration_secs);

    // Set modes
    rx_mode_cmd = {0};
    rx_mode_cmd.opcode = IONIC_CMD_RX_MODE_SET;
    rx_mode_cmd.rx_mode = IONIC_RX_MODE_F_BROADCAST;
    memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // Set Promiscuous mode
    rx_mode_cmd = {0};
    rx_mode_cmd.opcode = IONIC_CMD_RX_MODE_SET;
    rx_mode_cmd.rx_mode = IONIC_RX_MODE_F_PROMISC;
    memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // RESET
    d_cmd = {0};
    d_cmd.cmd.opcode = IONIC_CMD_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);
}

TEST_F(nicmgr_test, test2)
{
    struct ionic_rx_filter_add_cmd rx_cmd;
    uint64_t mac1;

    // Get eth device
    Eth *eth_dev = (Eth *)devmgr->GetDevice("oob_mnic0");
    assert(eth_dev != NULL);

    union ionic_dev_cmd d_cmd;
    union ionic_dev_cmd_comp d_comp;

    // RESET
    d_cmd.cmd.opcode = IONIC_CMD_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // LIF_INIT
    struct ionic_lif_init_cmd init_cmd = {0};
    init_cmd.opcode = IONIC_CMD_LIF_INIT;
    memcpy(&d_cmd, &init_cmd, sizeof(init_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    rx_cmd = {0};
    // struct ionic_rx_filter_add_comp rx_comp;
    rx_cmd.opcode = IONIC_CMD_RX_FILTER_ADD;
    rx_cmd.match = IONIC_RX_FILTER_MATCH_VLAN;
    rx_cmd.vlan.vlan = 10;
    // printf("opcode: %d rx_cmd_size: %d\n", d_cmd.cmd.opcode, sizeof(rx_cmd));
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    // memcpy(&d_comp, &rx_comp, sizeof(rx_comp));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    rx_cmd.opcode = IONIC_CMD_RX_FILTER_ADD;
    rx_cmd.match = IONIC_RX_FILTER_MATCH_MAC;
    mac1 = 0x12345678ABCD;
    memcpy(rx_cmd.mac.addr, &mac1, 6);
    memrev(rx_cmd.mac.addr, 6);
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

     // Set modes
     struct ionic_rx_mode_set_cmd rx_mode_cmd;
     rx_mode_cmd.opcode = IONIC_CMD_RX_MODE_SET;
     rx_mode_cmd.rx_mode = IONIC_RX_MODE_F_BROADCAST;
     memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

     // Set Promiscuous mode
     rx_mode_cmd.opcode = IONIC_CMD_RX_MODE_SET;
     rx_mode_cmd.rx_mode = IONIC_RX_MODE_F_PROMISC;
     memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // RESET
    d_cmd.cmd.opcode = IONIC_CMD_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // LIF_INIT
    init_cmd.opcode = IONIC_CMD_LIF_INIT;
    memcpy(&d_cmd, &init_cmd, sizeof(init_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // Vlan Filter
    // struct ionic_rx_filter_add_comp rx_comp;
    rx_cmd.opcode = IONIC_CMD_RX_FILTER_ADD;
    rx_cmd.match = IONIC_RX_FILTER_MATCH_VLAN;
    rx_cmd.vlan.vlan = 10;
    // printf("opcode: %d rx_cmd_size: %d\n", d_cmd.cmd.opcode, sizeof(rx_cmd));
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    // memcpy(&d_comp, &rx_comp, sizeof(rx_comp));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // Mac Filter
    rx_cmd.opcode = IONIC_CMD_RX_FILTER_ADD;
    rx_cmd.match = IONIC_RX_FILTER_MATCH_MAC;
    mac1 = 0x12345678ABCD;
    memcpy(rx_cmd.mac.addr, &mac1, 6);
    memrev(rx_cmd.mac.addr, 6);
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

     // Set modes
     rx_mode_cmd.opcode = IONIC_CMD_RX_MODE_SET;
     rx_mode_cmd.rx_mode = IONIC_RX_MODE_F_BROADCAST;
     memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

     // Set Promiscuous mode
     rx_mode_cmd.opcode = IONIC_CMD_RX_MODE_SET;
     rx_mode_cmd.rx_mode = IONIC_RX_MODE_F_PROMISC;
     memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // RESET
    d_cmd.cmd.opcode = IONIC_CMD_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);
}

#endif
TEST_F(nicmgr_test, test3)
{
    struct ionic_rx_filter_add_cmd rx_cmd = {0};
    uint64_t mac1;
    Eth *eth_dev = NULL;
    union ionic_dev_cmd d_cmd;
    union ionic_dev_cmd_comp d_comp;
    struct ionic_lif_init_cmd init_cmd = {0};


    // Get eth device
    eth_dev = (Eth *)devmgr->GetDevice("eth0");
    assert(eth_dev != NULL);

    // RESET
    d_cmd = {0};
    d_cmd.cmd.opcode = IONIC_CMD_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // LIF_INIT
    init_cmd = {0};
    init_cmd.opcode = IONIC_CMD_LIF_INIT;
    memcpy(&d_cmd, &init_cmd, sizeof(init_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    rx_cmd = {0};
    rx_cmd.opcode = IONIC_CMD_RX_FILTER_ADD;
    rx_cmd.match = IONIC_RX_FILTER_MATCH_MAC;
    mac1 = 0x12345678ABCD;
    memcpy(rx_cmd.mac.addr, &mac1, 6);
    memrev(rx_cmd.mac.addr, 6);
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    rx_cmd = {0};
    rx_cmd.opcode = IONIC_CMD_RX_FILTER_ADD;
    rx_cmd.match = IONIC_RX_FILTER_MATCH_MAC;
    mac1 = 0x01005e010101;
    memcpy(rx_cmd.mac.addr, &mac1, 6);
    memrev(rx_cmd.mac.addr, 6);
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);


    // Get eth device
    eth_dev = (Eth *)devmgr->GetDevice("oob_mnic0");
    assert(eth_dev != NULL);

    // RESET
    d_cmd.cmd.opcode = IONIC_CMD_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // LIF_INIT
    init_cmd = {0};
    init_cmd.opcode = IONIC_CMD_LIF_INIT;
    memcpy(&d_cmd, &init_cmd, sizeof(init_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    rx_cmd = {0};
    rx_cmd.opcode = IONIC_CMD_RX_FILTER_ADD;
    rx_cmd.match = IONIC_RX_FILTER_MATCH_MAC;
    mac1 = 0x12345678ABCD;
    memcpy(rx_cmd.mac.addr, &mac1, 6);
    memrev(rx_cmd.mac.addr, 6);
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    rx_cmd = {0};
    rx_cmd.opcode = IONIC_CMD_RX_FILTER_ADD;
    rx_cmd.match = IONIC_RX_FILTER_MATCH_MAC;
    mac1 = 0x01005e020202;
    memcpy(rx_cmd.mac.addr, &mac1, 6);
    memrev(rx_cmd.mac.addr, 6);
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);


    devapi *dev_api = devmgr->DevApi();
    // Add Mac
    dev_api->swm_add_mac(0x000000010001, 0);
    // Add Vlan
    dev_api->swm_add_vlan(0, 0);
    // Add Mac
    dev_api->swm_add_mac(0x000000010001, 1);
    // Enable TX for channel 0
    dev_api->swm_enable_tx(0);
    // Disable TX for channel 0
    dev_api->swm_disable_tx(0);
    // Del Mac
    dev_api->swm_del_mac(0x000000010001, 1);
    // Add Mac
    dev_api->swm_add_mac(0x000000010001, 1);
    // Enable rx
    dev_api->swm_enable_rx(0);
    // Enable rx
    dev_api->swm_disable_rx(0);
    // Enable bcast filters
    lif_bcast_filter_t bc_filter = {0};
    bc_filter.arp = true;
    bc_filter.dhcp_client = true;
    bc_filter.dhcp_server = true;
    bc_filter.netbios = true;
    dev_api->swm_upd_bcast_filter(bc_filter, 0);
    // Modify bcast filters
    bc_filter.arp = false;
    bc_filter.dhcp_client = false;
    bc_filter.dhcp_server = false;
    bc_filter.netbios = true;
    dev_api->swm_upd_bcast_filter(bc_filter, 0);
    // Enable mcast filters
    lif_mcast_filter_t mc_filter = {0};
    mc_filter.ipv6_neigh_adv = true;
    mc_filter.ipv6_router_adv = true;
    mc_filter.dhcpv6_relay = true;
    mc_filter.dhcpv6_mcast = true;
    mc_filter.ipv6_mld = true;
    mc_filter.ipv6_neigh_sol = true;
    dev_api->swm_upd_mcast_filter(mc_filter, 1);


#if 0
    // Add Vlan
    dev_api->swm_add_vlan(10);
#endif



#if 0
    // Enable SWM
    dev_api->swm_enable();
    // Add Uplink
    dev_api->swm_set_port(1);
    // Add Mac
    dev_api->swm_add_mac(0x000000010001);
    // Add Untagged Vlan
    dev_api->swm_add_vlan(0);
    // Remove BC
    dev_api->swm_upd_rx_bmode(false);
    // Remove ALL_MC
    dev_api->swm_upd_rx_mmode(false);
    // Add to BC
    dev_api->swm_upd_rx_bmode(true);
    // Add to ALL_MC
    dev_api->swm_upd_rx_mmode(true);
    // Add Bcast filter
    lif_bcast_filter_t bcast_filter = {0};
    bcast_filter.arp = true;
    dev_api->swm_upd_bcast_filter(bcast_filter);
    // Change Uplink
    dev_api->swm_set_port(5);
    // Add a new vlan
    dev_api->swm_add_vlan(10);
    // Del vlan
    dev_api->swm_del_vlan(10);
    // Disable SWM
    dev_api->swm_disable();
#endif



#if 0
    // Enable swm
    devmgr->swm_update(true, 1, 0, 0);

    // Change swm
    devmgr->swm_update(true, 1, 10, 0);

    // Disable swm
    devmgr->swm_update(false, 0, 0, 0);
#endif
}

int main(int argc, char **argv) {
    printf("In main test.....");
    // sdk::lib::logger::init(sdk_error_logger, sdk_debug_logger);
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--classic") == 0) {
            g_fwd_mode = sdk::lib::FORWARDING_MODE_CLASSIC;
        }
    }
    printf("\n------------------ Executing tests in mode: %s ---------------------\n",
           (g_fwd_mode == sdk::lib::FORWARDING_MODE_CLASSIC) ? "CLASSIC" : "SMART");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
