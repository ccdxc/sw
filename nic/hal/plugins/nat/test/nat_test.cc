#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/include/pd_api.hpp"
#include "nic/include/fte.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"

#include <tins/tins.h>

using namespace fte;

class nat_test : public fte_base_test {
protected:
    nat_test() {}

    virtual ~nat_test() {}

    // will be called immediately after the constructor before each test
    virtual void SetUp() {}

    // will be called immediately after each test before the destructor
    virtual void TearDown() {}

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        hal::pd::pd_if_get_lport_id_args_t lport_args;
        hal::pd::pd_l2seg_get_flow_lkupid_args_t lkupid_args;
        hal::pd::pd_func_args_t          pd_func_args = {0};
        fte_base_test::SetUpTestCase();

        // create topo
        intfh1_ = add_uplink(PORT_NUM_1);
        intfh2_ =  add_uplink(PORT_NUM_2);
        vrfh1_ = add_vrf();
        vrfh2_ = add_vrf();
        nwh1_ = add_network(vrfh1_, 0x0A000000, 24, 0xAABB0A000000);
        nwh2_ = add_network(vrfh2_, 0x0A010000, 24, 0xAABB0A010000);
        l2segh1_ = add_l2segment(nwh1_, 100);
        l2segh2_ = add_l2segment(nwh2_, 200);
        gwh1_ = add_endpoint(l2segh1_, intfh1_, 0x0A000001, 0xAABB0A000001, 0);
        gwh2_ = add_endpoint(l2segh2_, intfh2_, 0x0A010001, 0xAABB0A010001, 0);
        add_route(vrfh1_, 0x0A000000, 24, gwh1_);
        add_route(vrfh2_, 0x0A010000, 24, gwh2_);
        poolh_ = add_nat_pool(vrfh1_, 0x0A640000, 24);

        // firewall rules
        std::vector<v4_rule_t> rules = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW }
        };

        add_nwsec_policy(vrfh1_, rules);
        add_nwsec_policy(vrfh2_, rules);


        // read lport/lkup ids
        lport_args.pi_if = hal::find_if_by_handle(intfh1_);
        pd_func_args.pd_if_get_lport_id = &lport_args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_IF_GET_LPORT_ID, &pd_func_args);
        lport1_ = lport_args.lport_id;

        lport_args.pi_if = hal::find_if_by_handle(intfh2_);
        pd_func_args.pd_if_get_lport_id = &lport_args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_IF_GET_LPORT_ID, &pd_func_args);
        lport2_ = lport_args.lport_id;

        lkupid_args.l2seg = hal::l2seg_lookup_by_handle(l2segh1_);
        pd_func_args.pd_l2seg_get_flow_lkupid = &lkupid_args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, &pd_func_args);
        lkupid1_ =  lkupid_args.hwid;

        lkupid_args.l2seg = hal::l2seg_lookup_by_handle(l2segh2_);
        pd_func_args.pd_l2seg_get_flow_lkupid = &lkupid_args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_L2SEG_GET_FLOW_LKPID, &pd_func_args);
        lkupid2_ =  lkupid_args.hwid;

    }

    static hal_handle_t gwh1_, gwh2_, vrfh1_, vrfh2_, nwh1_, nwh2_,
        l2segh1_, l2segh2_, intfh1_, intfh2_, poolh_;
    static uint32_t lport1_, lport2_, lkupid1_, lkupid2_;

public:
    static void route_lookup_test(void *ptr);
};

hal_handle_t nat_test::gwh1_, nat_test::gwh2_, nat_test::vrfh1_, nat_test::vrfh2_,
    nat_test::nwh1_, nat_test::nwh2_, nat_test::l2segh1_, nat_test::l2segh2_,
    nat_test::intfh1_, nat_test::intfh2_, nat_test::poolh_;
uint32_t nat_test::lport1_, nat_test::lport2_, nat_test::lkupid1_, nat_test::lkupid2_;

