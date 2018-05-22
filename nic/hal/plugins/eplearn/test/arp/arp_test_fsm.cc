#include <arpa/inet.h>
#include <gtest/gtest.h>
#include "sdk/twheel.hpp"

#include "nic/hal/plugins/eplearn/arp/arp_learn.hpp"
#include "nic/hal/plugins/eplearn/arp/ndp_learn.hpp"
#include "nic/hal/plugins/eplearn/arp/arp_trans.hpp"
#include "nic/hal/plugins/eplearn/eplearn.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/include/pd_api.hpp"

#include "nic/hal/src/nw/interface.hpp"
#include "nic/hal/src/nw/endpoint.hpp"
#include "nic/hal/src/nw/session.hpp"
#include "nic/hal/src/nw/nw.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/gen/proto/hal/l2segment.pb.h"
#include "nic/gen/proto/hal/vrf.pb.h"
#include "nic/gen/proto/hal/endpoint.pb.h"
#include "nic/gen/proto/hal/nw.pb.h"

#include <tins/tins.h>

using namespace Tins;
using namespace hal;
using namespace hal::eplearn;
using sdk::lib::twheel;

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

vrf_t *dummy_ten;
uint64_t dummy_l2seg_hdl;
#define MAX_ENDPOINTS 8
#define ARP_ENTRY_TIMEOUT 5
hal_handle_t ep_handles[MAX_ENDPOINTS];
uint32_t mac_addr_base = 12345;
uint8_t mac_addr_buffer[MAX_ENDPOINTS][ETH_ADDR_LEN + 1];

#define GET_MAC_ADDR(_ep) ((unsigned char*)mac_addr_buffer[_ep])

static void
init_mac_addresses(void)
{
    for (uint32_t i = 0; i < MAX_ENDPOINTS; i++) {
        uint32_t mac_addr = mac_addr_base + i;
        memset(mac_addr_buffer[i], 0, ETH_ADDR_LEN + 1);
        mac_addr_buffer[i][5] = ((uint8_t*)(&mac_addr))[0];
        mac_addr_buffer[i][4] = ((uint8_t*)(&mac_addr))[1];
    }
}

void fte_ctx_init(fte::ctx_t &ctx, hal::vrf_t *ten, hal::ep_t *ep,
        hal::ep_t *dep, fte::cpu_rxhdr_t *cpu_rxhdr,
        uint8_t *pkt, size_t pkt_len,
        fte::flow_t iflow[], fte::flow_t rflow[], fte::feature_state_t feature_state[]);

