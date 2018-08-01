#include <gtest/gtest.h>
#include <stdio.h>
#include "hal.hpp"
#include "nic.hpp"

using namespace std;

class span_test : public ::testing::Test {
protected:
  span_test() {
  }

  virtual ~span_test() {
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

// ----------------------------------------------------------------------------
// Span test
// ----------------------------------------------------------------------------
TEST_F(span_test, test1)
{
    shared_ptr<HalClient> client = HalClient::GetInstance(FWD_MODE_CLASSIC);
    shared_ptr<Uplink> uplink1 = make_shared<Uplink>(1);
    shared_ptr<Vrf> host_vrf = make_shared<Vrf>();
    shared_ptr<Vrf> infra_vrf = make_shared<Vrf>();

    struct eth_spec spec;

    spec.rxq_count = 1;
    spec.txq_count = 1;
    spec.adminq_count = 1;

    printf("Create Ethernet Lif\n");
    shared_ptr<EthLif> host_eth = make_shared<EthLif>(uplink1, host_vrf, spec, /* is_mgmt_lif */ false);

    printf("Create SPAN Lif\n");
    shared_ptr<EthLif> span_eth = make_shared<EthLif>(uplink1, infra_vrf, spec, /* is_mgmt_lif */ false);

    // while (1) { sleep(INT_MAX); }

    printf("Done!\n");

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
