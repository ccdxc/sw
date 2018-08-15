#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/include/pd_api.hpp"
#include "nic/include/fte.hpp"
#include <vector>

#include <tins/tins.h>

using namespace fte;
using namespace std;

class fwding_test : public fte_base_test {
protected:
    fwding_test() {}

    virtual ~fwding_test() {}

    // will be called immediately after the constructor before each test
    virtual void SetUp() {}

    // will be called immediately after each test before the destructor
    virtual void TearDown() {}

    // Will be called at the beginning of all test cases in this class
    static void SetUpTestCase() {
        setenv("DISABLE_LOGGING", "true", 1);
        fte_base_test::SetUpTestCase();

        // create topo
        vrfh_ = add_vrf();
        nwh_ = add_network(vrfh_, 0x0A000000, 24, 0xAABB0A000000);
        l2segh1_ = add_l2segment(nwh_, 100);
        l2segh2_ = add_l2segment(nwh_, 200);
        intfh1_ = add_uplink(1);
        intfh2_ =  add_uplink(2);
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

hal_handle_t fwding_test::eph_, fwding_test::gwh_, fwding_test::vrfh_, fwding_test::nwh_,
    fwding_test::l2segh1_, fwding_test::l2segh2_, fwding_test::intfh1_, fwding_test::intfh2_;


TEST_F (fwding_test, route_lookup)
{
    hal_ret_t ret;

    return;

    // Create TCP session
    Tins::TCP tcp = Tins::TCP(80, 1000);
    tcp.flags(Tins::TCP::SYN);

    Tins::EthernetII eth =
        Tins::EthernetII(Tins::HWAddress<6>("aa:bb:cc:dd:ee:ff"),
                         Tins::HWAddress<6>("aa:bb:0a:00:00:01")) /
        Tins::Dot1Q(100) /
        Tins::IP(Tins::IPv4Address(htonl(0x0A0001AA)), Tins::IPv4Address(htonl(0x0A000001))) /
        tcp;

    ret = inject_eth_pkt(fte::FLOW_MISS_LIFQ, intfh1_, l2segh1_, eth);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_NE(ctx_.session(), nullptr);
    EXPECT_EQ(ctx_.dif()->hal_handle, intfh2_);
    EXPECT_EQ(ctx_.dl2seg()->hal_handle, l2segh2_);

    hal::pd::pd_if_get_lport_id_args_t args;
    hal::pd::pd_func_args_t pd_func_args = {0};
    args.pi_if = hal::find_if_by_handle(intfh2_);
    pd_func_args.pd_if_get_lport_id = &args;
    hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_IF_GET_LPORT_ID, &pd_func_args);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.lport, args.lport_id);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mac_sa_rewrite, 1);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mac_da_rewrite, 1);
}

void timeit(const std::string &msg, int count, std::function<void()> fn)
{
    cout << msg << " " << count << " " << std::flush;

    std::clock_t start = clock();
    fn();
    int ticks = clock()-start;

    cout << " (" << 1000.0*ticks/CLOCKS_PER_SEC << " ms) ";
    if (count) {
        cout << count*CLOCKS_PER_SEC/ticks << "/sec";
    }
    cout << "\n";
}

TEST_F (fwding_test, flow_benchmark)
{
    vector<Tins::EthernetII> pkts;
    const int num_flows = 1000; // thousnads

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

    timeit("inject_pkts", pkts.size(), [&]() {
            inject_eth_pkt(fte::FLOW_MISS_LIFQ, intfh1_, l2segh1_, pkts);
        });

}