void arp_topo_setup()
{
   hal_ret_t                   ret;
   VrfSpec                  ten_spec;
   VrfResponse              ten_rsp;
   L2SegmentSpec               l2seg_spec;
   L2SegmentResponse           l2seg_rsp;
   InterfaceSpec               up_spec;
   InterfaceResponse           up_rsp;
   EndpointSpec                ep_spec, ep_spec1;
   EndpointResponse            ep_rsp;
   EndpointUpdateRequest       ep_req, ep_req1;
   NetworkSpec                 nw_spec, nw_spec1;
   NetworkResponse             nw_rsp, nw_rsp1;
   NetworkKeyHandle            *nkh = NULL;

   init_mac_addresses();
   // Create vrf
   ten_spec.mutable_key_or_handle()->set_vrf_id(1);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::vrf_create(ten_spec, &ten_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);

   // Create network
   nw_spec.set_rmac(0x0000DEADBEEE);
   nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
   nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
   nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
   nw_spec.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::network_create(nw_spec, &nw_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);
   uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

   nw_spec1.set_rmac(0x0000DEADBEEF);
   nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->set_prefix_len(24);
   nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
   nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0b000000);
   nw_spec1.mutable_key_or_handle()->mutable_nw_key()->mutable_vrf_key_handle()->set_vrf_id(1);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::network_create(nw_spec1, &nw_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);
   uint64_t nw_hdl1 = nw_rsp.mutable_status()->nw_handle();

   // Create L2 Segment
   l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
   nkh = l2seg_spec.add_network_key_handle();
   nkh->set_nw_handle(nw_hdl);
   l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
   l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
   l2seg_spec.mutable_wire_encap()->set_encap_value(11);
   l2seg_spec.mutable_eplearn_cfg()->mutable_arp()->set_entry_timeout(ARP_ENTRY_TIMEOUT);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);
   uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();
   dummy_l2seg_hdl = l2seg_hdl;

   l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
   nkh = l2seg_spec.add_network_key_handle();
   nkh->set_nw_handle(nw_hdl1);
   l2seg_spec.mutable_key_or_handle()->set_segment_id(2);
   l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
   l2seg_spec.mutable_wire_encap()->set_encap_value(12);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);
   // uint64_t l2seg_hdl2 = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

   // Create an uplink
   up_spec.set_type(intf::IF_TYPE_UPLINK);
   up_spec.mutable_key_or_handle()->set_interface_id(1);
   up_spec.mutable_if_uplink_info()->set_port_num(1);
   hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
   ret = hal::interface_create(up_spec, &up_rsp);
   hal::hal_cfg_db_close();
   ASSERT_TRUE(ret == HAL_RET_OK);
   // ::google::protobuf::uint64 up_hdl = up_rsp.mutable_status()->if_handle();

   ep_t *dummy_ep;
   for (int i = 0; i < MAX_ENDPOINTS; i++) {
       ep_spec.mutable_vrf_key_handle()->set_vrf_id(1);
       ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);


       // Create a lif
       LifSpec                     lif_spec;
       LifResponse                 lif_rsp;
       lif_spec.mutable_key_or_handle()->set_lif_id(40 + i);
       hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
       ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
       hal::hal_cfg_db_close();
       ASSERT_TRUE(ret == HAL_RET_OK);

       InterfaceSpec                   enicif_spec;
       InterfaceResponse               enicif_rsp;
       enicif_spec.set_type(intf::IF_TYPE_ENIC);
       enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(40 + i);
       enicif_spec.mutable_key_or_handle()->set_interface_id(200 + i);
       enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
       enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(1);
       enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(100 + i);
       hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
       ret = hal::interface_create(enicif_spec, &enicif_rsp);
       hal::hal_cfg_db_close();
       ASSERT_TRUE(ret == HAL_RET_OK);

       ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(enicif_rsp.mutable_status()->if_handle());
       ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(
               (mac_addr_base + i));
       //ep_spec.mutable_endpoint_attrs()->add_ip_address();
       hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
       ret = hal::endpoint_create(ep_spec, &ep_rsp);
       hal::hal_cfg_db_close();
       ASSERT_TRUE(ret == HAL_RET_OK);
       ep_handles[i] = ep_rsp.endpoint_status().endpoint_handle();

       dummy_ep = find_ep_by_handle(ep_handles[i]);
       HAL_TRACE_INFO(" SMAC {} ",
                    macaddr2str(dummy_ep->l2_key.mac_addr));
       uint8_t *mac_addr = dummy_ep->l2_key.mac_addr;
       //ep_t *sep = hal::find_ep_by_l2_key(1, (uint8_t*)(mac_addr_base.c_str()));
       //ASSERT_TRUE(sep != nullptr);

       //strcpy((char*)(dummy_ep->l2_key.mac_addr),
       //        (mac_addr_base + std::to_string(i)).c_str());
       mac_addr[0] = mac_addr[0];
   }

}

class arp_fsm_test : public hal_base_test {
   protected:
    arp_fsm_test() {
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
        //hal_test_utils_slab_disable_delete();
        /* Override the timer so that we can control  */
        //hal::periodic::g_twheel = twheel::factory(10, 100, false);
        arp_topo_setup();
        //Reduce the time to avoid thread locking.
        arp_trans_t::set_state_timeout(ARP_BOUND, 1 * TIME_MSECS_PER_MIN);
        arp_trans_t::set_state_timeout(ARP_INIT, 1 * TIME_MSECS_PER_MIN);
    }
};

