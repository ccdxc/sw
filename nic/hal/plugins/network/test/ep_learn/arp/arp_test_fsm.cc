#include <arpa/inet.h>
#include <gtest/gtest.h>
#include "nic/include/twheel.hpp"

#include "nic/hal/plugins/network/ep_learn/arp/arp_learn.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/include/arp_trans.hpp"

#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/endpoint.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/network.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/gen/proto/hal/l2segment.pb.h"
#include "nic/gen/proto/hal/tenant.pb.h"
#include "nic/gen/proto/hal/endpoint.pb.h"
#include "nic/gen/proto/hal/nw.pb.h"

#include <tins/tins.h>

using namespace Tins;
using namespace hal;
using namespace hal::network;
using hal::utils::twheel;

/*
 * Note: For now timer wheel is supposed to be non-thread safe.
 * Reason: Update tick locks that slice and calls the callback function.
 * During the callback function, we try to add or delete timer which will
 * obviously wait indefinitely as there is not lock available.
 */

namespace hal {
namespace periodic {
extern twheel *g_twheel;
}
}  // namespace hal

tenant_t *dummy_ten;
ep_t *dummy_ep;


void fte_ctx_init(fte::ctx_t &ctx, hal::tenant_t *ten, hal::ep_t *ep,
        fte::cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len,
        fte::flow_t iflow[], fte::flow_t rflow[]);

void arp_topo_setup()
{
   hal_ret_t                   ret;
   TenantSpec                  ten_spec;
   TenantResponse              ten_rsp;
   L2SegmentSpec               l2seg_spec;
   L2SegmentResponse           l2seg_rsp;
   InterfaceSpec               up_spec;
   InterfaceResponse           up_rsp;
   EndpointSpec                ep_spec, ep_spec1;
   EndpointResponse            ep_rsp;
   EndpointUpdateRequest       ep_req, ep_req1;
   NetworkSpec                 nw_spec, nw_spec1;
   NetworkResponse             nw_rsp, nw_rsp1;


   // Create tenant
   ten_spec.mutable_key_or_handle()->set_tenant_id(1);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::tenant_create(ten_spec, &ten_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);

   // Create network
   nw_spec.mutable_meta()->set_tenant_id(1);
   nw_spec.set_rmac(0x0000DEADBEEE);
   nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
   nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
   nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::network_create(nw_spec, &nw_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);
   uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

   nw_spec1.mutable_meta()->set_tenant_id(1);
   nw_spec1.set_rmac(0x0000DEADBEEF);
   nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
   nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
   nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0b000000);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::network_create(nw_spec1, &nw_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);
   uint64_t nw_hdl1 = nw_rsp.mutable_status()->nw_handle();

   // Create L2 Segment
   l2seg_spec.mutable_meta()->set_tenant_id(1);
   l2seg_spec.add_network_handle(nw_hdl);
   l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
   l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
   l2seg_spec.mutable_fabric_encap()->set_encap_value(11);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);
   uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

   l2seg_spec.mutable_meta()->set_tenant_id(1);
   l2seg_spec.add_network_handle(nw_hdl1);
   l2seg_spec.mutable_key_or_handle()->set_segment_id(2);
   l2seg_spec.mutable_fabric_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
   l2seg_spec.mutable_fabric_encap()->set_encap_value(12);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);
   // uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

   // Create an uplink
   up_spec.mutable_meta()->set_tenant_id(1);
   up_spec.set_type(intf::IF_TYPE_UPLINK);
   up_spec.mutable_key_or_handle()->set_interface_id(1);
   up_spec.mutable_if_uplink_info()->set_port_num(1);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::interface_create(up_spec, &up_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);
   // ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

   up_spec.mutable_meta()->set_tenant_id(1);
   up_spec.set_type(intf::IF_TYPE_UPLINK);
   up_spec.mutable_key_or_handle()->set_interface_id(2);
   up_spec.mutable_if_uplink_info()->set_port_num(2);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::interface_create(up_spec, &up_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);
   ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();
   dummy_ten = tenant_lookup_by_id(1);
   ASSERT_TRUE(dummy_ten != NULL);

   // Create 2 Endpoints
   ep_spec.mutable_meta()->set_tenant_id(1);
   ep_spec.set_l2_segment_handle(l2seg_hdl);
   ep_spec.set_interface_handle(up_hdl2);
   ep_spec.set_mac_address(0x00000000ABCD);
   ep_spec.add_ip_address();
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::endpoint_create(ep_spec, &ep_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);
   dummy_ep = find_ep_by_handle(ep_rsp.endpoint_status().endpoint_handle());
   ASSERT_TRUE(dummy_ep != NULL);
}

class arp_fsm_test : public hal_base_test {
   protected:
    arp_fsm_test() {
        strcpy((char *)(dummy_ep->l2_key.mac_addr), "123456");
    }

    virtual ~arp_fsm_test() {}

    // will be called immediately after the constructor before each test
    virtual void SetUp() {

    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {}

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        hal_base_test::SetUpTestCase();
        /* Override the timer so that we can control  */
        hal::periodic::g_twheel = twheel::factory(10, 100, false);
        arp_topo_setup();
    }
};

