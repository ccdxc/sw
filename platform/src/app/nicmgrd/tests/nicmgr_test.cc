#include <gtest/gtest.h>
#include <stdio.h>

#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/eth_dev.hpp"
#include "platform/src/app/nicmgrd/src/delphic.hpp"

using namespace std;

class pciemgr *pciemgr;

enum ForwardingMode g_fwd_mode = FWD_MODE_CLASSIC_NIC;

namespace nicmgr {
shared_ptr<nicmgr::NicMgrService> g_nicmgr_svc;
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
    DeviceManager *devmgr;

    if (g_fwd_mode == FWD_MODE_CLASSIC_NIC) {
        devmgr =
            new DeviceManager("../platform/src/app/nicmgrd/etc/eth.json",
                              g_fwd_mode, PLATFORM_NONE, false);
    } else {
        devmgr =
            new DeviceManager("../platform/src/app/nicmgrd/etc/eth-smart.json",
                              g_fwd_mode, PLATFORM_NONE, false);
    }
    EXPECT_TRUE(devmgr != NULL);

    pciemgr = new class pciemgr("nicmgr_test");
    // load config
    if (g_fwd_mode == FWD_MODE_CLASSIC_NIC) {
        devmgr->LoadConfig("../platform/src/app/nicmgrd/etc/eth.json");
    } else {
        devmgr->LoadConfig("../platform/src/app/nicmgrd/etc/eth-smart.json");
    }

    // Get eth device
    Eth *eth_dev = (Eth *)devmgr->GetDevice(1); // for hw_lif_id of 1

    union dev_cmd d_cmd;
    union dev_cmd_comp d_comp;

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
}

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "--classic") == 0) {
          g_fwd_mode = FWD_MODE_CLASSIC_NIC;
      }
  }
  printf("Execting tests in mode: %s\n",
         (g_fwd_mode == FWD_MODE_CLASSIC_NIC) ? "CLASSIC" : "SMART");
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
