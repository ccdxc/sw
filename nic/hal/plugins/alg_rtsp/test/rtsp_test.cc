#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/plugins/alg_rtsp/core.hpp"
#include "nic/p4/iris/include/defines.h"

#include <tins/tins.h>

using namespace hal::plugins::alg_rtsp;

class rtsp_test : public fte_base_test {
protected:
    rtsp_test() {}

    virtual ~rtsp_test() {}

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
        hal_handle_t intfh1 = add_uplink(1);
        hal_handle_t intfh2 =  add_uplink(2);
        client_eph = add_endpoint(l2segh, intfh1, 0x0A000001 , 0xAABB0A000001, 0);
        server_eph = add_endpoint(l2segh, intfh2, 0x0A000002 , 0xAABB0A000002, 0);

        // firewall rules
        std::vector<v4_rule_t> rules = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_TCP,
                               dport_low: RTSP_PORT, dport_high: RTSP_PORT,
                               alg: nwsec::APP_SVC_RTSP } },
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_DENY,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_TCP,
                               dport_low: 0, dport_high: 0xFFFF,
                               alg: nwsec::APP_SVC_NONE} },
        };

        add_nwsec_policy(vrfh, rules);
    }
    
    static hal_handle_t client_eph, server_eph;
};

hal_handle_t rtsp_test::client_eph, rtsp_test::server_eph;

TEST_F(rtsp_test, rtsp_session)
{
    hal_ret_t ret;

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(RTSP_PORT, 1000);
    tcp.flags(Tins::TCP::SYN);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.mcast_en);
    EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.mcast_en);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.session()->rflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);

    // TCP SYN re-transmit on ALG_CFLOW_LIFQ
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);


    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(1000, RTSP_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    // RTSP Setup
    tcp = Tins::TCP(RTSP_PORT, 1000) /
        Tins::RawPDU("SETUP rtsp://example.com/foo/bar/baz.rm RTSP/1.0\r\n"
                     "CSeq: 302\r\n"
                     "Transport: RTP/AVP;unicast;client_port=4588-4589\r\n"
                     "\r\n");
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    //RTSP Resp
    tcp = Tins::TCP(1000, RTSP_PORT) /
        Tins::RawPDU("RTSP/1.0 200 OK\r\n"
                     "CSeq: 302\r\n"
                     "Session: 47112344\r\n"
                     "Transport: RTP/AVP;unicast;\r\n"
                     "  source=10.0.0.2;destination=10.0.0.1;\r\n"
                     "  client_port=4588-4589;server_port=6256-6257\r\n"
                     "\r\n");

    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    // Check expected flows
    CHECK_ALLOW_UDP(client_eph, server_eph, 4588, 6256, "c:4588 <- s:6256");
    CHECK_ALLOW_UDP(client_eph, server_eph, 4589, 6257, "c:4589 <- s:6257");

    CHECK_DENY_UDP(server_eph, client_eph, 6256, 4588, "c:4588 -> s:6256");
    CHECK_DENY_UDP(server_eph, client_eph, 6257, 4589, "c:4589 -> s:6257");

    CHECK_DENY_UDP(client_eph, server_eph, 4589, 6256, "c:4589 <- s:6256");
    CHECK_DENY_UDP(client_eph, server_eph, 4588, 6257, "c:4588 <- s:6257");
    CHECK_DENY_UDP(server_eph, client_eph, 6257, 4588, "c:4588 -> s:6257");
    CHECK_DENY_UDP(server_eph, client_eph, 6256, 4589, "c:4589 -> s:6256");

    CHECK_DENY_UDP(client_eph, server_eph, 6256, 4588, "s:4588 -> c:6256");
    CHECK_DENY_UDP(client_eph, server_eph, 6257 ,4589, "s:4589 -> c:6257");
    CHECK_DENY_UDP(server_eph, client_eph, 4588, 6256, "s:4588 <- c:6256");
    CHECK_DENY_UDP(server_eph, client_eph, 4589, 6257, "s:4589 <- c:6257");
 }

