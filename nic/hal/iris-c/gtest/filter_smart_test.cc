#include <gtest/gtest.h>
#include <stdio.h>
#include "hal_grpc_client.hpp"
#include "uplink.hpp"
#include "ethlif.hpp"

using namespace std;

#if 0
namespace hal {
 namespace utils {

 hal::utils::mem_mgr     g_hal_mem_mgr;

 }    // namespace utils
 }    // namespace hal
#endif

class filter_test : public ::testing::Test {
protected:
  filter_test() {
  }

  virtual ~filter_test() {
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
// Filter test
// ----------------------------------------------------------------------------
TEST_F(filter_test, test1)
{
    mac_t mac1 = 0x000102030405;
    mac_t mac2 = 0x000102030406;
    // mac_t mac3 = 0x000102030407;
    vlan_t vlan1 = 0;
    vlan_t vlan2 = 1;
    // vlan_t vlan3 = 2;

    // Create Hal GRPC client
    // shared_ptr<HalClient> client = HalGRPCClient::GetInstance(FWD_MODE_SMART);
    // HalClient *client = HalGRPCClient::Factory(FWD_MODE_SMART);
    HalGRPCClient::Factory(FWD_MODE_SMART);

    // Create Uplinks
    Uplink *up1 = Uplink::Factory(1);
    // Uplink *up2 = Uplink::Factory(2);

    // Create Lifs
    EthLif *lif1 = EthLif::Factory(1 /*hw_lif_id*/, up1, false /*is_mgmt*/);
    // EthLif *lif2 = EthLif::Factory(2 /*hw_lif_id*/, up2, false /*is_mgmt*/);

    // Add Mac filter
    lif1->AddMac(mac1);

    // Add Vlan filter.
    // - Create L2seg
    // - Add L2seg on Enic
    // - Create EP
    lif1->AddVlan(vlan1);

    // Del Vlan filter.
    // - Delete L2seg
    // - Del L2seg on Enic
    // - Delete EP
    lif1->DelVlan(vlan1);

    // Add (Mac,Vlan) filter - Create L2seg, adds l2seg to enic, creates EP
    lif1->AddMacVlan(mac1, vlan1);

    // Removes Macfilter - Doesnt affect (Mac-Vlan) entity
    lif1->DelMac(mac1);

    // Del (Mac,Vlan) filter - Deletes (Mac-Vlan) entity
    lif1->DelMacVlan(mac1, vlan1);

    // No filters at this point

    // Add (Mac,Vlan) filter - Create (Mac-Vlan) entity
    lif1->AddMacVlan(mac1, vlan1);

    // Add Mac filter - Nop
    lif1->AddMac(mac1);

    // Add Vlan filter - Nop
    lif1->AddVlan(vlan1);

    // Add different mac for same vlan. Just Create EP. No l2seg or l2seg on enic.
    lif1->AddMacVlan(mac2, vlan1);

    // Del mac2. Just removal of EP.
    lif1->DelMacVlan(mac2, vlan1);

    // Del mac1. Nop
    lif1->DelMac(mac1);

    // Del vlan1. Nop
    lif1->DelVlan(vlan1);

    // Del (Mac,Vlan). Deletes (Mac-Vlan) entity
    lif1->DelMacVlan(mac1, vlan1);

    // No filters now

    // Add (m1, v1). Create L2seg, adds l2seg to enic, creates EP
    lif1->AddMacVlan(mac1, vlan1);

    // Add (m1, v2). Create L2seg, adds l2seg to enic, creates EP
    lif1->AddMacVlan(mac1, vlan2);

    lif1->DelMacVlan(mac1, vlan1);
    lif1->DelMacVlan(mac1, vlan2);

    // HalGRPCClient::Destroy(client);
}

TEST_F(filter_test, test2)
{
    mac_t mac1 = 0x000102030405;
    mac_t mac2 = 0x000102030406;
    // mac_t mac3 = 0x000102030407;
    vlan_t vlan1 = 0;
    // vlan_t vlan2 = 1;
    // vlan_t vlan3 = 2;

    // Create Hal GRPC client
    // HalClient *client = HalGRPCClient::Factory(FWD_MODE_SMART);
    HalGRPCClient::Factory(FWD_MODE_SMART);

    // Create Uplinks
    Uplink *up1 = Uplink::Factory(20);
    Uplink::Factory(20);

    // Create Lifs
    EthLif *lif1 = EthLif::Factory(20 /*hw_lif_id*/, up1, false /*is_mgmt*/);
    EthLif::Factory(20 /*hw_lif_id*/, up1, false /*is_mgmt*/);

    // Add Mac filter
    lif1->AddMac(mac1);
    lif1->AddMac(mac1);

    // Add Vlan filter.
    // - Create L2seg
    // - Add L2seg on Enic
    // - Create EP
    lif1->AddVlan(vlan1);
    lif1->AddVlan(vlan1);

    // Del Vlan filter.
    // - Delete L2seg
    // - Del L2seg on Enic
    // - Delete EP
    lif1->DelVlan(vlan1);
    lif1->DelVlan(vlan1);

    // Add (Mac,Vlan) filter - Create L2seg, adds l2seg to enic, creates EP
    lif1->AddMacVlan(mac1, vlan1);
    lif1->AddMacVlan(mac1, vlan1);

    // Removes Macfilter - Doesnt affect (Mac-Vlan) entity
    lif1->DelMac(mac1);
    lif1->DelMac(mac1);

    // Del (Mac,Vlan) filter - Deletes (Mac-Vlan) entity
    lif1->DelMacVlan(mac1, vlan1);
    lif1->DelMacVlan(mac1, vlan1);

    // No filters at this point

    // Add (Mac,Vlan) filter - Create (Mac-Vlan) entity
    lif1->AddMacVlan(mac1, vlan1);
    lif1->AddMacVlan(mac1, vlan1);

    // Add Mac filter - Nop
    lif1->AddMac(mac1);
    lif1->AddMac(mac1);

    // Add Vlan filter - Nop
    lif1->AddVlan(vlan1);
    lif1->AddVlan(vlan1);

    // Add different mac for same vlan. Just Create EP. No l2seg or l2seg on enic.
    lif1->AddMacVlan(mac2, vlan1);
    lif1->AddMacVlan(mac2, vlan1);

    // Del mac2. Just removal of EP.
    lif1->DelMacVlan(mac2, vlan1);
    lif1->DelMacVlan(mac2, vlan1);

    // Del mac1. Nop
    lif1->DelMac(mac1);
    lif1->DelMac(mac1);

    // Del vlan1. Nop
    lif1->DelVlan(vlan1);
    lif1->DelVlan(vlan1);

    // Del (Mac,Vlan). Deletes (Mac-Vlan) entity
    lif1->DelMacVlan(mac1, vlan1);
    lif1->DelMacVlan(mac1, vlan1);

    // HalGRPCClient::Destroy(client);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
