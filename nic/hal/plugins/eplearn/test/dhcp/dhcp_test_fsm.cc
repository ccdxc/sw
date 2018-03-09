#include <arpa/inet.h>
#include <gtest/gtest.h>
#include "sdk/twheel.hpp"
// clang-format off
#include "nic/hal/plugins/eplearn/dhcp/dhcp_packet.hpp"
#include "nic/hal/plugins/eplearn/dhcp/dhcp_learn.hpp"
#include "nic/hal/plugins/eplearn/dhcp/dhcp_trans.hpp"
#include "nic/hal/plugins/eplearn/eplearn.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"

#include "nic/hal/src/interface.hpp"
#include "nic/hal/src/endpoint.hpp"
#include "nic/hal/src/session.hpp"
#include "nic/hal/src/nw.hpp"
#include "nic/gen/proto/hal/interface.pb.h"
#include "nic/gen/proto/hal/l2segment.pb.h"
#include "nic/gen/proto/hal/vrf.pb.h"
#include "nic/gen/proto/hal/endpoint.pb.h"
#include "nic/gen/proto/hal/nw.pb.h"
// clang-format on

#include <tins/tins.h>

using namespace Tins;
using namespace hal::eplearn;
using sdk::lib::twheel;
using namespace hal;

twheel *hal_twheel;
/*
 * Note: For now timer wheel is supposed to be non-thread safe.
 * Reason: Update tick locks that slice and calls the callback function.
 * During the callback function, we try to add or delete timer which will
 * obviously wait indefinitely as there is not lock available.
 */
namespace hal {
extern hal_state    *g_hal_state;
namespace periodic {
twheel *g_twheel;
}
}  // namespace hal

vrf_t *dummy_ten;
#define MAX_ENDPOINTS 4
hal_handle_t ep_handles[MAX_ENDPOINTS];
string mac_addr_base = "12345";
#define GET_MAC_ADDR(_ep) ((unsigned char*)((mac_addr_base + std::to_string(_ep)).c_str()))

hal_handle_t *dhcp_server_ep = &ep_handles[MAX_ENDPOINTS - 1];

void fte_ctx_init(fte::ctx_t &ctx, hal::vrf_t *ten, hal::ep_t *ep,
        hal::ep_t *dep, fte::cpu_rxhdr_t *cpu_rxhdr,
        uint8_t *pkt, size_t pkt_len,
        fte::flow_t iflow[], fte::flow_t rflow[], fte::feature_state_t feature_state[]);


