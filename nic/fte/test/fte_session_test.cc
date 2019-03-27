#include "fte_base_test.hpp"
#include <gtest/gtest.h>
#include "nic/fte/fte.hpp"
#include <pthread.h>

using namespace fte;

class fte_session_test;

class fte_session_test : public fte_base_test {
protected:
    fte_session_test() {}

    virtual ~fte_session_test() {}

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
        l2segh = add_l2segment(nwh, 100);
        intfh1 = add_uplink(1);
        intfh2 = add_uplink(2);
        intfh3 =  add_enic(l2segh, 100, 0x0000DEADBEEF, intfh1);
        client_eph = add_endpoint(l2segh, intfh1, 0x0A000001 , 0xAABB0A000001, 0);
        server_eph = add_endpoint(l2segh, intfh2, 0x0A000002 , 0xAABB0A000002, 0);
        client2_eph = add_endpoint(l2segh, intfh3, 0x0, 0xAABB0A000003, 0);

        // firewall rules
        std::vector<v4_rule_t> rules = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW }
        };

        add_nwsec_policy(vrfh, rules); 
    }

    static hal_handle_t client_eph, server_eph, intfh1, l2segh, intfh2, intfh3, client2_eph;

public:
    static void fte_session_create();
};

hal_handle_t fte_session_test::client_eph, fte_session_test::server_eph, 
                  fte_session_test::intfh1, fte_session_test::l2segh, fte_session_test::intfh2;
hal_handle_t fte_session_test::client2_eph, fte_session_test::intfh3;

void fte_session_test::fte_session_create()
{
    hal_ret_t  ret = HAL_RET_OK;

    Tins::TCP tcp = Tins::TCP(100, 101);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
}

void* send_flow_miss(void *ptr)
{
    fte_session_test *test = (fte_session_test *)ptr;

    for (int i=0; i<10; i++) {
        test->fte_session_create();
    }

    return NULL;
}

void* send_delete_all(void *ptr)
{
    SessionDeleteResponseMsg delrsp;

    for (int i=0; i<10; i++) {
         hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
         hal::session_delete_all(&delrsp);
         hal::hal_cfg_db_close();
    }
 
    return NULL;
}

void* send_get_all(void *ptr)
{
    SessionGetResponseMsg    getrsp;

    for (int i=0; i<10; i++) {
         hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
         hal::session_get_all(&getrsp);
         hal::hal_cfg_db_close();
    }

    return NULL;
}

TEST_F(fte_session_test, session_create)
{
    pthread_t mThreadID1, mThreadID2, mThreadID3;
    pthread_create(&mThreadID1, NULL, send_flow_miss, (void *)this);
    pthread_create(&mThreadID2, NULL, send_delete_all, NULL);
    pthread_create(&mThreadID3, NULL, send_get_all, NULL); 
    pthread_join(mThreadID1, NULL);
    pthread_join(mThreadID2, NULL);
    pthread_join(mThreadID3, NULL);
}

TEST_F(fte_session_test, fte_stats)
{
    fte::fte_stats_t    stats;
    vector<Tins::EthernetII> pkts;
    const int num_flows = 10;
    uint64_t  softq_req = 0;

    stats = fte::fte_stats_get(FTE_ID);
    softq_req = stats.fte_hbm_stats->qstats.softq_req;
    cout << "softq req: " << softq_req++ << endl;

    for (uint32_t sport = 1; sport <= num_flows; sport++) {
        for (uint32_t dport = 1; dport <= 5; dport++) {
            Tins::TCP tcp = Tins::TCP(dport, sport);
            tcp.flags(Tins::TCP::SYN);

            Tins::EthernetII eth =
                Tins::EthernetII(Tins::HWAddress<6>("aa:bb:0a:00:00:02"),
                                 Tins::HWAddress<6>("aa:bb:0a:00:00:01")) /
                Tins::Dot1Q(100) /
                Tins::IP(Tins::IPv4Address(htonl(0x0A000002)), Tins::IPv4Address(htonl(0x0A000001))) /
                tcp;
            pkts.push_back(eth);
            inject_eth_pkt(fte::FLOW_MISS_LIFQ, intfh1, l2segh, pkts);
        }
    }
    stats = fte::fte_stats_get(FTE_ID);
    EXPECT_NE(stats.fte_hbm_stats->cpsstats.cps, 0);
    EXPECT_EQ(stats.fte_hbm_stats->qstats.softq_req, softq_req+50);
    cout << "CPS: " << stats.fte_hbm_stats->cpsstats.cps << endl;
    cout << "Max. CPS: " << stats.fte_hbm_stats->cpsstats.cps_hwm << endl;
}