EthernetII *get_default_arp_packet(ARP::Flags type,
                                   const char *sender_ip_address,
                                   unsigned char *sender_hw_addr,
                                   const char *target_ip_addr,
                                   unsigned char *target_hw_addr) {
    EthernetII *eth = new EthernetII();
    eth->src_addr(sender_hw_addr);
    ARP *arp = new ARP();

    eth->inner_pdu(arp);

    // Retrieve a pointer to the stored TCP PDU
    arp = eth->find_pdu<ARP>();

    arp->opcode(type);

    HWAddress<6> hw_address(sender_hw_addr);
    arp->sender_hw_addr(hw_address);

    if (type == ARPOP_REVREQUEST) {
        HWAddress<6> hw_address_1(sender_hw_addr);
        arp->target_hw_addr(hw_address_1);
    } else {
        HWAddress<6> hw_address_2(target_hw_addr);
        arp->target_hw_addr(hw_address_2);
    }


    arp->sender_ip_addr(sender_ip_address);
    arp->target_ip_addr(target_ip_addr);

    return eth;
}

EthernetII *get_default_neighbor_disc_packet(ICMPv6::Types type,
                                   const char *sender_ip_address,
                                   unsigned char *sender_hw_addr,
                                   const char *target_ip_addr,
                                   unsigned char *target_hw_addr) {
    EthernetII *eth = new EthernetII();
    IPv6 *ipv6 = new IPv6();
    ICMPv6 *neigh_disc = new ICMPv6();
    ICMPv6::option option;

    eth->src_addr(sender_hw_addr);
    eth->dst_addr(sender_hw_addr);
    eth->inner_pdu(ipv6);
    ipv6->inner_pdu(neigh_disc);

    ipv6->src_addr(IPv6Address(sender_ip_address));

    // Retrieve a pointer to the stored TCP PDU
    neigh_disc = ipv6->find_pdu<ICMPv6>();

    neigh_disc->type(type);

    if (type == ICMPv6::NEIGHBOUR_SOLICIT) {
        neigh_disc->target_addr(IPv6Address(target_ip_addr));
        HWAddress<6> hw_address(sender_hw_addr);
        neigh_disc->source_link_layer_addr(hw_address);
    } else if (type == ICMPv6::NEIGHBOUR_ADVERT) {
        neigh_disc->target_addr(IPv6Address(sender_ip_address));
        HWAddress<6> hw_address(sender_hw_addr);
        neigh_disc->target_link_layer_addr(hw_address);
        ipv6->dst_addr(IPv6Address(target_ip_addr));
    }

    ipv6->src_addr(IPv6Address(sender_ip_address));

    return eth;
}


static hal::pd::l2seg_hw_id_t
get_l2seg_hw_id(hal_handle_t l2seg_handle)
{
    hal::l2seg_t *l2seg = hal::l2seg_lookup_by_handle(l2seg_handle);
    //ASSERT_TRUE(l2seg != nullptr);
    hal::pd::pd_l2seg_get_flow_lkupid_args_t args;
    args.l2seg = l2seg;
    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, (void *)&args);
    return args.hwid;
}


hal_ret_t arp_packet_send(hal_handle_t ep_handle,
                     ARP::Flags type, const char *sender_ip_address,
                     unsigned char *sender_hw_addr, const char *target_ip_addr,
                     unsigned char *target_hw_addr = NULL,
                     hal_handle_t dst_ep_handle = 0) {
    EthernetII *eth =
        get_default_arp_packet(type, sender_ip_address, sender_hw_addr,
                               target_ip_addr, target_hw_addr);

    std::vector<uint8_t> buffer = eth->serialize();
    fte::ctx_t ctx;
    ep_t *dst_ep = nullptr;
    hal_ret_t ret;

    ep_t *dummy_ep = find_ep_by_handle(ep_handle);
    if (dst_ep_handle) {
        dst_ep = find_ep_by_handle(dst_ep_handle);
    }
    fte::cpu_rxhdr_t cpu_rxhdr;
    cpu_rxhdr.flags = 0;
    cpu_rxhdr.l2_offset = 0;
    cpu_rxhdr.lkp_vrf = get_l2seg_hw_id(dummy_l2seg_hdl);
    fte::feature_state_t feature_state[100];
    eplearn_info_t info;
    memset(&info, 0, sizeof(info));
    memset(&feature_state, 0, sizeof(feature_state));
    feature_state[fte::feature_id(FTE_FEATURE_EP_LEARN)].ctx_state = &info;
    fte_ctx_init(ctx, dummy_ten,
            dummy_ep, dst_ep, &cpu_rxhdr, &buffer[0], buffer.size(), NULL, NULL, feature_state);
    ctx.set_feature_name(FTE_FEATURE_EP_LEARN.c_str());
    ret = arp_process_packet(ctx);
    ctx.process();
    return ret;
}