void dhcp_topo_setup()
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
    InterfaceSpec               enicif_spec;
    InterfaceResponse           enicif_rsp;
    LifSpec                     lif_spec;
    LifResponse                 lif_rsp;


    // Create vrf
    ten_spec.mutable_key_or_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::vrf_create(ten_spec, &ten_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);

    // Create network
    nw_spec.mutable_vrf_key_handle()->set_vrf_id(1);;
    nw_spec.set_rmac(0x0000DEADBEEE);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0a000000);
    nw_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl = nw_rsp.mutable_status()->nw_handle();

    nw_spec1.mutable_vrf_key_handle()->set_vrf_id(1);
    nw_spec1.set_rmac(0x0000DEADBEEF);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->set_prefix_len(24);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_ip_af(types::IP_AF_INET);
    nw_spec1.mutable_key_or_handle()->mutable_ip_prefix()->mutable_address()->set_v4_addr(0x0b000000);
    nw_spec1.mutable_vrf_key_handle()->set_vrf_id(1);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::network_create(nw_spec1, &nw_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t nw_hdl1 = nw_rsp.mutable_status()->nw_handle();

    // Create L2 Segment
    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    nkh =l2seg_spec.add_network_key_handle();
    nkh->set_nw_handle(nw_hdl);
    l2seg_spec.mutable_key_or_handle()->set_segment_id(1);
    l2seg_spec.mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    l2seg_spec.mutable_wire_encap()->set_encap_value(11);
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    ret = hal::l2segment_create(l2seg_spec, &l2seg_rsp);
    hal::hal_cfg_db_close();
    ASSERT_TRUE(ret == HAL_RET_OK);
    uint64_t l2seg_hdl = l2seg_rsp.mutable_l2segment_status()->l2segment_handle();

    l2seg_spec.mutable_vrf_key_handle()->set_vrf_id(1);
    nkh =l2seg_spec.add_network_key_handle();
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




    dummy_ten = vrf_lookup_by_id(1);
    ASSERT_TRUE(dummy_ten != NULL);

    //ep_t *dummy_ep;
    for (int i = 0; i < MAX_ENDPOINTS; i++) {

        // Create a lif
        lif_spec.mutable_key_or_handle()->set_lif_id(i + 1);
        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::lif_create(lif_spec, &lif_rsp, NULL);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);

        enicif_spec.set_type(intf::IF_TYPE_ENIC);
        enicif_spec.mutable_if_enic_info()->mutable_lif_key_or_handle()->set_lif_id(i + 1);
        enicif_spec.mutable_key_or_handle()->set_interface_id(i + 100);
        enicif_spec.mutable_if_enic_info()->set_enic_type(intf::IF_ENIC_TYPE_USEG);
        enicif_spec.mutable_if_enic_info()->mutable_enic_info()->mutable_l2segment_key_handle()->set_segment_id(2);
        enicif_spec.mutable_if_enic_info()->mutable_enic_info()->set_encap_vlan_id(2);

        hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
        ret = hal::interface_create(enicif_spec, &up_rsp);
        hal::hal_cfg_db_close();
        ASSERT_TRUE(ret == HAL_RET_OK);
        ::google::protobuf::uint64 up_hdl2 = up_rsp.mutable_status()->if_handle();

       ep_spec.mutable_vrf_key_handle()->set_vrf_id(1);;
       ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->mutable_l2segment_key_handle()->set_l2segment_handle(l2seg_hdl);
       ep_spec.mutable_endpoint_attrs()->mutable_interface_key_handle()->set_if_handle(up_hdl2);
       ep_spec.mutable_key_or_handle()->mutable_endpoint_key()->mutable_l2_key()->set_mac_address(0x00000000ABCD + i);
       //ep_spec.mutable_endpoint_attrs()->add_ip_address();
       hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
       ret = hal::endpoint_create(ep_spec, &ep_rsp);
       hal::hal_cfg_db_close();
       ASSERT_TRUE(ret == HAL_RET_OK);
       ep_handles[i] = ep_rsp.endpoint_status().endpoint_handle();
       //dummy_ep = find_ep_by_handle(ep_handles[i]);
       //strcpy((char*)(dummy_ep->l2_key.mac_addr),
         //      (mac_addr_base + std::to_string(i)).c_str());
    }

}

class dhcp_fsm_test : public hal_base_test {

   protected:
    dhcp_fsm_test() {
        dhcp_init();
    }
    virtual ~dhcp_fsm_test() {}


    // will be called immediately after the constructor before each test
    virtual void SetUp() {
        hal_twheel = twheel::factory(10, 100, false);
        ASSERT_TRUE(hal_twheel != NULL);
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {}

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        hal_base_test::SetUpTestCase();
        dhcp_lib_init();
        dhcp_topo_setup();
    }
};

EthernetII *get_default_dhcp_packet(DHCP::Flags type, uint32_t xid,
                                    unsigned char *chaddr, const char *yiaddr) {
    EthernetII *eth = new EthernetII();
    IP *ip = new IP();
    UDP *udp = new UDP();
    DHCP *dhcp = new DHCP();

    udp->inner_pdu(dhcp);
    ip->inner_pdu(udp);
    eth->inner_pdu(ip);

    // Retrieve a pointer to the stored TCP PDU
    dhcp = eth->find_pdu<DHCP>();

    dhcp->type(type);
    dhcp->xid(xid);
    dhcp->htype(1);
    dhcp->hlen(6);
    dhcp->secs(5);
    dhcp->ciaddr("1.1.1.1");
    if (yiaddr) {
        dhcp->yiaddr(yiaddr);
    }
    dhcp->siaddr("3.3.3.3");
    dhcp->giaddr("4.4.4.4");
    HWAddress<6> hw_address(chaddr);
    dhcp->chaddr(hw_address);
    const char *sname = "SNAME";
    dhcp->sname((const uint8_t *)sname);
    const char *fname = "FNAME";
    dhcp->file((const uint8_t *)fname);

    return eth;
}

