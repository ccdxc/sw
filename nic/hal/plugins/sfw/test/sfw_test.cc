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

class sfw_test : public fte_base_test {
protected:
    sfw_test() {}

    virtual ~sfw_test() {}

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


        // firewall rules
        std::vector<v4_rule_t> rules = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW, 
                        from: { addr: g_eps[0].ip, plen: 32 }, 
                        to: { addr: g_eps[1].ip, plen: 32},
                        app: { proto:IPPROTO_TCP,
                               dport_low: 22, dport_high: 22,
                               alg: nwsec::APP_SVC_NONE} },
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_TCP,
                               dport_low: 80, dport_high: 80,
                               alg: nwsec::APP_SVC_NONE} },
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_DENY,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_TCP,
                               dport_low: 0, dport_high: 0xFFFF,
                               alg: nwsec::APP_SVC_NONE} },
        };

        nwsech_ = add_nwsec_policy(vrfh_, rules);
        fte_base_test::set_logging_disable(true);
    }

    static hal_handle_t vrfh_, nwh_, l2segh_, eph1_, eph2_,  intfh1_, intfh2_, nwsech_;
};

hal_handle_t sfw_test::vrfh_, sfw_test::nwh_, sfw_test::l2segh_,
    sfw_test::eph1_, sfw_test::eph2_,  sfw_test::intfh1_,
    sfw_test::intfh2_, sfw_test::nwsech_;

TEST_F(sfw_test, allow_rule)
{
    CHECK_ALLOW_TCP(eph2_, eph1_, 22, 1000, "ep1->ep2 22");
    CHECK_ALLOW_TCP(eph2_, eph1_, 80, 1000, "ep1->ep2 80");
    CHECK_ALLOW_TCP(eph1_, eph2_, 80, 1000, "ep2->ep1 80");
}

TEST_F(sfw_test, deny_rule)
{
    CHECK_DENY_TCP(eph1_, eph2_, 22, 1000, "ep2->ep1 22");
    CHECK_DENY_TCP(eph2_, eph1_, 100, 1000, "ep1->ep2 100");
    CHECK_DENY_TCP(eph1_, eph2_, 100, 1000, "ep2->ep1 100");
}