hal_ret_t arp_ipv6_packet_send(hal_handle_t ep_handle,
                    ICMPv6::Types type, const char *sender_ip_address,
                     unsigned char *sender_hw_addr, const char *target_ip_addr,
                     unsigned char *target_hw_addr = NULL,
                     hal_handle_t dst_ep_handle = 0) {
    EthernetII *eth = get_default_neighbor_disc_packet(type,
                               sender_ip_address, sender_hw_addr,
                               target_ip_addr, target_hw_addr);
    hal_ret_t ret;

    std::vector<uint8_t> buffer = eth->serialize();
    fte::ctx_t ctx;
    ep_t *dst_ep = nullptr;

    ep_t *dummy_ep = find_ep_by_handle(ep_handle);
    if (dst_ep_handle) {
        dst_ep = find_ep_by_handle(dst_ep_handle);
    }
    fte::cpu_rxhdr_t cpu_rxhdr;
    cpu_rxhdr.l4_offset = L2_ETH_HDR_LEN + sizeof(ipv6_header_t);
    cpu_rxhdr.l3_offset = L2_ETH_HDR_LEN;
    cpu_rxhdr.flags = 0;

    fte::feature_state_t feature_state[100];
    memset(&feature_state, 0, sizeof(feature_state));
    eplearn_info_t info;
    memset(&info, 0, sizeof(info));
    feature_state[fte::feature_id(FTE_FEATURE_EP_LEARN)].ctx_state = &info;
    fte_ctx_init(ctx, dummy_ten,
            dummy_ep, dst_ep, &cpu_rxhdr, &buffer[0], buffer.size(), NULL, NULL, feature_state);
    ctx.set_feature_name(FTE_FEATURE_EP_LEARN.c_str());
    ret = neighbor_disc_process_packet(ctx);
    ctx.process();
    return ret;
}

TEST_F(arp_fsm_test, arp_ipv6_request_response) {
    arp_ipv6_packet_send(ep_handles[4], ICMPv6::Types::NEIGHBOUR_SOLICIT,
                    "fe80::c001:2ff:fe40:0",
                    GET_MAC_ADDR(4), "fe80::c002:3ff:fee4:0");
    arp_trans_key_t key;
    ep_t *dummy_ep = find_ep_by_handle(ep_handles[4]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(4), dummy_ep, ARP_TRANS_IPV6,
                                    &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);

    ip_addr_t ip = {0};
    ip.af = IP_AF_IPV6;
    inet_pton(AF_INET6, "fe80::c001:2ff:fe40:0", &(ip.addr.v6_addr));
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));

    arp_ipv6_packet_send(ep_handles[5], ICMPv6::Types::NEIGHBOUR_ADVERT,
                    "fe80::c002:3ff:fee4:0",
                    GET_MAC_ADDR(5), "fe80::c001:2ff:fe40:0");

    dummy_ep = find_ep_by_handle(ep_handles[5]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(5),
            dummy_ep, ARP_TRANS_IPV6, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    ip.af = IP_AF_IPV6;
    inet_pton(AF_INET6, "fe80::c002:3ff:fee4:0", &(ip.addr.v6_addr));
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));

    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);
    //ASSERT_EQ(arp_trans_t::ep_l3_entry_ht()->num_entries(), 2);
}

