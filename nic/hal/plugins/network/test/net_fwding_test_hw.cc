#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/include/pd_api.hpp"
#include "nic/include/fte.hpp"
#include "lib/utils/time_profile.hpp"
#include <vector>
#include "nic/hal/test/utils/hal_test_utils.hpp"

#include <tins/tins.h>

using namespace fte;
using namespace std;

class fwding_test_hw : public fte_base_test {
protected:
    fwding_test_hw() {}

    virtual ~fwding_test_hw() {}

    // will be called immediately after the constructor before each test
    virtual void SetUp() {}

    // will be called immediately after each test before the destructor
    virtual void TearDown() {}

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        setenv("HAL_CONFIG_PATH", "/nic/conf", 1);
        fte_base_test::SetUpTestCase("hal_hw.json");

        // create topo
        vrfh_ = add_vrf();
        nwh_ = add_network(vrfh_, 0x0A000000, 24, 0xAABB0A000000);
        l2segh1_ = add_l2segment(nwh_, 100);
        l2segh2_ = add_l2segment(nwh_, 200);
        intfh1_ = add_uplink(PORT_NUM_1);
        intfh2_ =  add_uplink(PORT_NUM_2);
        eph_ = add_endpoint(l2segh1_, intfh1_, 0x0A000001, 0xAABB0A000001, 0);
        gwh_ = add_endpoint(l2segh2_, intfh2_, 0x0A000002, 0xAABB0A000002, 0);
        add_route(vrfh_, 0x0A000100, 24, gwh_);

        // firewall rules
        std::vector<v4_rule_t> rules = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW }
        };

        add_nwsec_policy(vrfh_, rules);
    }

    static hal_handle_t eph_, gwh_, vrfh_, nwh_, l2segh1_, l2segh2_, intfh1_, intfh2_;

public:
    static void route_lookup_test(void *ptr);
};

hal_handle_t fwding_test_hw::eph_, fwding_test_hw::gwh_, fwding_test_hw::vrfh_, fwding_test_hw::nwh_,
    fwding_test_hw::l2segh1_, fwding_test_hw::l2segh2_, fwding_test_hw::intfh1_, fwding_test_hw::intfh2_;

TEST_F (fwding_test_hw, flow_benchmark)
{
    vector<Tins::EthernetII> pkts;
    const int num_flows = 100; // thousnads

    for (uint32_t sport = 1; sport <= num_flows; sport++) {
        for (uint32_t dport = 1; dport <= 1000; dport++) {
            Tins::TCP tcp = Tins::TCP(dport, sport);
            tcp.flags(Tins::TCP::SYN);

            Tins::EthernetII eth =
                Tins::EthernetII(Tins::HWAddress<6>("aa:bb:0a:00:00:02"),
                                 Tins::HWAddress<6>("aa:bb:0a:00:00:01")) /
                Tins::Dot1Q(100) /
                Tins::IP(Tins::IPv4Address(htonl(0x0A000002)), Tins::IPv4Address(htonl(0x0A000001))) /
                tcp;
            pkts.push_back(eth);
        }
    }

    inject_eth_pkt(fte::FLOW_MISS_LIFQ, intfh1_, l2segh1_, pkts);

    double total_secs = (double)time_profile_total(sdk::utils::time_profile::FTE_CTXT_INIT) / (double)1000000000ULL;
    printf("Total secs: %lf\n", total_secs);
    printf("Rate: %lf flows/sec\n", (double)(num_flows * 1000 / total_secs));
}