void dhcp_packet_send(hal_handle_t ep_handle,
                      DHCP::Flags type, uint32_t xid, unsigned char *chaddr,
                      const char *yiaddr = NULL,
                      std::vector<DHCP::option> *options = NULL) {
    EthernetII *eth = get_default_dhcp_packet(type, xid, chaddr, yiaddr);
    DHCP *dhcp = eth->find_pdu<DHCP>();

    if (options) {
        for (std::vector<DHCP::option>::iterator it = options->begin();
             it != options->end(); ++it) {
            dhcp->add_option(*it);
        }
    }

    std::vector<uint8_t> buffer = eth->serialize();
    fte::ctx_t ctx;
    ep_t *dummy_ep = find_ep_by_handle(ep_handle);
    fte::cpu_rxhdr_t cpu_hdr;
    cpu_hdr.flags = 0;
    cpu_hdr.l3_offset = L2_ETH_HDR_LEN;

    fte::feature_state_t feature_state[100];
    eplearn_info_t info;
    memset(&info, 0, sizeof(info));
    memset(&feature_state, 0, sizeof(feature_state));
    feature_state[fte::feature_id(FTE_FEATURE_EP_LEARN)].ctx_state = &info;
    fte_ctx_init(ctx, dummy_ten,
            dummy_ep, dummy_ep, &cpu_hdr, &buffer[0], buffer.size(), NULL, NULL, feature_state);
    ctx.set_feature_name(FTE_FEATURE_EP_LEARN.c_str());
    hal_ret_t ret = dhcp_process_packet(ctx);
    ASSERT_EQ(ret, HAL_RET_OK);
    ctx.process();
}

static void setup_basic_dhcp_session(hal_handle_t ep_handle,
                                     uint32_t xid, unsigned char *chaddr,
                                     const char *ip_address,
                                     bool discover = true) {
    std::vector<DHCP::option> options;
    const dhcp_ctx *ctx;
    dhcp_trans_t *trans;
    dhcp_trans_key_t key;
    trans_ip_entry_key_t ip_key;

    ep_t *ep = find_ep_by_handle(ep_handle);
    dhcp_trans_t::init_dhcp_trans_key(chaddr, xid, ep, &key);
    if (discover) {
        dhcp_packet_send(ep_handle,
                DHCP::Flags::DISCOVER, xid, chaddr);

        dhcp_trans_t *trans = reinterpret_cast<dhcp_trans_t *>(
            dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
        ASSERT_TRUE(trans != NULL);

        ctx = trans->get_ctx();
        ASSERT_EQ(memcmp(ctx->chaddr_, chaddr, ETH_ADDR_LEN), 0);
        ASSERT_EQ(trans->get_state(), DHCP_SELECTING);

        dhcp_packet_send(*dhcp_server_ep,
                DHCP::Flags::OFFER, xid, chaddr);
        trans = reinterpret_cast<dhcp_trans_t *>(
            dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
        ASSERT_TRUE(trans != NULL);
        ctx = trans->get_ctx();
        ASSERT_EQ(memcmp(ctx->chaddr_, chaddr, ETH_ADDR_LEN), 0);
        ASSERT_EQ(trans->get_state(), DHCP_SELECTING);
    }
    std::vector<uint8_t> server_identifier = {4, 3, 2, 1};
    // Create a DHCP::option
    DHCP::option server_id_opt(DHCP::DHCP_SERVER_IDENTIFIER,
                               server_identifier.begin(),
                               server_identifier.end());
    options.push_back(server_id_opt);

    dhcp_packet_send(ep_handle,
            DHCP::Flags::REQUEST, xid, chaddr, NULL, &options);

    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans != NULL);
    ctx = trans->get_ctx();
    ASSERT_EQ(memcmp(ctx->chaddr_, chaddr, ETH_ADDR_LEN), 0);
    ASSERT_EQ(trans->get_state(), DHCP_REQUESTING);
    ASSERT_EQ(memcmp(&ctx->server_identifer_, &server_identifier[0],
                     server_identifier.size()),
              0);

    options.clear();
    const char *ip_addr = ip_address;
    std::vector<uint8_t> lease_time = {3, 2, 1, 0};
    DHCP::option lease_time_opt(DHCP::DHCP_LEASE_TIME, lease_time.begin(),
                                lease_time.end());
    options.push_back(lease_time_opt);
    DHCP::option renewal_time_opt(DHCP::DHCP_RENEWAL_TIME, lease_time.begin(),
                                  lease_time.end());
    options.push_back(renewal_time_opt);
    DHCP::option rebinding_time_opt(DHCP::DHCP_REBINDING_TIME,
                                    lease_time.begin(), lease_time.end());
    options.push_back(rebinding_time_opt);
    dhcp_packet_send(*dhcp_server_ep,
            DHCP::Flags::ACK, xid, chaddr, ip_addr, &options);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans != NULL);
    ASSERT_EQ(trans->get_state(), DHCP_BOUND);
    ctx = trans->get_ctx();
    ASSERT_EQ(memcmp(ctx->chaddr_, chaddr, ETH_ADDR_LEN), 0);
    struct in_addr yiaddr;
    inet_pton(AF_INET, ip_addr, &(yiaddr.s_addr));
    ASSERT_EQ(memcmp(&ctx->yiaddr_, &yiaddr.s_addr, sizeof(yiaddr)), 0);
    ASSERT_EQ(
        memcmp(&ctx->lease_time_, &lease_time[0], sizeof(ctx->lease_time_)), 0);
    ASSERT_EQ(
        memcmp(&ctx->renewal_time_, &lease_time[0], sizeof(ctx->renewal_time_)),
        0);
    ASSERT_EQ(memcmp(&ctx->rebinding_time_, &lease_time[0],
                     sizeof(ctx->rebinding_time_)),
              0);
    ip_addr_t ip_new_addr = {0};
    ip_new_addr.addr.v4_addr = ntohl(yiaddr.s_addr);
    dhcp_trans_t::init_ip_entry_key(&(ip_new_addr),
                                    dummy_ten->vrf_id, &ip_key);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_ip_entry_ht()->lookup(&ip_key));
    ASSERT_TRUE(trans != NULL);
}