TEST_F(arp_fsm_test, arp6_spoofing) {

    arp_ipv6_packet_send(ep_handles[6], ICMPv6::Types::NEIGHBOUR_SOLICIT,
                    "fe80::c001:2ff:fe40:0",
                    GET_MAC_ADDR(4), "fe80::c002:3ff:fee4:0");
    ep_t *dummy_ep = find_ep_by_handle(ep_handles[6]);
    ip_addr_t ip = {0};
    ip.af = IP_AF_IPV6;
    inet_pton(AF_INET6, "fe80::c001:2ff:fe40:0", &(ip.addr.v6_addr));
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));
    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);
    ASSERT_EQ(g_hal_state->ep_l3_entry_ht()->num_entries(), 2);
}

TEST_F(arp_fsm_test, arp_request_response_same_ep_with_already_ndp) {
    arp_packet_send(ep_handles[4], ARP::Flags::REQUEST, "1.1.1.1",
                    GET_MAC_ADDR(4), "1.1.1.2");
    arp_trans_key_t key;
    ep_t *dummy_ep = find_ep_by_handle(ep_handles[4]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(4), dummy_ep,
            ARP_TRANS_IPV4, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);

    ip_addr_t ip = {0};
    inet_pton(AF_INET, "1.1.1.1", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    inet_pton(AF_INET, "1.1.1.2", &(ip.addr.v4_addr));
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));
    arp_packet_send(ep_handles[5], ARP::Flags::REPLY, "1.1.1.2",
                    GET_MAC_ADDR(5), "1.1.1.1", GET_MAC_ADDR(4));
    dummy_ep = find_ep_by_handle(ep_handles[5]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(5), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    inet_pton(AF_INET, "1.1.1.2", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(5), dummy_ep, ARP_TRANS_IPV4,
            &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));

    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 4);
    //ASSERT_EQ(arp_trans_t::ep_l3_entry_ht()->num_entries(), 2);
}

TEST_F(arp_fsm_test, arp6_entry_timeout) {
    arp_trans_key_t key;
    ep_t *dummy_ep = find_ep_by_handle(ep_handles[5]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(5), dummy_ep, ARP_TRANS_IPV6, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);

    sleep(ARP_ENTRY_TIMEOUT + 1);
    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 0);
    ASSERT_EQ(g_hal_state->ep_l3_entry_ht()->num_entries(), 0);
}

TEST_F(arp_fsm_test, arp_request_response) {
    arp_packet_send(ep_handles[0], ARP::Flags::REQUEST, "1.1.1.1",
                    GET_MAC_ADDR(0), "1.1.1.2");
    arp_trans_key_t key;
    ep_t *dummy_ep = find_ep_by_handle(ep_handles[0]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep, ARP_TRANS_IPV4, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);

    ip_addr_t ip = {0};
    inet_pton(AF_INET, "1.1.1.1", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    inet_pton(AF_INET, "1.1.1.2", &(ip.addr.v4_addr));
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));
    arp_packet_send(ep_handles[1], ARP::Flags::REPLY, "1.1.1.2",
                    GET_MAC_ADDR(1), "1.1.1.1", GET_MAC_ADDR(0));
    dummy_ep = find_ep_by_handle(ep_handles[1]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(1), dummy_ep, ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    inet_pton(AF_INET, "1.1.1.2", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));

    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);
    //ASSERT_EQ(arp_trans_t::ep_l3_entry_ht()->num_entries(), 2);
}

TEST_F(arp_fsm_test, arp_request_response_repeat) {
    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);
    ASSERT_EQ(g_hal_state->ep_l3_entry_ht()->num_entries(), 2);
    arp_packet_send(ep_handles[0], ARP::Flags::REQUEST, "1.1.1.1",
                    GET_MAC_ADDR(0), "1.1.1.2");
    arp_trans_key_t key;
    ep_t *dummy_ep = find_ep_by_handle(ep_handles[0]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep,
            ARP_TRANS_IPV4, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);

    ip_addr_t ip = {0};
    inet_pton(AF_INET, "1.1.1.1", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    inet_pton(AF_INET, "1.1.1.2", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));
    arp_packet_send(ep_handles[1], ARP::Flags::REPLY, "1.1.1.2",
                    GET_MAC_ADDR(1), "1.1.1.1", GET_MAC_ADDR(0));
    dummy_ep = find_ep_by_handle(ep_handles[1]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(1), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    inet_pton(AF_INET, "1.1.1.2", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));

    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);
    //ASSERT_EQ(arp_trans_t::ep_l3_entry_ht()->num_entries(), 2);
}

