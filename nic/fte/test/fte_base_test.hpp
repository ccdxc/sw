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


    static hal_ret_t inject_pkt(fte::cpu_rxhdr_t *cpu_rxhdr, uint8_t *pkt, size_t pkt_len,
                                bool *drop, hal::session_t **session);

    static hal_ret_t inject_ipv4_pkt(const fte::lifqid_t &lifq,
                                     hal_handle_t dep, hal_handle_t sep,
                                     Tins::PDU &l4pdu, bool *drop, hal::session_t **session);

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
private:
    static uint32_t vrf_id_, l2seg_id_, intf_id_, nwsec_id_;
    static fte::ctx_t ctx_;
    static uint16_t num_features_;
    static fte::feature_state_t *feature_state_;
};