TEST_F(dhcp_fsm_test, dhcp_basic_discover_request_offer) {
    const char *ip_address = "192.168.1.10";
    uint32_t xid = 1234;
    dhcp_trans_t *trans;
    dhcp_trans_key_t key;
    ep_t *ep = find_ep_by_handle(ep_handles[0]);
    dhcp_trans_t::init_dhcp_trans_key(ep->l2_key.mac_addr, xid, ep, &key);


    setup_basic_dhcp_session(ep_handles[0], xid,
            ep->l2_key.mac_addr, ip_address);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans != NULL);
    delete trans;
    ASSERT_EQ(dhcp_trans_t::dhcplearn_key_ht()->num_entries(), 0);
    ASSERT_EQ(dhcp_trans_t::dhcplearn_ip_entry_ht()->num_entries(), 0);

    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[0]);
    ip_addr_t ip = {0};
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ASSERT_TRUE(!ip_in_ep(&ip, ep, NULL));
}

TEST_F(dhcp_fsm_test, dhcp_basic_request_delete) {
    const char *ip_address = "192.168.1.11";
    uint32_t xid = 1234;
    dhcp_trans_t *trans;
    dhcp_trans_key_t key;
    ep_t *ep = find_ep_by_handle(ep_handles[1]);
    dhcp_trans_t::init_dhcp_trans_key(ep->l2_key.mac_addr, xid, ep, &key);

    setup_basic_dhcp_session(ep_handles[1], xid,
            ep->l2_key.mac_addr, ip_address);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans != NULL);
    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[1]);
    ip_addr_t ip = {0};
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, ep, NULL));
    delete trans;
    ASSERT_EQ(dhcp_trans_t::dhcplearn_key_ht()->num_entries(), 0);
    ASSERT_EQ(dhcp_trans_t::dhcplearn_ip_entry_ht()->num_entries(), 0);
    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(!ip_in_ep(&ip, ep, NULL));
}