TEST_F(nat_test, dnat)
{
    hal_ret_t ret;
    uint32_t client_ip = 0x0A000064, server_ip = 0x0A010064;
    uint32_t nat_ip;
    bool reinject = true;

    // Add nat mapping for server in vrf2
    add_nat_mapping(vrfh2_, server_ip, poolh_, &nat_ip);

    // Create TCP session (vrf1:10.0.0.100 to vrf2:10.1.0.100(nat_ip))
    Tins::TCP tcp = Tins::TCP(80, 1000);
    tcp.flags(Tins::TCP::SYN);

    Tins::EthernetII eth =
        Tins::EthernetII(Tins::HWAddress<6>("aa:bb:cc:dd:ee:ff"),
                         Tins::HWAddress<6>("aa:bb:0a:00:00:64")) /
        Tins::Dot1Q(100) /
        Tins::IP(Tins::IPv4Address(htonl(nat_ip)), Tins::IPv4Address(htonl(client_ip))) /
        tcp;

 reinject:
    ret = inject_eth_pkt(fte::FLOW_MISS_LIFQ, intfh1_, l2segh1_, eth);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_TRUE(ctx_.valid_rflow());
    EXPECT_NE(ctx_.session(), nullptr);
    EXPECT_EQ(ctx_.dif()->hal_handle, intfh2_);
    EXPECT_EQ(ctx_.dl2seg()->hal_handle, l2segh2_);
    EXPECT_EQ(ctx_.dep()->hal_handle, gwh2_);

    // check iflow
    auto attrs = &ctx_.session()->iflow->pgm_attrs;
    auto config = &ctx_.session()->iflow->config;

    EXPECT_EQ(config->key.svrf_id, 1);
    EXPECT_EQ(config->key.sip.v4_addr, client_ip);
    EXPECT_EQ(config->key.dip.v4_addr, nat_ip);
    EXPECT_EQ(config->key.sport, 1000);
    EXPECT_EQ(config->key.dport, 80);
    EXPECT_EQ(config->key.proto, IP_PROTO_TCP);

    EXPECT_EQ(config->nat_type, hal::NAT_TYPE_DNAT);
    EXPECT_EQ(config->nat_sip.addr.v4_addr, 0);
    EXPECT_EQ(config->nat_dip.addr.v4_addr, server_ip);
    EXPECT_EQ(config->nat_sport, 0);
    EXPECT_EQ(config->nat_dport, 0);

    EXPECT_EQ(attrs->lport, lport2_);
    EXPECT_EQ(attrs->mac_sa_rewrite, 1);
    EXPECT_EQ(attrs->mac_da_rewrite, 1);
    EXPECT_EQ(attrs->vrf_hwid, lkupid1_);
    EXPECT_EQ(attrs->rw_act, hal::REWRITE_IPV4_NAT_DST_REWRITE_ID);
    EXPECT_EQ(attrs->tnnl_vnid, 200);
    EXPECT_EQ(attrs->nat_ip.addr.v4_addr, server_ip);

     // check rflow
    attrs = &ctx_.session()->iflow->reverse_flow->pgm_attrs;
    config = &ctx_.session()->iflow->reverse_flow->config;

    EXPECT_EQ(config->key.svrf_id, 2);
    EXPECT_EQ(config->key.sip.v4_addr, server_ip);
    EXPECT_EQ(config->key.dip.v4_addr, client_ip);
    EXPECT_EQ(config->key.sport, 80);
    EXPECT_EQ(config->key.dport, 1000);
    EXPECT_EQ(config->key.proto, IP_PROTO_TCP);

    EXPECT_EQ(config->nat_type, hal::NAT_TYPE_SNAT);
    EXPECT_EQ(config->nat_sip.addr.v4_addr, nat_ip);
    EXPECT_EQ(config->nat_dip.addr.v4_addr, 0);
    EXPECT_EQ(config->nat_sport, 0);
    EXPECT_EQ(config->nat_dport, 0);

    EXPECT_EQ(attrs->lport, lport1_);
    EXPECT_EQ(attrs->mac_sa_rewrite, 1);
    EXPECT_EQ(attrs->mac_da_rewrite, 1);
    EXPECT_EQ(attrs->vrf_hwid, lkupid2_);
    EXPECT_EQ(attrs->rw_act, hal::REWRITE_IPV4_NAT_SRC_REWRITE_ID);
    EXPECT_EQ(attrs->tnnl_vnid, 100);
    EXPECT_EQ(attrs->nat_ip.addr.v4_addr, nat_ip);

    // reinject the packet (to test reading existing session)
    if (reinject) {
        reinject = false;
        goto reinject;
    }
}