TEST_F (fte_session_test, fte_test_local_remote_ep_lookup)
{
    hal_ret_t ret;
    hal::session_t *session = NULL;
    hal::flow_t    *flow = NULL;
    hal::ep_t      *sep = NULL, *dep = NULL;
    hal::l2seg_t   *l2seg = NULL;

    // Create TCP session
    Tins::TCP tcp = Tins::TCP(5000, 5001);
    tcp.flags(Tins::TCP::SYN);

    Tins::EthernetII eth =
         Tins::EthernetII(Tins::HWAddress<6>("aa:bb:0a:00:00:04"),
                          Tins::HWAddress<6>("aa:bb:0a:00:00:03")) /
                          Tins::Dot1Q(100) /
                          Tins::IP(Tins::IPv4Address(htonl(0x0A000004)), Tins::IPv4Address(htonl(0x0A000003))) /
                          tcp;

    ret = inject_eth_pkt(fte::FLOW_MISS_LIFQ, intfh3, l2segh, eth);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_NE(ctx_.session(), nullptr);
    EXPECT_EQ(ctx_.sif()->hal_handle, intfh3);
    EXPECT_EQ(ctx_.sl2seg()->hal_handle, l2segh);

    session = ctx_.session();
    flow = session->iflow;
    ret = hal::ep_get_from_flow_key(&flow->config.key, &sep, &dep);
    ASSERT_NE(ret, HAL_RET_OK);
    sep = hal::find_ep_by_l2_key(flow->config.l2_info.l2seg_id, flow->config.l2_info.smac);
    dep = hal::find_ep_by_l2_key(flow->config.l2_info.l2seg_id, flow->config.l2_info.dmac);

    EXPECT_EQ(dep, nullptr);
    EXPECT_NE(sep, nullptr);
    l2seg = hal::l2seg_lookup_by_handle(sep->l2seg_handle);
    ASSERT_NE(l2seg, nullptr);
    ASSERT_EQ(sep->l2seg_handle, l2segh); 
}

TEST_F (fte_session_test, fte_test_local_local_ep_lookup)
{
    hal_ret_t ret;
    hal::session_t *session = NULL;
    hal::flow_t    *flow = NULL;
    hal::ep_t      *l3sep = NULL, *l3dep = NULL;
    hal::ep_t      *l2sep = NULL, *l2dep = NULL;
    hal::l2seg_t   *l2seg = NULL;

    // Create TCP session
    Tins::TCP tcp = Tins::TCP(5000, 5001);
    tcp.flags(Tins::TCP::SYN);

    Tins::EthernetII eth =
         Tins::EthernetII(Tins::HWAddress<6>("aa:bb:0a:00:00:01"),
                          Tins::HWAddress<6>("aa:bb:0a:00:00:02")) /
                          Tins::Dot1Q(100) /
                          Tins::IP(Tins::IPv4Address(htonl(0x0A000001)), Tins::IPv4Address(htonl(0x0A000002))) /
                          tcp;

    ret = inject_eth_pkt(fte::FLOW_MISS_LIFQ, intfh2, l2segh, eth);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_NE(ctx_.session(), nullptr);
    EXPECT_EQ(ctx_.sif()->hal_handle, intfh2);
    EXPECT_EQ(ctx_.sl2seg()->hal_handle, l2segh);

    session = ctx_.session();
    flow = session->iflow;
    ret = hal::ep_get_from_flow_key(&flow->config.key, &l3sep, &l3dep);
    ASSERT_EQ(ret, HAL_RET_OK);
    l2sep = hal::find_ep_by_l2_key(flow->config.l2_info.l2seg_id, flow->config.l2_info.smac);
    l2dep = hal::find_ep_by_l2_key(flow->config.l2_info.l2seg_id, flow->config.l2_info.dmac);
    ASSERT_EQ(l3dep, l2dep);
    ASSERT_EQ(l3sep, l2sep);

    EXPECT_NE(l3dep, nullptr);
    EXPECT_NE(l3sep, nullptr);
    l2seg = hal::l2seg_lookup_by_handle(l3sep->l2seg_handle);
    ASSERT_NE(l2seg, nullptr);
    ASSERT_EQ(l3sep->l2seg_handle, l2segh);
}