TEST_F(dhcp_fsm_test, dhcp_basic_request_release) {
    const char *ip_address = "192.168.1.12";
    uint32_t xid = 1234;
    std::vector<DHCP::option> options;
    dhcp_trans_t *trans;
    dhcp_trans_key_t key;
    ep_t *ep = find_ep_by_handle(ep_handles[1]);
    dhcp_trans_t::init_dhcp_trans_key(ep->l2_key.mac_addr, xid, ep, &key);

    setup_basic_dhcp_session(ep_handles[1], xid,
            ep->l2_key.mac_addr, ip_address);
    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[1]);
    ip_addr_t ip = {0};
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, ep, NULL));

    std::vector<uint8_t> server_identifier = {4, 3, 2, 1};
    // Create a DHCP::option
    DHCP::option server_id_opt(DHCP::DHCP_SERVER_IDENTIFIER,
                               server_identifier.begin(),
                               server_identifier.end());
    options.push_back(server_id_opt);

    dhcp_packet_send(ep_handles[1],
            DHCP::Flags::RELEASE, xid, ep->l2_key.mac_addr,
                NULL, &options);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans == NULL);
    ASSERT_EQ(dhcp_trans_t::dhcplearn_key_ht()->num_entries(), 0);
    ASSERT_EQ(dhcp_trans_t::dhcplearn_ip_entry_ht()->num_entries(), 0);
    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(!ip_in_ep(&ip, ep, NULL));
}

TEST_F(dhcp_fsm_test, dhcp_inform) {
    const char *ip_address = "192.168.1.13";
    uint32_t xid = 1234;
    std::vector<DHCP::option> options;
    const dhcp_ctx *ctx;
    dhcp_trans_t *trans;
    dhcp_trans_key_t key;
    ep_t *ep = find_ep_by_handle(ep_handles[1]);
    dhcp_trans_t::init_dhcp_trans_key(ep->l2_key.mac_addr, xid, ep, &key);
    ip_addr_t ip = {0};

    std::vector<uint8_t> server_identifier = {4, 3, 2, 1};
    // Create a DHCP::option
    DHCP::option server_id_opt(DHCP::DHCP_SERVER_IDENTIFIER,
                               server_identifier.begin(),
                               server_identifier.end());
    options.push_back(server_id_opt);

    dhcp_packet_send(ep_handles[1],
            DHCP::Flags::INFORM, xid,
            ep->l2_key.mac_addr, ip_address, &options);

    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans != NULL);
    ctx = trans->get_ctx();
    ASSERT_EQ(memcmp(ctx->chaddr_,
            ep->l2_key.mac_addr, ETH_ADDR_LEN), 0);
    ASSERT_EQ(trans->get_state(), DHCP_BOUND);
    ASSERT_EQ(memcmp(&ctx->server_identifer_, &server_identifier[0],
                     server_identifier.size()),
              0);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans != NULL);
    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, ep, NULL));

    delete trans;
    ASSERT_EQ(dhcp_trans_t::dhcplearn_key_ht()->num_entries(), 0);
    ASSERT_EQ(dhcp_trans_t::dhcplearn_ip_entry_ht()->num_entries(), 0);
    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(!ip_in_ep(&ip, ep, NULL));
}

TEST_F(dhcp_fsm_test, dhcp_inform_decline) {
    uint32_t xid = 1234;
    const char *ip_address = "192.168.1.14";
    std::vector<DHCP::option> options;
    dhcp_trans_t *trans;
    const dhcp_ctx *ctx;
    dhcp_trans_key_t key;
    ep_t *ep = find_ep_by_handle(ep_handles[1]);
    dhcp_trans_t::init_dhcp_trans_key(ep->l2_key.mac_addr, xid, ep, &key);
    ip_addr_t ip = {0};

    std::vector<uint8_t> server_identifier = {4, 3, 2, 1};
    // Create a DHCP::option
    DHCP::option server_id_opt(DHCP::DHCP_SERVER_IDENTIFIER,
                               server_identifier.begin(),
                               server_identifier.end());
    options.push_back(server_id_opt);

    dhcp_packet_send(ep_handles[1],
            DHCP::Flags::INFORM, xid,
            ep->l2_key.mac_addr, ip_address, &options);

    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans != NULL);
    ctx = trans->get_ctx();
    ASSERT_EQ(memcmp(ctx->chaddr_, ep->l2_key.mac_addr, ETH_ADDR_LEN), 0);
    ASSERT_EQ(trans->get_state(), DHCP_BOUND);
    ASSERT_EQ(memcmp(&ctx->server_identifer_, &server_identifier[0],
                     server_identifier.size()),
              0);
    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, ep, NULL));

    dhcp_packet_send(*dhcp_server_ep,
            DHCP::Flags::DECLINE, xid, ep->l2_key.mac_addr);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans == NULL);
    ASSERT_EQ(dhcp_trans_t::dhcplearn_key_ht()->num_entries(), 0);
    ASSERT_EQ(dhcp_trans_t::dhcplearn_ip_entry_ht()->num_entries(), 0);
    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(!ip_in_ep(&ip, ep, NULL));
}