EthernetII *get_default_arp_packet(ARP::Flags type,
                                   const char *sender_ip_address,
                                   unsigned char *sender_hw_addr,
                                   const char *target_ip_addr,
                                   unsigned char *target_hw_addr) {
    EthernetII *eth = new EthernetII();
    ARP *arp = new ARP();

    eth->inner_pdu(arp);

    // Retrieve a pointer to the stored TCP PDU
    arp = eth->find_pdu<ARP>();

    arp->opcode(type);

    HWAddress<6> hw_address(sender_hw_addr);
    arp->sender_hw_addr(hw_address);

    HWAddress<6> hw_address_1(target_hw_addr);
    arp->target_hw_addr(hw_address_1);

    arp->sender_ip_addr(sender_ip_address);
    arp->target_ip_addr(target_ip_addr);

    return eth;
}

void arp_packet_send(ARP::Flags type, const char *sender_ip_address,
                     unsigned char *sender_hw_addr, const char *target_ip_addr,
                     unsigned char *target_hw_addr = NULL) {
    EthernetII *eth =
        get_default_arp_packet(type, sender_ip_address, sender_hw_addr,
                               target_ip_addr, target_hw_addr);

    std::vector<uint8_t> buffer = eth->serialize();
    fte::ctx_t ctx;
    fte_ctx_init(ctx, dummy_ten,
            dummy_ep, NULL, &buffer[0], buffer.size(), NULL, NULL);
    hal_ret_t ret = arp_process_packet(ctx);
    ASSERT_EQ(ret, HAL_RET_OK);
}

TEST_F(arp_fsm_test, arp_request_response) {
    arp_packet_send(ARP::Flags::REQUEST, "1.1.1.1", (unsigned char *)"123456",
                    "1.1.1.2");
    arp_trans_key_t key;
    arp_trans_t::init_arp_trans_key((unsigned char *)"123456", dummy_ep, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    arp_packet_send(ARP::Flags::REPLY, "1.1.1.2", (unsigned char *)"567890",
                    "1.1.1.1", (unsigned char *)"123456");
    arp_trans_t::init_arp_trans_key((unsigned char *)"567890", dummy_ep, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    arp_trans_t::init_arp_trans_key((unsigned char *)"123456", dummy_ep, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));

    ASSERT_EQ(g_hal_state->arplearn_key_ht()->num_entries(), 2);
}

TEST_F(arp_fsm_test, arp_spoofing) {
    arp_packet_send(ARP::Flags::REQUEST, "1.1.1.1", (unsigned char *)"123456",
                    "1.1.1.2");
    arp_trans_key_t key;
    arp_trans_t::init_arp_trans_key((unsigned char *)"123456", dummy_ep, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    arp_packet_send(ARP::Flags::REPLY, "1.1.1.2", (unsigned char *)"567890",
                    "1.1.1.1", (unsigned char *)"123456");
    arp_trans_t::init_arp_trans_key((unsigned char *)"567890", dummy_ep, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    arp_trans_t::init_arp_trans_key((unsigned char *)"123456", dummy_ep, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));

    ASSERT_EQ(g_hal_state->arplearn_key_ht()->num_entries(), 2);
}

TEST_F(arp_fsm_test, arp_entry_timeout) {
    arp_packet_send(ARP::Flags::REQUEST, "1.1.1.1", (unsigned char *)"123456",
                    "1.1.1.2");
    arp_trans_key_t key;
    arp_trans_t::init_arp_trans_key((unsigned char *)"123456", dummy_ep, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);

    sleep(1);
    hal::periodic::g_twheel->tick(entry->get_current_state_timeout() + 100);
    entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry == NULL);
}


TEST_F(arp_fsm_test, arp_request_response_replace) {
    arp_packet_send(ARP::Flags::REQUEST, "1.1.1.1", (unsigned char *)"123456",
                    "1.1.1.2");
    arp_trans_key_t key;
    arp_trans_t::init_arp_trans_key((unsigned char *)"123456", dummy_ep, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    arp_packet_send(ARP::Flags::REPLY, "1.1.1.2", (unsigned char *)"567890",
                    "1.1.1.1", (unsigned char *)"123456");
    arp_trans_t::init_arp_trans_key((unsigned char *)"567890", dummy_ep, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    arp_trans_t::init_arp_trans_key((unsigned char *)"123456", dummy_ep, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));

    ASSERT_EQ(g_hal_state->arplearn_key_ht()->num_entries(), 2);

    strcpy((char *)(dummy_ep->l2_key.mac_addr), "654321");
    arp_packet_send(ARP::Flags::REQUEST, "1.1.1.1", (unsigned char *)"654321",
                    "1.1.1.2");
    arp_trans_t::init_arp_trans_key((unsigned char *)"654321", dummy_ep, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    ASSERT_EQ(g_hal_state->arplearn_key_ht()->num_entries(), 2);

    strcpy((char *)(dummy_ep->l2_key.mac_addr), "098756");
    arp_packet_send(ARP::Flags::REQUEST, "1.1.1.2", (unsigned char *)"098756",
                    "1.1.1.2");
    arp_trans_t::init_arp_trans_key((unsigned char *)"098756", dummy_ep, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        g_hal_state->arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    ASSERT_EQ(g_hal_state->arplearn_key_ht()->num_entries(), 2);
}
