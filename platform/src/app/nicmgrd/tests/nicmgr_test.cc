#include <gtest/gtest.h>
#include <stdio.h>

#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/eth_dev.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"
#include <grpc++/grpc++.h>
#include "gen/proto/interface.grpc.pb.h"
#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>
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

class pciemgr *pciemgr;

enum ForwardingMode g_fwd_mode = FWD_MODE_SMART_NIC;

namespace nicmgr {
shared_ptr<nicmgr::NicMgrService> g_nicmgr_svc;
}

DeviceManager *devmgr = NULL;

void
create_uplinks()
{
    InterfaceSpec           *spec;
    InterfaceRequestMsg     req_msg;
    InterfaceResponseMsg    rsp_msg;
    ClientContext           context;
    Status                  status;
    uint64_t                port_num[3] = {1, 5, 9};
    int                     num_uplinks = 3;
    int                     if_id_start = 128;

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
         cout << "[INFO] Connecting to HAL, channel status = " << channel->GetState(true) << endl;
     }

    std::unique_ptr<Interface::Stub> intf_stub_ = Interface::NewStub(channel);

    for (uint32_t i = 0; i < num_uplinks; i++) {
        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_interface_id(if_id_start++);
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

void
nicmgr_init()
{
    // DeviceManager *devmgr;

    if (g_fwd_mode == FWD_MODE_CLASSIC_NIC) {
        devmgr =
            new DeviceManager("../platform/src/app/nicmgrd/etc/device.json",
                              g_fwd_mode, PLATFORM_HW);
    } else {
        devmgr =
            new DeviceManager("../platform/src/app/nicmgrd/etc/eth-smart.json",
                              g_fwd_mode, PLATFORM_HW);
    }
    EXPECT_TRUE(devmgr != NULL);

    // load config
    if (g_fwd_mode == FWD_MODE_CLASSIC_NIC) {
        devmgr->LoadConfig("../platform/src/app/nicmgrd/etc/device.json");
    } else {
        devmgr->LoadConfig("../platform/src/app/nicmgrd/etc/eth-smart.json");
    }

    // nicmgr::handle_hal_up();
    devicemanager_init();

}

static int
sdk_trace_cb (sdk_trace_level_e trace_level, const char *format, ...)
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

TEST_F(nicmgr_test, test1)
{
    // Get eth device
    // 66: OOB Lif
    Eth *eth_dev = (Eth *)devmgr->GetDevice(66); // for hw_lif_id of 1

    union dev_cmd d_cmd;
    union dev_cmd_comp d_comp;

    // RESET
    d_cmd.cmd.opcode = CMD_OPCODE_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // LIF_INIT
    struct lif_init_cmd init_cmd;
    init_cmd.opcode = CMD_OPCODE_LIF_INIT;
    memcpy(&d_cmd, &init_cmd, sizeof(init_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    struct rx_filter_add_cmd rx_cmd;
    // struct rx_filter_add_comp rx_comp;
    rx_cmd.opcode = CMD_OPCODE_RX_FILTER_ADD;
    rx_cmd.match = RX_FILTER_MATCH_VLAN;
    rx_cmd.vlan.vlan = 10;
    // printf("opcode: %d rx_cmd_size: %d\n", d_cmd.cmd.opcode, sizeof(rx_cmd));
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    // memcpy(&d_comp, &rx_comp, sizeof(rx_comp));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    rx_cmd.opcode = CMD_OPCODE_RX_FILTER_ADD;
    rx_cmd.match = RX_FILTER_MATCH_MAC;
    uint64_t mac1 = 0x12345678ABCD;
    memcpy(rx_cmd.mac.addr, &mac1, 6);
    memrev(rx_cmd.mac.addr, 6);
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

     // Set modes
     struct rx_mode_set_cmd rx_mode_cmd;
     rx_mode_cmd.opcode = CMD_OPCODE_RX_MODE_SET;
     rx_mode_cmd.rx_mode = RX_MODE_F_BROADCAST;
     memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

     // Set Promiscuous mode
     rx_mode_cmd.opcode = CMD_OPCODE_RX_MODE_SET;
     rx_mode_cmd.rx_mode = RX_MODE_F_PROMISC;
     memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // RESET
    d_cmd.cmd.opcode = CMD_OPCODE_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // LIF_INIT
    init_cmd.opcode = CMD_OPCODE_LIF_INIT;
    memcpy(&d_cmd, &init_cmd, sizeof(init_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // Mac Filter
    rx_cmd.opcode = CMD_OPCODE_RX_FILTER_ADD;
    rx_cmd.match = RX_FILTER_MATCH_MAC;
    mac1 = 0x12345678ABCD;
    memcpy(rx_cmd.mac.addr, &mac1, 6);
    memrev(rx_cmd.mac.addr, 6);
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);


    int num_vlan_filters = 2048;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    for (int i = 1; i < num_vlan_filters; i++) {
        // Vlan Filter
        rx_cmd.opcode = CMD_OPCODE_RX_FILTER_ADD;
        rx_cmd.match = RX_FILTER_MATCH_VLAN;
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
     rx_mode_cmd.opcode = CMD_OPCODE_RX_MODE_SET;
     rx_mode_cmd.rx_mode = RX_MODE_F_BROADCAST;
     memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

     // Set Promiscuous mode
     rx_mode_cmd.opcode = CMD_OPCODE_RX_MODE_SET;
     rx_mode_cmd.rx_mode = RX_MODE_F_PROMISC;
     memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // RESET
    d_cmd.cmd.opcode = CMD_OPCODE_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);
}

TEST_F(nicmgr_test, test2)
{
    // Get eth device
    // 68: Host Lif
    Eth *eth_dev = (Eth *)devmgr->GetDevice(68); // for hw_lif_id of 1

    hal_lif_info_t *lif_info = eth_dev->GetHalLifInfo();
    lif_info->enable_rdma = false;

    union dev_cmd d_cmd;
    union dev_cmd_comp d_comp;

    // RESET
    d_cmd.cmd.opcode = CMD_OPCODE_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // LIF_INIT
    struct lif_init_cmd init_cmd;
    init_cmd.opcode = CMD_OPCODE_LIF_INIT;
    memcpy(&d_cmd, &init_cmd, sizeof(init_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    struct rx_filter_add_cmd rx_cmd;
    // struct rx_filter_add_comp rx_comp;
    rx_cmd.opcode = CMD_OPCODE_RX_FILTER_ADD;
    rx_cmd.match = RX_FILTER_MATCH_VLAN;
    rx_cmd.vlan.vlan = 10;
    // printf("opcode: %d rx_cmd_size: %d\n", d_cmd.cmd.opcode, sizeof(rx_cmd));
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    // memcpy(&d_comp, &rx_comp, sizeof(rx_comp));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    rx_cmd.opcode = CMD_OPCODE_RX_FILTER_ADD;
    rx_cmd.match = RX_FILTER_MATCH_MAC;
    uint64_t mac1 = 0x12345678ABCD;
    memcpy(rx_cmd.mac.addr, &mac1, 6);
    memrev(rx_cmd.mac.addr, 6);
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

     // Set modes
     struct rx_mode_set_cmd rx_mode_cmd;
     rx_mode_cmd.opcode = CMD_OPCODE_RX_MODE_SET;
     rx_mode_cmd.rx_mode = RX_MODE_F_BROADCAST;
     memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

     // Set Promiscuous mode
     rx_mode_cmd.opcode = CMD_OPCODE_RX_MODE_SET;
     rx_mode_cmd.rx_mode = RX_MODE_F_PROMISC;
     memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // RESET
    d_cmd.cmd.opcode = CMD_OPCODE_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // LIF_INIT
    init_cmd.opcode = CMD_OPCODE_LIF_INIT;
    memcpy(&d_cmd, &init_cmd, sizeof(init_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // Vlan Filter
    // struct rx_filter_add_comp rx_comp;
    rx_cmd.opcode = CMD_OPCODE_RX_FILTER_ADD;
    rx_cmd.match = RX_FILTER_MATCH_VLAN;
    rx_cmd.vlan.vlan = 10;
    // printf("opcode: %d rx_cmd_size: %d\n", d_cmd.cmd.opcode, sizeof(rx_cmd));
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    // memcpy(&d_comp, &rx_comp, sizeof(rx_comp));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // Mac Filter
    rx_cmd.opcode = CMD_OPCODE_RX_FILTER_ADD;
    rx_cmd.match = RX_FILTER_MATCH_MAC;
    mac1 = 0x12345678ABCD;
    memcpy(rx_cmd.mac.addr, &mac1, 6);
    memrev(rx_cmd.mac.addr, 6);
    memcpy(&d_cmd, &rx_cmd, sizeof(rx_cmd));
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

     // Set modes
     rx_mode_cmd.opcode = CMD_OPCODE_RX_MODE_SET;
     rx_mode_cmd.rx_mode = RX_MODE_F_BROADCAST;
     memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

     // Set Promiscuous mode
     rx_mode_cmd.opcode = CMD_OPCODE_RX_MODE_SET;
     rx_mode_cmd.rx_mode = RX_MODE_F_PROMISC;
     memcpy(&d_cmd, &rx_mode_cmd, sizeof(rx_mode_cmd));
     eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);

    // RESET
    d_cmd.cmd.opcode = CMD_OPCODE_RESET;
    eth_dev->CmdHandler(&d_cmd, NULL, &d_comp, NULL);
}

int main(int argc, char **argv) {
    utils::logger::init(false);
    sdk_init();
    // sdk::lib::logger::init(sdk_error_logger, sdk_debug_logger);
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--classic") == 0) {
            g_fwd_mode = FWD_MODE_CLASSIC_NIC;
        }
    }
    printf("\n------------------ Execting tests in mode: %s ---------------------\n",
           (g_fwd_mode == FWD_MODE_CLASSIC_NIC) ? "CLASSIC" : "SMART");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
