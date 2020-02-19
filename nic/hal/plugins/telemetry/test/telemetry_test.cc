#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/pd/pd_api.hpp"
#include "nic/include/fte.hpp"
#include "nic/fte/fte_ctx.hpp"
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include <tins/tins.h>

struct {
    uint32_t ip;
    uint64_t mac;
} g_eps[] = {
    { 0x0A000001, 0xAABB0A000001},
    { 0x0A000002, 0xAABB0A000002},
};

class telemetry_test : public fte_base_test {
protected:
    telemetry_test() {}

    virtual ~telemetry_test() {}

    // will be called immediately after the constructor before each test
    virtual void SetUp() {}

    // will be called immediately after each test before the destructor
    virtual void TearDown() {}

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        fte_base_test::SetUpTestCase();

        // create topo
        vrfh_ = add_vrf();
        nwh_ = add_network(vrfh_, 0x0A000000, 8, 0xAABB0A000000);
        l2segh_ = add_l2segment(nwh_, 100);
        intfh1_ =  add_uplink(PORT_NUM_1);
        intfh2_ =  add_uplink(PORT_NUM_2);
        eph1_ = add_endpoint(l2segh_, intfh1_, g_eps[0].ip, g_eps[0].mac, 0);
        eph2_ = add_endpoint(l2segh_, intfh2_, g_eps[1].ip, g_eps[1].mac, 0);

        telemetry::MonitorAction mon_action;
        mon_action.add_action(telemetry::MIRROR);
        kh::MirrorSessionKeyHandle *mkh = mon_action.add_ms_key_handle();
        mkh->set_mirrorsession_id(1);
        // flowmon rules
        std::vector<v4_rule_t> rules = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW,
                        from: { addr: g_eps[0].ip, plen: 32 }, 
                        to: { addr: g_eps[1].ip, plen: 32},
                        app: { proto:IPPROTO_TCP, dport_low: 22, dport_high: 22,
                        alg: nwsec::APP_SVC_NONE },
                        mon_action: mon_action,
                        collect: false },
        };
        flowmonh_ = add_flowmon_policy(vrfh_, rules);

        std::vector<v4_rule_t> rules2 = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW,
                        from: { addr: g_eps[1].ip, plen: 32 }, 
                        to: { addr: g_eps[0].ip, plen: 32},
                        app: { proto:IPPROTO_TCP, dport_low: 22, dport_high: 22,
                        alg: nwsec::APP_SVC_NONE },
                        mon_action: mon_action,
                        collect: false },
        };

        flowmonh_ = add_flowmon_policy(vrfh_, rules2);
        
        telemetry::MonitorAction ipfix_action;
        ipfix_action.add_action(telemetry::COLLECT_FLOW_STATS);
        std::vector<v4_rule_t> rules3 = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW,
                        from: { addr: g_eps[0].ip, plen: 32 }, 
                        to: { addr: g_eps[1].ip, plen: 32},
                        app: { proto:IPPROTO_TCP, dport_low: 33, dport_high: 33,
                        alg: nwsec::APP_SVC_NONE },
                        mon_action: ipfix_action,
                        collect: true },
        };
        flowmonh_ = add_flowmon_policy(vrfh_, rules3);

        std::vector<v4_rule_t> rules4 = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW,
                        from: { addr: g_eps[1].ip, plen: 32 }, 
                        to: { addr: g_eps[0].ip, plen: 32},
                        app: { proto:IPPROTO_TCP, dport_low: 33, dport_high: 33,
                        alg: nwsec::APP_SVC_NONE },
                        mon_action: ipfix_action,
                        collect: true },
        };
        flowmonh_ = add_flowmon_policy(vrfh_, rules4);

        fte_base_test::set_logging_disable(true);
    }

    static hal_handle_t vrfh_, nwh_, l2segh_, eph1_, eph2_,  intfh1_, intfh2_, flowmonh_;
};

hal_handle_t telemetry_test::vrfh_, telemetry_test::nwh_, telemetry_test::l2segh_,
    telemetry_test::eph1_, telemetry_test::eph2_,  telemetry_test::intfh1_,
    telemetry_test::intfh2_, telemetry_test::flowmonh_;

TEST_F(telemetry_test, mirror_action)
{
    CHECK_MIRROR_ACTION(eph2_, eph1_, 22, 1000, "ep1->ep2 22");
    //CHECK_MIRROR_ACTION(eph2_, eph1_, 80, 1000, "ep1->ep2 80");
    //CHECK_MIRROR_ACTION(eph1_, eph2_, 80, 1000, "ep2->ep1 80");
}

TEST_F(telemetry_test, mirror_action2)
{
    CHECK_MIRROR_ACTION(eph1_, eph2_, 22, 1000, "ep2->ep1 22");
    //CHECK_MIRROR_ACTION(eph2_, eph1_, 100, 1000, "ep1->ep2 100");
    //CHECK_MIRROR_ACTION(eph1_, eph2_, 100, 1000, "ep2->ep1 100");
}

TEST_F(telemetry_test, collect_action1)
{
    CHECK_COLLECT_ACTION(eph2_, eph1_, 33, 1000, "ep1->ep2 33");
    //CHECK_MIRROR_ACTION(eph2_, eph1_, 100, 1000, "ep1->ep2 100");
    //CHECK_MIRROR_ACTION(eph1_, eph2_, 100, 1000, "ep2->ep1 100");
}

TEST_F(telemetry_test, collect_action2)
{
    CHECK_COLLECT_ACTION(eph1_, eph2_, 33, 1000, "ep2->ep1 33");
    //CHECK_MIRROR_ACTION(eph2_, eph1_, 100, 1000, "ep1->ep2 100");
    //CHECK_MIRROR_ACTION(eph1_, eph2_, 100, 1000, "ep2->ep1 100");
}