TEST_F(arp_fsm_test, arp_spoofing) {

    hal_ret_t ret = arp_packet_send(ep_handles[0],
                    ARP::Flags::REQUEST, "1.1.1.10",
                    GET_MAC_ADDR(1), "1.1.1.2");

    ASSERT_TRUE(ret != HAL_RET_OK);
    ep_t *dummy_ep = find_ep_by_handle(ep_handles[0]);
    ip_addr_t ip = {0};
    inet_pton(AF_INET, "1.1.1.10", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));

    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);
}

TEST_F(arp_fsm_test, arp_entry_timeout) {
    arp_packet_send(ep_handles[0], ARP::Flags::REQUEST, "1.1.1.1",
                    GET_MAC_ADDR(0), "1.1.1.2");
    arp_trans_key_t key;
    ep_t *dummy_ep = find_ep_by_handle(ep_handles[0]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep,
            ARP_TRANS_IPV4, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);

    sleep(ARP_ENTRY_TIMEOUT + 1);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry == NULL);
    dummy_ep = find_ep_by_handle(ep_handles[1]);
    ip_addr_t ip = {0};
    inet_pton(AF_INET, "1.1.1.1", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));
}

TEST_F(arp_fsm_test, arp_request_response_again) {
    arp_packet_send(ep_handles[0], ARP::Flags::REQUEST, "1.1.1.1",
                    GET_MAC_ADDR(0), "1.1.1.2");
    arp_trans_key_t key;
    ep_t *dummy_ep = find_ep_by_handle(ep_handles[0]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep,
            ARP_TRANS_IPV4, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);

    ip_addr_t ip = {0};
    inet_pton(AF_INET, "1.1.1.1", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    inet_pton(AF_INET, "1.1.1.2", &(ip.addr.v4_addr));
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));
    arp_packet_send(ep_handles[1], ARP::Flags::REPLY, "1.1.1.2",
                    GET_MAC_ADDR(1), "1.1.1.1", GET_MAC_ADDR(0));
    dummy_ep = find_ep_by_handle(ep_handles[1]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(1), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    inet_pton(AF_INET, "1.1.1.2", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));

    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);
    ASSERT_EQ(g_hal_state->ep_l3_entry_ht()->num_entries(), 2);

    delete entry;
}


TEST_F(arp_fsm_test, arp_request_response_replace_with_different_ep) {

    arp_packet_send(ep_handles[0], ARP::Flags::REQUEST, "1.1.1.1",
                    GET_MAC_ADDR(0), "1.1.1.2");
    arp_trans_key_t key;
    ep_t *dummy_ep = find_ep_by_handle(ep_handles[0]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep,
            ARP_TRANS_IPV4, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    arp_packet_send(ep_handles[1], ARP::Flags::REPLY, "1.1.1.2",
                    GET_MAC_ADDR(1), "1.1.1.1", GET_MAC_ADDR(0));
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(1), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));

    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);
    ASSERT_EQ(g_hal_state->ep_l3_entry_ht()->num_entries(), 2);

    //Lets replace both.
    arp_packet_send(ep_handles[2],
                    ARP::Flags::REQUEST, "1.1.1.1",
                    GET_MAC_ADDR(2), "1.1.1.2");
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(2), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);

    arp_packet_send(ep_handles[3], ARP::Flags::REPLY, "1.1.1.2",
                    GET_MAC_ADDR(3), "1.1.1.1", GET_MAC_ADDR(2));
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(3), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);
    dummy_ep = find_ep_by_handle(ep_handles[0]);
    ip_addr_t ip = {0};
    inet_pton(AF_INET, "1.1.1.1", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));
    dummy_ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, "1.1.1.2", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));
}

