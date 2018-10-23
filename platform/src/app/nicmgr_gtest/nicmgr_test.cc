#include <gtest/gtest.h>
#include <stdio.h>

#include "dev.hpp"
#include "eth_dev.hpp"

using namespace std;

enum ForwardingMode g_fwd_mode = FWD_MODE_SMART_NIC;

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
    DeviceManager *devmgr = new DeviceManager(g_fwd_mode, "/sw/nic/conf", PLATFORM_MODE_NONE);
    EXPECT_TRUE(devmgr != NULL);

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