TEST_F(nat_test, snat)
{
    hal_ret_t ret;
    uint32_t client_ip = 0x0A010065, server_ip = 0x0A000065;
    uint32_t nat_ip;
    bool reinject = true;

    // Add nat mapping for client in vrf2
    add_nat_mapping(vrfh2_, client_ip, poolh_, &nat_ip);

    // Create TCP session (vrf2:10.1.0.101 to vrf1:10.0.0.101)
    Tins::TCP tcp = Tins::TCP(80, 1000);
    tcp.flags(Tins::TCP::SYN);

    Tins::EthernetII eth =
        Tins::EthernetII(Tins::HWAddress<6>("aa:bb:0a:00:00:65"),
                         Tins::HWAddress<6>("aa:bb:cc:dd:ee:ff")) /
        Tins::Dot1Q(200) /
        Tins::IP(Tins::IPv4Address(htonl(server_ip)), Tins::IPv4Address(htonl(client_ip))) /
        tcp;

 reinject:
    ret = inject_eth_pkt(fte::FLOW_MISS_LIFQ, intfh2_, l2segh2_, eth);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_TRUE(ctx_.valid_rflow());
    EXPECT_NE(ctx_.session(), nullptr);
    EXPECT_EQ(ctx_.dif()->hal_handle, intfh1_);
    EXPECT_EQ(ctx_.dl2seg()->hal_handle, l2segh1_);
    EXPECT_EQ(ctx_.dep()->hal_handle, gwh1_);

    // check iflow
    auto attrs = &ctx_.session()->iflow->pgm_attrs;
    auto config = &ctx_.session()->iflow->config;

    EXPECT_EQ(config->key.svrf_id, 2);
    EXPECT_EQ(config->key.sip.v4_addr, client_ip);
    EXPECT_EQ(config->key.dip.v4_addr, server_ip);
    EXPECT_EQ(config->key.sport, 1000);
    EXPECT_EQ(config->key.dport, 80);
    EXPECT_EQ(config->key.proto, IP_PROTO_TCP);

    EXPECT_EQ(config->nat_type, hal::NAT_TYPE_SNAT);
    EXPECT_EQ(config->nat_sip.addr.v4_addr, nat_ip);
    EXPECT_EQ(config->nat_dip.addr.v4_addr, 0);
    EXPECT_EQ(config->nat_sport, 0);
    EXPECT_EQ(config->nat_dport, 0);

    EXPECT_EQ(attrs->lport, lport1_);
    EXPECT_EQ(attrs->mac_sa_rewrite, 1);
    EXPECT_EQ(attrs->mac_da_rewrite, 1);
    EXPECT_EQ(attrs->vrf_hwid, lkupid2_);
    EXPECT_EQ(attrs->rw_act, hal::REWRITE_IPV4_NAT_SRC_REWRITE_ID);
    EXPECT_EQ(attrs->tnnl_vnid, 100);
    EXPECT_EQ(attrs->nat_ip.addr.v4_addr, nat_ip);

     // check rflow
    attrs = &ctx_.session()->iflow->reverse_flow->pgm_attrs;
    config = &ctx_.session()->iflow->reverse_flow->config;

    EXPECT_EQ(config->key.svrf_id, 1);
    EXPECT_EQ(config->key.sip.v4_addr, server_ip);
    EXPECT_EQ(config->key.dip.v4_addr, nat_ip);
    EXPECT_EQ(config->key.sport, 80);
    EXPECT_EQ(config->key.dport, 1000);
    EXPECT_EQ(config->key.proto, IP_PROTO_TCP);

    EXPECT_EQ(config->nat_type, hal::NAT_TYPE_DNAT);
    EXPECT_EQ(config->nat_sip.addr.v4_addr, 0);
    EXPECT_EQ(config->nat_dip.addr.v4_addr, client_ip);
    EXPECT_EQ(config->nat_sport, 0);
    EXPECT_EQ(config->nat_dport, 0);

    EXPECT_EQ(attrs->lport, lport2_);
    EXPECT_EQ(attrs->mac_sa_rewrite, 1);
    EXPECT_EQ(attrs->mac_da_rewrite, 1);
    EXPECT_EQ(attrs->vrf_hwid, lkupid1_);
    EXPECT_EQ(attrs->rw_act, hal::REWRITE_IPV4_NAT_DST_REWRITE_ID);
    EXPECT_EQ(attrs->tnnl_vnid, 200);
    EXPECT_EQ(attrs->nat_ip.addr.v4_addr, client_ip);

    // reinject the packet (to test reading existing session)
    if (reinject) {
        reinject = false;
        goto reinject;
    }
}

