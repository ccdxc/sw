#include <gtest/gtest.h>
#include <stdio.h>

#include "nic/sdk/lib/device/device.hpp"
#include "nic/sdk/platform/devapi/devapi_types.hpp"
#include "platform/src/lib/devapi_iris/devapi_iris.hpp"
#include "platform/src/lib/nicmgr/include/logger.hpp"

using namespace std;

class devapi_iris_test : public ::testing::Test {
protected:
  devapi_iris_test() {
  }

  virtual ~devapi_iris_test() {
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
TEST_F(devapi_iris_test, test1)
{
    sdk_ret_t ret = SDK_RET_OK;
    devapi *dev_api = devapi_iris::factory();

    // uplink
    dev_api->uplink_create(128, 1, false);
    ASSERT_TRUE(ret == SDK_RET_OK);

    dev_api->uplink_destroy(1);

    devapi_iris::destroy(dev_api);


}

// ----------------------------------------------------------------------------
// Smart NIC case
// ----------------------------------------------------------------------------
TEST_F(devapi_iris_test, test2)
{
    sdk_ret_t ret = SDK_RET_OK;
    lif_info_t info = {0};
    mac_t mac1 = 0x000000010101;

    devapi *dev_api = devapi_iris::factory();

    dev_api->set_micro_seg_en(true);

    // uplink
    dev_api->uplink_create(128, 2, false);
    ASSERT_TRUE(ret == SDK_RET_OK);

    dev_api->uplink_destroy(1);

    info.lif_id = 1;
    info.pinned_uplink_port_num = 2;
    info.is_management = false;
    info.receive_promiscuous = true;
    info.max_vlan_filters = 10;
    info.max_mac_filters = 10;
    info.max_mac_vlan_filters = 10;
    dev_api->lif_create(&info);

    // Add mac
    dev_api->lif_add_mac(1, mac1);
    dev_api->lif_add_vlan(1, 10);
    dev_api->lif_del_vlan(1, 10);
    dev_api->lif_del_mac(1, mac1);

    // devapi_iris::destroy(dev_api);


}

#if 0
// Multicast filter
TEST_F(devapi_iris_test, test2)
{
    hal_irisc_ret_t ret;
    hal_lif_info_t info = {0};
    mac_t mac1 = 0x01005E010101;

    // Create Lif
    // Create Lifs
    info.hw_lif_id = 2;
    info.pinned_uplink_port_num = 2;
    info.is_management = false;
    info.receive_promiscuous = true;
    info.max_vlan_filters = 10;
    info.max_mac_filters = 10;
    info.max_mac_vlan_filters = 10;
    Lif *lif1 = Lif::Factory(&info);

    // Add Mac filter
    lif1->AddMac(mac1++);
    lif1->AddMac(mac1++);
    lif1->AddMac(mac1++);

    // Add Vlan filter.
    // - Create L2seg
    // - Add L2seg on Enic
    // - Create EP
    lif1->AddVlan(1);

    lif1->AddVlan(2);

    ret = lif1->AddVlan(3);
    ASSERT_TRUE(ret == HAL_IRISC_RET_FAIL);
}

// ----------------------------------------------------------------------------
// Add Mac failure
// ----------------------------------------------------------------------------
TEST_F(devapi_iris_test, test3)
{
    hal_irisc_ret_t ret;
    hal_lif_info_t info = {0};
    mac_t mac1 = 0x000202030405;

    // Create Lif
    // Create Lifs
    info.hw_lif_id = 3;
    info.pinned_uplink_port_num = 1;
    info.is_management = false;
    info.receive_promiscuous = true;
    info.max_vlan_filters = 10;
    info.max_mac_filters = 10;
    info.max_mac_vlan_filters = 10;
    Lif *lif1 = Lif::Factory(&info);

    lif1->AddMac(mac1++);
    lif1->AddMac(mac1++);
    lif1->AddMac(mac1++);

    lif1->AddVlan(31);
    lif1->AddVlan(32);
    lif1->AddVlan(33);
    lif1->AddMac(mac1++);
    lif1->AddMac(mac1++);
    ret = lif1->AddMac(mac1++);
    ASSERT_TRUE(ret == HAL_IRISC_RET_FAIL);

}

// ----------------------------------------------------------------------------
// Add McastMac failure
// ----------------------------------------------------------------------------
TEST_F(devapi_iris_test, test4)
{
    hal_irisc_ret_t ret;
    hal_lif_info_t info = {0};
    mac_t mac1 = 0x01015E010101;

    // Create Lif
    // Create Lifs
    info.hw_lif_id = 4;
    info.pinned_uplink_port_num = 1;
    info.is_management = false;
    info.receive_promiscuous = true;
    info.max_vlan_filters = 10;
    info.max_mac_filters = 10;
    info.max_mac_vlan_filters = 10;
    Lif *lif1 = Lif::Factory(&info);

    lif1->AddMac(mac1++);
    lif1->AddMac(mac1++);
    lif1->AddMac(mac1++);

    lif1->AddVlan(41);
    lif1->AddVlan(42);
    lif1->AddVlan(43);
    lif1->AddMac(mac1++);
    lif1->AddMac(mac1++);
    ret = lif1->AddMac(mac1++);
    ASSERT_TRUE(ret == HAL_IRISC_RET_FAIL);

}

// ----------------------------------------------------------------------------
// Filter failure. HAL is down
// ----------------------------------------------------------------------------
TEST_F(devapi_iris_test, test5)
{
    hal_irisc_ret_t ret;
    hal_lif_info_t info = {0};
    mac_t mac1 = 0x000502030405;

    // Create Lif
    // Create Lifs
    info.hw_lif_id = 5;
    info.pinned_uplink_port_num = 1;
    info.is_management = false;
    info.receive_promiscuous = true;
    info.max_vlan_filters = 10;
    info.max_mac_filters = 10;
    info.max_mac_vlan_filters = 10;
    Lif *lif1 = Lif::Factory(&info);

    // Bringing down HAL
    HalCommonClient *client = HalGRPCClient::GetInstance();
    HalGRPCClient::Destroy(client);

    ret = lif1->AddMac(mac1++);
    ASSERT_TRUE(ret == HAL_IRISC_RET_FAIL);
}

// ----------------------------------------------------------------------------
// Filter failure. HAL is down
// ----------------------------------------------------------------------------
TEST_F(devapi_iris_test, test6)
{
    hal_irisc_ret_t ret;
    hal_lif_info_t info = {0};
    mac_t mac1 = 0x000502030405;

    HalGRPCClient::Factory(FWD_MODE_CLASSIC);

    // Create Lif
    // Create Lifs
    info.hw_lif_id = 6;
    info.pinned_uplink_port_num = 1;
    info.is_management = false;
    info.receive_promiscuous = true;
    info.max_vlan_filters = 10;
    info.max_mac_filters = 10;
    info.max_mac_vlan_filters = 10;
    Lif *lif1 = Lif::Factory(&info);

    ret = lif1->AddMac(mac1++);
    ASSERT_TRUE(ret == HAL_IRISC_RET_SUCCESS);

    // Bringing down HAL
    HalCommonClient *client = HalGRPCClient::GetInstance();
    HalGRPCClient::Destroy(client);

    ret = lif1->AddVlan(1);
    ASSERT_TRUE(ret == HAL_IRISC_RET_FAIL);
}

// ----------------------------------------------------------------------------
// Filter failure. HAL is down
// ----------------------------------------------------------------------------
TEST_F(devapi_iris_test, test7)
{
    hal_irisc_ret_t ret;
    hal_lif_info_t info = {0};
    mac_t mac1 = 0x000502030405;

    // Bringing down HAL
    HalCommonClient *client = HalGRPCClient::GetInstance();
    HalGRPCClient::Destroy(client);

    // Create Lif
    // Create Lifs
    info.hw_lif_id = 7;
    info.pinned_uplink_port_num = 1;
    info.is_management = false;
    info.receive_promiscuous = true;
    info.max_vlan_filters = 10;
    info.max_mac_filters = 10;
    info.max_mac_vlan_filters = 10;
    Lif *lif1 = Lif::Factory(&info);
    ASSERT_TRUE(lif1 == NULL);
}
#endif


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    utils::logger::init();
    return RUN_ALL_TESTS();
}