TEST_F(dhcp_fsm_test, dhcp_basic_offer_renew) {
    const char *ip_address = "192.168.1.15";
    uint32_t xid = 1234;
    std::vector<DHCP::option> options;
    dhcp_trans_t *trans;
    const dhcp_ctx *ctx;
    dhcp_trans_key_t key;
    trans_ip_entry_key_t ip_key;
    ep_t *ep = find_ep_by_handle(ep_handles[1]);
    dhcp_trans_t::init_dhcp_trans_key(ep->l2_key.mac_addr, xid, ep, &key);
    ip_addr_t ip = {0};


    setup_basic_dhcp_session(ep_handles[1],
            xid, ep->l2_key.mac_addr, ip_address);
    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, ep, NULL));

    std::vector<uint8_t> server_identifier = {4, 3, 2, 1};
    // Create a DHCP::option
    DHCP::option server_id_opt(DHCP::DHCP_SERVER_IDENTIFIER,
                               server_identifier.begin(),
                               server_identifier.end());
    options.push_back(server_id_opt);

    dhcp_packet_send(ep_handles[1],
            DHCP::Flags::REQUEST, xid, ep->l2_key.mac_addr, NULL, &options);

    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans != NULL);
    ctx = trans->get_ctx();
    ASSERT_EQ(memcmp(ctx->chaddr_, ep->l2_key.mac_addr, ETH_ADDR_LEN), 0);
    ASSERT_EQ(trans->get_state(), DHCP_RENEWING);
    ASSERT_EQ(memcmp(&ctx->server_identifer_, &server_identifier[0],
                     server_identifier.size()),
              0);
    options.clear();
    const char *renew_ip_address = "192.168.1.16";
    std::vector<uint8_t> lease_time = {3, 2, 1, 0};
    DHCP::option lease_time_opt(DHCP::DHCP_LEASE_TIME, lease_time.begin(),
                                lease_time.end());
    options.push_back(lease_time_opt);
    DHCP::option renewal_time_opt(DHCP::DHCP_RENEWAL_TIME, lease_time.begin(),
                                  lease_time.end());
    options.push_back(renewal_time_opt);
    DHCP::option rebinding_time_opt(DHCP::DHCP_REBINDING_TIME,
                                    lease_time.begin(), lease_time.end());
    options.push_back(rebinding_time_opt);

    /* Make sure no old IP address */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(!ip_in_ep(&ip, ep, NULL));

    dhcp_packet_send(*dhcp_server_ep,
            DHCP::Flags::ACK, xid, ep->l2_key.mac_addr,
            renew_ip_address, &options);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans != NULL);
    ASSERT_EQ(trans->get_state(), DHCP_BOUND);

    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    dhcp_trans_t::init_ip_entry_key(&(ip),
                                    dummy_ten->vrf_id, &ip_key);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_ip_entry_ht()->lookup(&ip_key));
    ASSERT_TRUE(trans == NULL);


    inet_pton(AF_INET, renew_ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    dhcp_trans_t::init_ip_entry_key(&ip,
                                    dummy_ten->vrf_id, &ip_key);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_ip_entry_ht()->lookup(&ip_key));
    ASSERT_TRUE(trans != NULL);

    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, renew_ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, ep, NULL));

    /* Make sure no old IP address */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(!ip_in_ep(&ip, ep, NULL));
    delete trans;
    ASSERT_EQ(dhcp_trans_t::dhcplearn_key_ht()->num_entries(), 0);
    ASSERT_EQ(dhcp_trans_t::dhcplearn_ip_entry_ht()->num_entries(), 0);
    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, renew_ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(!ip_in_ep(&ip, ep, NULL));
}

