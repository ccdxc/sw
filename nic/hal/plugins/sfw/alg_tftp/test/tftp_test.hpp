#pragma once

#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"

#define TFTP_PORT 69

using namespace hal::plugins::alg_tftp;
using namespace session;
using namespace fte;

class tftp_test : public fte_base_test {
protected:
    tftp_test() {}

    virtual ~tftp_test() { fte_base_test::CleanUpE2ETestCase(); std::system("rm tftp_file.txt"); }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {}

    // will be called immediately after each test before the destructor
    virtual void TearDown() {}

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        fte_base_test::SetUpTestCase();

        // create topo
        hal_handle_t vrfh = add_vrf();
        hal_handle_t nwh = add_network(vrfh, 0x0A000000, 8, 0xAABB0A000000);
        hal_handle_t l2segh = add_l2segment(nwh, 100);
        hal_handle_t intfh1 = add_uplink(PORT_NUM_1);
        hal_handle_t intfh2 =  add_uplink(PORT_NUM_2);
        client_eph = add_endpoint(l2segh, intfh1, 0x0A000001 , 0xAABB0A000001, 0);
        server_eph = add_endpoint(l2segh, intfh2, 0x0A000002 , 0xAABB0A000002, 0);
        s_e2e_eph = add_endpoint(l2segh, intfh1, 0x40000001 , 0xEEBB0A000001, 0, true);
        c_e2e_eph = add_endpoint(l2segh, intfh2, 0x40000002 , 0xEEBB0A000002, 0, true);

        // firewall rules
        std::vector<v4_rule_t> rules = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_UDP,
                               dport_low: TFTP_PORT, dport_high: TFTP_PORT,
                               alg: nwsec::APP_SVC_TFTP } },
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_DENY,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_UDP,
                               dport_low: 0, dport_high: 0xFFFF,
                               alg: nwsec::APP_SVC_NONE} },
        };

        add_nwsec_policy(vrfh, rules);
    }

    static hal_handle_t client_eph, server_eph;

public:
    static void tftp_session_create(void *ptr);
    static hal_handle_t c_e2e_eph, s_e2e_eph;
};