TEST_F(arp_fsm_test, arp_request_response_replace_with_different_ip_same_ep) {

    arp_packet_send(ep_handles[0], ARP::Flags::REQUEST, "1.1.1.1",
                    GET_MAC_ADDR(0), "1.1.1.2");
    arp_trans_key_t key;
    ep_t *dummy_ep = find_ep_by_handle(ep_handles[0]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep,
            ARP_TRANS_IPV4, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    arp_packet_send(ep_handles[1], ARP::Flags::REPLY, "1.1.1.2",
                    GET_MAC_ADDR(1), "1.1.1.1", GET_MAC_ADDR(0));
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(1), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));

    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);
    //ASSERT_EQ(arp_trans_t::ep_l3_entry_ht()->num_entries(), 2);

    //Lets replace both with different IP.
    arp_packet_send(ep_handles[0],
                    ARP::Flags::REQUEST, "1.1.1.3",
                    GET_MAC_ADDR(0), "1.1.1.4");
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);

    arp_packet_send(ep_handles[1], ARP::Flags::REPLY, "1.1.1.4",
                    GET_MAC_ADDR(1), "1.1.1.3", GET_MAC_ADDR(0));
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(1), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);
    ASSERT_EQ(arp_trans_t::arplearn_ip_entry_ht()->num_entries(), 2);

    dummy_ep = find_ep_by_handle(ep_handles[0]);
    ip_addr_t ip = {0};
    inet_pton(AF_INET, "1.1.1.3", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    inet_pton(AF_INET, "1.1.1.1", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));
    inet_pton(AF_INET, "1.1.1.4", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    dummy_ep = find_ep_by_handle(ep_handles[1]);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    inet_pton(AF_INET, "1.1.1.2", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));

    //Lets replace back both with different IP.
    arp_packet_send(ep_handles[0],
                    ARP::Flags::REQUEST, "1.1.1.1",
                    GET_MAC_ADDR(0), "1.1.1.2");
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(0), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);

    arp_packet_send(ep_handles[1], ARP::Flags::REPLY, "1.1.1.2",
                    GET_MAC_ADDR(1), "1.1.1.1", GET_MAC_ADDR(0));
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(1), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 2);
    ASSERT_EQ(arp_trans_t::arplearn_ip_entry_ht()->num_entries(), 2);

    dummy_ep = find_ep_by_handle(ep_handles[0]);
    ip = {0};
    inet_pton(AF_INET, "1.1.1.1", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    inet_pton(AF_INET, "1.1.1.3", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));
    inet_pton(AF_INET, "1.1.1.2", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    dummy_ep = find_ep_by_handle(ep_handles[1]);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));
    inet_pton(AF_INET, "1.1.1.4", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));
}

TEST_F(arp_fsm_test, rarp_request_response) {
    arp_packet_send(ep_handles[2], (ARP::Flags)3, "2.2.2.2",
                    GET_MAC_ADDR(2), "2.2.2.1");
    arp_trans_key_t key;
    ep_t *dummy_ep = find_ep_by_handle(ep_handles[2]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(2), dummy_ep,
            ARP_TRANS_IPV4, &key);
    arp_trans_t *entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);

    ip_addr_t ip = {0};
    inet_pton(AF_INET, "2.2.2.2", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_FALSE(ip_in_ep(&ip, dummy_ep, NULL));
    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 3);

    arp_packet_send(ep_handles[3], (ARP::Flags)4, "2.2.2.1",
                    GET_MAC_ADDR(3), "2.2.2.2", GET_MAC_ADDR(2),
                    ep_handles[2]);
    dummy_ep = find_ep_by_handle(ep_handles[2]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(2), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    inet_pton(AF_INET, "2.2.2.2", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));

    /* Make sure RARP server entry is also added */
    dummy_ep = find_ep_by_handle(ep_handles[3]);
    arp_trans_t::init_arp_trans_key(GET_MAC_ADDR(3), dummy_ep,
            ARP_TRANS_IPV4, &key);
    entry = reinterpret_cast<arp_trans_t *>(
        arp_trans_t::arplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(entry != NULL);
    inet_pton(AF_INET, "2.2.2.1", &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, dummy_ep, NULL));

    ASSERT_EQ(arp_trans_t::arplearn_key_ht()->num_entries(), 4);
    ASSERT_EQ(g_hal_state->ep_l3_entry_ht()->num_entries(), 4);
}
