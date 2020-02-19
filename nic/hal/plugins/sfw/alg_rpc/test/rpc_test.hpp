#pragma once

#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/plugins/sfw/cfg/nwsec_group.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"

#define MSRPC_PORT 135
#define SUNRPC_PORT 111

class rpc_test : public fte_base_test {
protected:
    rpc_test() {}

    virtual ~rpc_test() {}

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
        s_e2e_eph = add_endpoint(l2segh, intfh1, 0x40000001 , 0xEEBB0D000001, 0, true);
        c_e2e_eph = add_endpoint(l2segh, intfh2, 0x40000002 , 0xEEBB0D000002, 0, true);
        hal::rpc_programid_t programids[] = { { 100000, 120 },
                                              { 100024, 100 },
                                              { 100023, 300 },
                                              { 111, 100 } };
        hal::rpc_uuid_t uuids[] = { { {0xef, 0xaf, 0x83, 0x08, 0x5d, 0x1f, 0x11, 0xc9, 0x91,
                                       0xa4, 0x08, 0x00, 0x2b, 0x14, 0xa0, 0xfa}, 90}, 
                                    { {0x12, 0x34, 0x57, 0x78, 0x12, 0x34, 0xab, 0xcd, 0xef,
                                       0x00, 0x01, 0x23, 0x45, 0x67, 0x89, 0xac}, 100},
                                    { {0xe3, 0x51, 0x42, 0x35, 0x4b, 0x06, 0x11, 0xd1, 0xab,
                                       0x04, 0x00, 0xc0, 0x4f, 0xc2, 0xdc, 0xd2}, 100},
                                    { {0xcc, 0xd8, 0xc0, 0x74, 0xd0, 0xe5, 0x4a, 0x40, 0x92,
                                       0xb4, 0xd0, 0x74, 0xfa, 0xa6, 0xba, 0x28}, 200} };

        // firewall rules
        std::vector<v4_rule_t> rules = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_UDP,
                               dport_low: SUNRPC_PORT, dport_high: SUNRPC_PORT,
                               alg: nwsec::APP_SVC_SUN_RPC,
                               idle_timeout: 30,
                               has_alg_opts: true,
                               alg_opt: { opt: { sunrpc_opts: { programid_sz: 4, program_ids: programids }, }, },
                               } },
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_TCP,
                               dport_low: SUNRPC_PORT, dport_high: SUNRPC_PORT,
                               alg: nwsec::APP_SVC_SUN_RPC,
                               idle_timeout: 30,
                               has_alg_opts: true,
                               alg_opt: { opt: { sunrpc_opts: { programid_sz: 3, program_ids: programids }, }, },
                               } },
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_TCP,
                               dport_low: MSRPC_PORT, dport_high: MSRPC_PORT,
                               alg: nwsec::APP_SVC_MSFT_RPC,
                               idle_timeout: 30,
                               has_alg_opts: true,
                               alg_opt: { opt: { msrpc_opts: { uuid_sz: 4, uuids: uuids }, }, },
                               } },
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_DENY,
                        from: {},
                        to: {},
                        app: { proto:0,
                               dport_low: 0, dport_high: 0xFFFF,
                               alg: nwsec::APP_SVC_NONE} },
        };

        add_nwsec_policy(vrfh, rules);
    }

    static hal_handle_t client_eph, server_eph;

public:
    static void sunrpc_session_create(void *);
    static void msrpc_session_create(void *);
    static hal_handle_t c_e2e_eph, s_e2e_eph;
};
