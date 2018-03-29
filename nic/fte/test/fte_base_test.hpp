#pragma once

#include <gtest/gtest.h>
#include "nic/include/base.h"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "nic/gen/proto/hal/nwsec.pb.h"
#include "nic/include/fte_ctx.hpp"
#include <tins/tins.h>

class fte_base_test : public hal_base_test {
public:
    static hal_handle_t add_vrf();
    static hal_handle_t add_network(hal_handle_t vrfh, uint32_t v4_addr,
                                    uint8_t prefix_len, uint64_t rmac);
    static hal_handle_t add_l2segment(hal_handle_t nwh, uint16_t vlan_id);
    static hal_handle_t add_uplink(uint8_t port_num);
    static hal_handle_t add_endpoint(hal_handle_t l2segh, hal_handle_t intfh,
                                     uint32_t ip, uint64_t mac, uint16_t useg_vlan);

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
            nwsec::PredefinedApps alg;
        } app;
    };

    static hal_handle_t add_nwsec_policy(hal_handle_t vrfh, std::vector<v4_rule_t> &rules);


    static hal_ret_t inject_pkt(fte::cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len);

    static hal_ret_t inject_ipv4_pkt(const fte::lifqid_t &lifq,
                                     hal_handle_t dep, hal_handle_t sep, Tins::PDU &l4pdu);

protected:
    fte_base_test() {}

    virtual ~fte_base_test() {}

    // will be called immediately after the constructor before each test
    virtual void SetUp() {}

    // will be called immediately after each test before the destructor
    virtual void TearDown() {}

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        hal_base_test::SetUpTestCase();

        // init fte feature state memory
        size_t fstate_size = fte::feature_state_size(&num_features_);
        feature_state_ = (fte::feature_state_t*)HAL_MALLOC(hal::HAL_MEM_ALLOC_FTE, fstate_size);

    }
    static fte::ctx_t ctx_;

private:
    static uint32_t vrf_id_, l2seg_id_, intf_id_, nwsec_id_;
    static uint16_t num_features_;
    static fte::feature_state_t *feature_state_;
};

#define CHECK_ALLOW_TCP(dep, sep, dport, sport, msg) {                  \
        hal_ret_t ret;                                                  \
        Tins::TCP tcp = Tins::TCP(dport, sport);                        \
        tcp.flags(Tins::TCP::SYN);                                      \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, tcp);      \
        EXPECT_EQ(ret, HAL_RET_OK)<< msg;                               \
        EXPECT_FALSE(ctx_.drop())<< msg;                                \
        EXPECT_NE(ctx_.session(), nullptr)<< msg;                       \
        EXPECT_NE(ctx_.session()->iflow, nullptr)<< msg;                \
        EXPECT_NE(ctx_.session()->rflow, nullptr)<< msg;                \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;      \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;      \
    }

#define CHECK_DENY_TCP(dep, sep, dport, sport, msg) {                   \
        hal_ret_t ret;                                                  \
        Tins::TCP tcp = Tins::TCP(dport, sport);                        \
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

#define CHECK_ALLOW_UDP(dep, sep, dport, sport, msg)                \
    {                                                               \
        hal_ret_t ret;                                              \
        Tins::UDP pdu = Tins::UDP(dport, sport);                    \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, pdu);  \
        EXPECT_EQ(ret, HAL_RET_OK) << msg;                          \
        EXPECT_FALSE(ctx_.drop()) << msg;                           \
        EXPECT_NE(ctx_.session(), nullptr) << msg;                  \
        EXPECT_NE(ctx_.session()->iflow, nullptr)<< msg;            \
        EXPECT_NE(ctx_.session()->rflow, nullptr)<< msg;            \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;  \
        EXPECT_FALSE(ctx_.session()->iflow->pgm_attrs.drop)<< msg;  \
    }

#define CHECK_DENY_UDP(dep, sep, dport, sport, msg)                 \
    {                                                               \
        hal_ret_t ret;                                              \
        Tins::UDP pdu = Tins::UDP(dport, sport);                    \
        ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, dep, sep, pdu);  \
        EXPECT_EQ(ret, HAL_RET_OK) << msg;                          \
        EXPECT_TRUE(ctx_.drop()) << msg;                            \
        EXPECT_NE(ctx_.session(), nullptr) << msg;                  \
        EXPECT_NE(ctx_.session()->iflow, nullptr) << msg;           \
        EXPECT_NE(ctx_.session()->rflow, nullptr) << msg;           \
        EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.drop) << msg;  \
        EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.drop) << msg;  \
    }