TEST_F(dhcp_fsm_test, dhcp_basic_offer_renew_nack) {
    const char *ip_address = "192.168.1.17";
    uint32_t xid = 1234;
    std::vector<DHCP::option> options;
    dhcp_trans_t *trans;
    const dhcp_ctx *ctx;
    dhcp_trans_key_t key;
    ep_t *ep = find_ep_by_handle(ep_handles[1]);
    dhcp_trans_t::init_dhcp_trans_key(ep->l2_key.mac_addr, xid, ep, &key);
    ip_addr_t ip = {0};

    setup_basic_dhcp_session(ep_handles[1],
            xid, ep->l2_key.mac_addr, ip_address);
    /* Check IP in EP */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(ip_in_ep(&ip, ep, NULL));

    std::vector<uint8_t> server_identifier = {4, 3, 2, 1};
    // Create a DHCP::option
    DHCP::option server_id_opt(DHCP::DHCP_SERVER_IDENTIFIER,
                               server_identifier.begin(),
                               server_identifier.end());
    options.push_back(server_id_opt);

    dhcp_packet_send(ep_handles[1],
            DHCP::Flags::REQUEST, xid, ep->l2_key.mac_addr, NULL, &options);

    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans != NULL);
    ctx = trans->get_ctx();
    ASSERT_EQ(memcmp(ctx->chaddr_, ep->l2_key.mac_addr, ETH_ADDR_LEN), 0);
    ASSERT_EQ(trans->get_state(), DHCP_RENEWING);
    ASSERT_EQ(memcmp(&ctx->server_identifer_, &server_identifier[0],
                     server_identifier.size()),
              0);
    options.clear();
    const char *renew_ip_addr = "192.168.1.10";
    std::vector<uint8_t> lease_time = {3, 2, 1, 0};
    DHCP::option lease_time_opt(DHCP::DHCP_LEASE_TIME, lease_time.begin(),
                                lease_time.end());
    options.push_back(lease_time_opt);
    DHCP::option renewal_time_opt(DHCP::DHCP_RENEWAL_TIME, lease_time.begin(),
                                  lease_time.end());
    options.push_back(renewal_time_opt);
    DHCP::option rebinding_time_opt(DHCP::DHCP_REBINDING_TIME,
                                    lease_time.begin(), lease_time.end());
    options.push_back(rebinding_time_opt);
    /* Make sure no old IP address */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(!ip_in_ep(&ip, ep, NULL));

    dhcp_packet_send(*dhcp_server_ep,
            DHCP::Flags::NAK, xid, ep->l2_key.mac_addr,
            renew_ip_addr, &options);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans == NULL);
    ASSERT_EQ(dhcp_trans_t::dhcplearn_key_ht()->num_entries(), 0);
    ASSERT_EQ(dhcp_trans_t::dhcplearn_ip_entry_ht()->num_entries(), 0);
    /* Make sure no old IP address */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, renew_ip_addr, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(!ip_in_ep(&ip, ep, NULL));

}


TEST_F(dhcp_fsm_test, dhcp_basic_bound_timeout) {
    const char *ip_address = "192.168.1.18";
    uint32_t xid = 1234;
    dhcp_trans_t *trans;
    dhcp_trans_key_t key;
    ep_t *ep = find_ep_by_handle(ep_handles[1]);
    dhcp_trans_t::init_dhcp_trans_key(ep->l2_key.mac_addr, xid, ep, &key);
    ip_addr_t ip = {0};

    setup_basic_dhcp_session(ep_handles[1],
            xid, ep->l2_key.mac_addr, ip_address);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans != NULL);

    sleep(1);
    hal::periodic::g_twheel->tick(trans->get_current_state_timeout() + 100);
    sleep(2.5);
    trans = reinterpret_cast<dhcp_trans_t *>(
        dhcp_trans_t::dhcplearn_key_ht()->lookup(&key));
    ASSERT_TRUE(trans == NULL);
    ASSERT_EQ(dhcp_trans_t::dhcplearn_key_ht()->num_entries(), 0);
    ASSERT_EQ(dhcp_trans_t::dhcplearn_ip_entry_ht()->num_entries(), 0);
    /* Make sure no old IP address */
    ep = find_ep_by_handle(ep_handles[1]);
    inet_pton(AF_INET, ip_address, &(ip.addr.v4_addr));
    ip.addr.v4_addr = ntohl(ip.addr.v4_addr);
    ASSERT_TRUE(!ip_in_ep(&ip, ep, NULL));
}
