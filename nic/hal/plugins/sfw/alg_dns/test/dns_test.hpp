//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#pragma once

#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"

#define DNS_PORT 53 

class dns_test : public fte_base_test {
protected:
    dns_test() {}

    virtual ~dns_test() {}

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

        // firewall rules
        std::vector<v4_rule_t> rules = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW,
                        from: { addr: 0x0A000001, plen: 32 },
                        to: { addr: 0x0A000002, plen: 32 },
                        app: { proto:IPPROTO_UDP,
                               dport_low: DNS_PORT, dport_high: DNS_PORT,
                               alg: nwsec::APP_SVC_DNS,
                               idle_timeout: 30,
                               has_alg_opts: true,
                               alg_opt: { opt: { dns_opts: { drop_multi_question_packets: 1, 
                                                             drop_large_domain_name_packets: 1,
                                                             drop_long_label_packets: 1,
                                                             drop_multizone_packets: 1,
                                                             max_msg_length:8192, },
                                         }, },
                               }, },
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
    static void dns_session_create(void *);
};
