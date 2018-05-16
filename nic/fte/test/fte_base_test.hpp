#pragma once

#include <gtest/gtest.h>
#include "nic/include/base.h"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/include/fte_ctx.hpp"
#include <tins/tins.h>

#define FTE_ID 0

class fte_base_test : public hal_base_test {
public:
    static hal_handle_t add_vrf();
    static hal_handle_t add_network(hal_handle_t vrfh, uint32_t v4_addr,
                                    uint8_t prefix_len, uint64_t rmac);
    static hal_handle_t add_l2segment(hal_handle_t nwh, uint16_t vlan_id);
    static hal_handle_t add_uplink(uint8_t port_num);
    static hal_handle_t add_endpoint(hal_handle_t l2segh, hal_handle_t intfh,
                                     uint32_t ip, uint64_t mac, uint16_t useg_vlan);
    static hal_handle_t add_route(hal_handle_t vrfh,
                                  uint32_t v4_addr, uint8_t prefix_len,
                                  hal_handle_t eph);
    struct v4_rule_t {
        nwsec::SecurityAction action;
        struct {
            uint32_t addr;
            uint8_t plen;
        } from, to;
        struct {
            uint16_t proto;
            uint16_t dport_low;
            uint16_t dport_high;
            nwsec::ALGName alg;
        } app;
    };

    static hal_handle_t add_nwsec_policy(hal_handle_t vrfh, std::vector<v4_rule_t> &rules);

    static hal_handle_t add_nat_pool(hal_handle_t vrfh,
                                     uint32_t v4_addr, uint8_t prefix_len);

    static hal_handle_t add_nat_mapping(hal_handle_t vrfh, uint32_t v4_addr,
                                        hal_handle_t poolh, uint32_t *mapped_ip);

    static hal_ret_t inject_pkt(fte::cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len);
    static hal_ret_t inject_eth_pkt(const fte::lifqid_t &lifq,
                                    hal_handle_t src_ifh, hal_handle_t src_l2segh,
                                    Tins::EthernetII &eth);
    static hal_ret_t inject_ipv4_pkt(const fte::lifqid_t &lifq,
                                     hal_handle_t dep, hal_handle_t sep, Tins::PDU &l4pdu);
    static void set_logging_disable(bool val) { ipc_logging_disable_ = val; }
protected:
    fte_base_test() {}

    virtual ~fte_base_test() { }

    // will be called immediately after the constructor before each test
    virtual void SetUp() { }

    // will be called immediately after each test before the destructor
    virtual void TearDown() { }

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        hal_base_test::SetUpTestCase();
        ipc_logging_disable_ = false;
    }
    static fte::ctx_t ctx_;

private:
    static uint32_t vrf_id_, l2seg_id_, intf_id_, nwsec_id_, nh_id_, pool_id_;
    static bool ipc_logging_disable_;
};

#define CHECK_ALLOW_TCP(dep, sep, dst_port, src_port, msg) {                   \
        hal_ret_t ret;                                                         \
        Tins::TCP tcp = Tins::TCP(dst_port, src_port);                         \
        tcp.flags(Tins::TCP::SYN);                                             \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, tcp);             \
        EXPECT_EQ(ret, HAL_RET_OK)<< msg;                                      \
        EXPECT_FALSE(ctx_.drop())<< msg;                                       \
        EXPECT_NE(ctx_.session(), nullptr)<< msg;                              \
        EXPECT_NE(ctx_.session()->iflow, nullptr)<< msg;                       \
        EXPECT_NE(ctx_.session()->rflow, nullptr)<< msg;                       \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;             \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;             \
        EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sipv4(),            \
                  ctx_.key().sip.v4_addr);                                     \
        EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->dipv4(),            \
                  ctx_.key().dip.v4_addr);                                     \
        EXPECT_EQ(((uint16_t)ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sport()), \
                  ctx_.key().sport);                                           \
        EXPECT_EQ(((uint16_t)ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->dport()), \
                  ctx_.key().dport);                                           \
        EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->ipprot(),            \
                  ctx_.key().proto);                                           \
    }

#define CHECK_DENY_TCP(dep, sep, dst_port, src_port, msg) {             \
        hal_ret_t ret;                                                  \
        Tins::TCP tcp = Tins::TCP(dst_port, src_port);                  \
        tcp.flags(Tins::TCP::SYN);                                      \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep,tcp);       \
        EXPECT_EQ(ret, HAL_RET_OK) << msg;                              \
        EXPECT_TRUE(ctx_.drop()) << msg;                                \
        EXPECT_NE(ctx_.session(), nullptr) << msg;                      \
        EXPECT_NE(ctx_.session()->iflow, nullptr) << msg;               \
        EXPECT_NE(ctx_.session()->rflow, nullptr) << msg;               \
        EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.drop) << msg;      \
        EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.drop) << msg;      \
    }

#define CHECK_ALLOW_UDP(dep, sep, dst_port, src_port, msg)          \
    {                                                               \
        hal_ret_t ret;                                              \
        Tins::UDP pdu = Tins::UDP(dst_port, src_port);              \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, pdu);  \
        EXPECT_EQ(ret, HAL_RET_OK) << msg;                          \
        EXPECT_FALSE(ctx_.drop()) << msg;                           \
        EXPECT_NE(ctx_.session(), nullptr) << msg;                  \
        EXPECT_NE(ctx_.session()->iflow, nullptr)<< msg;            \
        EXPECT_NE(ctx_.session()->rflow, nullptr)<< msg;            \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;  \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;  \
    }

#define CHECK_DENY_UDP(dep, sep, dst_port, src_port, msg)                      \
    {                                                                          \
        hal_ret_t ret;                                                         \
        Tins::UDP pdu = Tins::UDP(dst_port, src_port);                         \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, pdu);             \
        EXPECT_EQ(ret, HAL_RET_OK) << msg;                                     \
        EXPECT_TRUE(ctx_.drop()) << msg;                                       \
        EXPECT_NE(ctx_.session(), nullptr) << msg;                             \
        EXPECT_NE(ctx_.session()->iflow, nullptr) << msg;                      \
        EXPECT_NE(ctx_.session()->rflow, nullptr) << msg;                      \
        EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.drop) << msg;             \
        EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.drop) << msg;             \
        EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_RESPONDER)->sipv4(),            \
                  ctx_.get_key(hal::FLOW_ROLE_RESPONDER).sip.v4_addr);         \
        EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_RESPONDER)->dipv4(),            \
                  ctx_.get_key(hal::FLOW_ROLE_RESPONDER).dip.v4_addr);         \
        EXPECT_EQ(((uint16_t)ctx_.flow_log(hal::FLOW_ROLE_RESPONDER)->sport()), \
                  ctx_.get_key(hal::FLOW_ROLE_RESPONDER).sport);                \
        EXPECT_EQ(((uint16_t)ctx_.flow_log(hal::FLOW_ROLE_RESPONDER)->dport()), \
                  ctx_.get_key(hal::FLOW_ROLE_RESPONDER).dport);                \
        EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_RESPONDER)->ipprot(),            \
                  ctx_.get_key(hal::FLOW_ROLE_RESPONDER).proto);                \
    }
