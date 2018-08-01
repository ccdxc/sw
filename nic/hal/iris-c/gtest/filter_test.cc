#include <gtest/gtest.h>
#include <stdio.h>
#include "hal.hpp"
#include "nic.hpp"

using namespace std;

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
    shared_ptr<HalClient> client = HalClient::GetInstance(FWD_MODE_CLASSIC);
    shared_ptr<Uplink> uplink1 = make_shared<Uplink>(1);
    shared_ptr<Vrf> vrf1 = make_shared<Vrf>();

    mac_t mac1 = 0x000102030405;
    mac_t mac2 = 0x000102030406;
    mac_t mac3 = 0x000102030407;
    vlan_t vlan1 = 0;
    vlan_t vlan2 = 1;
    vlan_t vlan3 = 2;

    Lif::Probe();
    Enic::Probe();

    struct eth_spec spec;

    spec.rxq_count = 1;
    spec.txq_count = 1;
    spec.adminq_count = 1;

    printf("Create Ethernet Lif\n");
    shared_ptr<EthLif> eth1 = make_shared<EthLif>(uplink1, vrf1, spec, /* is_mgmt_lif */ false);

    printf("Insert a MAC address\n");
    //  case: Insert a MAC address
    //  expect: nop since no vlans are registered yet
    eth1->AddMac(mac1);

    printf("Insert a duplicate MAC address\n");
    //  case: Insert a duplicate MAC address
    //  expect: nop
    eth1->AddMac(mac1);

    printf("Insert another MAC address\n");
    //  case: Insert another MAC address
    //  expect: nop since no vlans are registered yet
    eth1->AddMac(mac2);

    printf("Insert a VLAN\n");
    //  case: Insert a VLAN
    //  expect: Wild card expansion to [(mac1, vlan1), (mac2, vlan1)]
    //      classic: ENIC create (mac1, vlan1), Add VLAN (vlan1) to ENIC, EP create (mac1, vlan1), EP create (mac2, vlan2)
    eth1->AddVlan(vlan1);

    printf("Insert a duplicate VLAN\n");
    //  case: Insert a duplicate VLAN
    //  expect: nop
    eth1->AddVlan(vlan1);

    printf("Insert another VLAN\n");
    //  case: Insert another VLAN
    //  expect: Wild card expansion to [(mac1, vlan2), (mac2, vlan2)]
    //      classic: Add VLAN (vlan2) to ENIC, EP create (mac1, vlan2), EP create (mac2, vlan2)
    eth1->AddVlan(vlan2);

    printf("Insert a MAC-VLAN filter\n");
    //  case: Insert a MAC-VLAN filter
    //  except:
    //      classic: EP create (mac3, vlan3)
    eth1->AddMacVlan(mac3, vlan3);

    printf("Insert a MAC-VLAN filter overlapping with a MAC & a VLAN filter\n");
    //  case: Insert a MAC-VLAN filter overlapping with a MAC & a VLAN filter
    //  except:
    //      classic: Registered (mac2, vlan2) - No Enic/EP create
    eth1->AddMacVlan(mac2, vlan2);

    printf("Insert a duplicate MAC-VLAN filter\n");
    //  case: Insert a duplicate MAC-VLAN filter
    //  except: nop
    eth1->AddMacVlan(mac3, vlan3);

    printf("Insert a MAC(new)-VLAN(existing) filter\n");
    //  case: Insert a MAC(new)-VLAN(existing) filter
    //  except:
    //      classic: EP create (mac3, vlan2)
    eth1->AddMacVlan(mac3, vlan2);

    printf("Delete a MAC from mac_table\n");
    //  case: Delete a mac from mac_table
    //  except:
    //      classic: EP delete for [(mac1, vlan1), (mac1, vlan2)]
    eth1->DelMac(mac1);

    printf("Delete an invalid MAC\n");
    //  case: Delete an invalid MAC
    //  except:
    //      classic: nop
    eth1->DelMac(mac3);

    printf("Delete a VLAN\n");
    //  case: Delete a vlan from vlan_table
    //  except:
    //      classic: EP delete for [(mac2, vlan1)]
    eth1->DelVlan(vlan1);

    printf("Delete an invalid VLAN\n");
    //  case: Delete an invalid VLAN
    //  except:
    //      classic: nop
    eth1->DelVlan(vlan3);

    printf("Delete a valid MAC-VLAN filter\n");
    //  case: Delete a valid MAC-VLAN filter
    //  except:
    //      classic: nop
    eth1->DelMacVlan(mac3, vlan3);

    printf("Delete an invalid MAC-VLAN filter\n");
    //  case: Delete an invalid MAC-VLAN filter
    //  except:
    //      classic: nop
    eth1->DelMacVlan(mac1, vlan1);

    printf("Delete a MAC-VLAN filter overlapping with a MAC & a VLAN filter\n");
    //  case: Delete a MAC-VLAN filter overlapping with a MAC & a VLAN filter
    //  except:
    //      classic: Deregistered (mac2, vlan2) - EP delete (mac2, vlan2)
    eth1->DelMac(mac2);
    eth1->DelVlan(vlan2);
    eth1->DelMacVlan(mac2, vlan2);

    printf("Done!\n");

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
