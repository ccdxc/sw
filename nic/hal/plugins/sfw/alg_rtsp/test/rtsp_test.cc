#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/plugins/sfw/alg_rtsp/core.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "nic/hal/test/utils/hal_test_utils.hpp"

#include <tins/tins.h>

using namespace hal::plugins::alg_rtsp;
using namespace fte;

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
        hal_handle_t intfh1 = add_uplink(PORT_NUM_1);
        hal_handle_t intfh2 =  add_uplink(PORT_NUM_2);
        client_eph = add_endpoint(l2segh, intfh1, 0x0A000001 , 0xAABB0A000001, 0);
        server_eph = add_endpoint(l2segh, intfh2, 0x0A000002 , 0xAABB0A000002, 0);
        client_eph1 = add_endpoint(l2segh, intfh1, 0x0A000101 , 0xAABB0A000003, 0);
        server_eph1 = add_endpoint(l2segh, intfh2, 0x0A000102 , 0xAABB0A000004, 0);
        sleep(100);

        // firewall rules
        std::vector<v4_rule_t> rules = {
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_ALLOW,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_TCP,
                               dport_low: RTSP_PORT, dport_high: RTSP_PORT,
                               alg: nwsec::APP_SVC_RTSP,
                               idle_timeout: 0x30} },
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_DENY,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_TCP,
                               dport_low: 0, dport_high: 0xFFFF,
                               alg: nwsec::APP_SVC_NONE,
                               idle_timeout: 0xFFFFFFFF} },
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_DENY,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_UDP,
                               dport_low: 0, dport_high: 0xFFFF,
                               alg: nwsec::APP_SVC_NONE,
                               idle_timeout: 0xFFFFFFFF} },
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_DENY,
                        from: {},
                        to: {},
                        app: { proto:0,
                               dport_low: 0, dport_high: 0xFFFF,
                               alg: nwsec::APP_SVC_NONE,
                               idle_timeout: 0xFFFFFFFF} },
        };

        add_nwsec_policy(vrfh, rules);
    }
    
    static hal_handle_t client_eph, server_eph, client_eph1, server_eph1;

public:
    static void rtsp_session_create(void *);
};

hal_handle_t rtsp_test::client_eph, rtsp_test::server_eph, rtsp_test::client_eph1, rtsp_test::server_eph1;

TEST_F(rtsp_test, rtsp_session)
{
    hal_ret_t ret;
    hal::session_t *session = NULL;

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(RTSP_PORT, 1000);
    tcp.flags(Tins::TCP::SYN);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.mcast_en);
    EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.mcast_en);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.session()->rflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    session = ctx_.session();
    EXPECT_NE(session->sfw_rule_id, 0);
    EXPECT_EQ(session->skip_sfw_reval, 0);
    EXPECT_EQ(session->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.session()->idle_timeout, 0x30);

    // TCP SYN re-transmit
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, 0);
    EXPECT_EQ(ret, HAL_RET_OK);


    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(1000, RTSP_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    //TCP ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(RTSP_PORT, 1000);
    tcp.flags(Tins::TCP::ACK);
    tcp.seq(1);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp, true);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    // RTSP Setup
    tcp = Tins::TCP(RTSP_PORT, 1000) /
        Tins::RawPDU("SETUP rtsp://example.com/foo/bar/baz.rm RTSP/1.0\r\n"
                     "CSeq: 302\r\n"
                     "Transport: RTP/AVP;unicast;client_port=4588-4589\r\n"
                     "\r\n");
    tcp.seq(1);
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
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    // Check expected flows
    CHECK_ALLOW_UDP(server_eph, client_eph, 6256, 4588, "c:4588 -> s:6256");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_RTSP);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    EXPECT_EQ(ctx_.session()->skip_sfw_reval, 1);
    EXPECT_EQ(ctx_.session()->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.session()->sfw_rule_id, 0);
    EXPECT_EQ(ctx_.session()->idle_timeout, 0x30);
    CHECK_ALLOW_UDP(server_eph, client_eph, 6257, 4589, "c:4589 -> s:6257");
    CHECK_ALLOW_UDP(client_eph, server_eph, 4588, 6256, "c:4588 <- s:6256");
    CHECK_ALLOW_UDP(client_eph, server_eph, 4589, 6257, "c:4589 <- s:6257");

    CHECK_DENY_UDP(client_eph, server_eph, 4589, 6256, "c:4589 <- s:6256");
    CHECK_DENY_UDP(client_eph, server_eph, 4588, 6257, "c:4588 <- s:6257");
    CHECK_DENY_UDP(server_eph, client_eph, 6257, 4588, "c:4588 -> s:6257");
    CHECK_DENY_UDP(server_eph, client_eph, 6256, 4589, "c:4589 -> s:6256");

    CHECK_DENY_UDP(client_eph, server_eph, 6256, 4588, "s:4588 -> c:6256");
    CHECK_DENY_UDP(client_eph, server_eph, 6257 ,4589, "s:4589 -> c:6257");
    CHECK_DENY_UDP(server_eph, client_eph, 4588, 6256, "s:4588 <- c:6256");
    CHECK_DENY_UDP(server_eph, client_eph, 4589, 6257, "s:4589 <- c:6257");
}

TEST_F(rtsp_test, app_sess_force_delete) {
    SessionDeleteResponseMsg delresp;
    SessionGetResponseMsg    resp1, resp2;
    hal::session_t          *session = NULL;
    hal_ret_t                ret;
    hal_handle_t             sess_hdl = 0;
    uint8_t                alg_sessions = 0;

    ret = hal::session_get_all(&resp1);
    EXPECT_EQ(ret, HAL_RET_OK);
    for (int idx=0; idx<resp1.response_size(); idx++) {
        SessionGetResponse rsp = resp1.response(idx);
        if (rsp.status().has_rtsp_info() &&
            rsp.status().rtsp_info().iscontrol()) {
            sess_hdl = rsp.status().session_handle();
        }
    }

    // Invoke delete callback
    session = hal::find_session_by_handle(sess_hdl);
    ASSERT_TRUE(session != NULL);
    ret = session_delete(session, true);
    ASSERT_EQ(ret, HAL_RET_OK);

    ret = hal::session_get_all(&resp2);
    EXPECT_EQ(ret, HAL_RET_OK);
    for (int idx=0; idx<resp2.response_size(); idx++) {
        SessionGetResponse rsp = resp2.response(idx);
        if (rsp.status().alg() == nwsec::APP_SVC_RTSP)
            alg_sessions++;
    }
    EXPECT_EQ(alg_sessions, 0);
}

TEST_F(rtsp_test, rtsp_session_interleaved)
{
    hal_ret_t              ret;
    SessionGetResponseMsg  resp;
    uint8_t                alg_sessions = 0, ctrl_alg_sessions = 0;
    hal::session_t        *session = NULL;
    
    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(RTSP_PORT, 1000);
    tcp.flags(Tins::TCP::SYN);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.mcast_en);
    EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.mcast_en);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.session()->rflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    session = ctx_.session();

    // TCP SYN re-transmit on ALG_CFLOW_LIFQ
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);


    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(1000, RTSP_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    // RTSP Setup
    tcp = Tins::TCP(RTSP_PORT, 1000) /
        Tins::RawPDU("SETUP rtsp://example.com/foo/bar/baz.rm RTSP/1.0\r\n"
                     "CSeq: 302\r\n"
                     "Transport: RTP/AVP/TCP;interleaved=0-1\r\n"
                     "\r\n");

    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    //RTSP Resp
    tcp = Tins::TCP(1000, RTSP_PORT) /
        Tins::RawPDU("RTSP/1.0 200 OK\r\n"
                     "CSeq: 302\r\n"
                     "Session: 47112344\r\n"
                     "Transport: RTP/AVP/TCP;interleaved=0-1\r\n");
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    ret = hal::session_get_all(&resp);
    EXPECT_EQ(ret, HAL_RET_OK);
    for (int idx=0; idx<resp.response_size(); idx++) {
        SessionGetResponse rsp = resp.response(idx);
        if (rsp.status().has_rtsp_info()) {
            alg_sessions++;
            if (rsp.status().rtsp_info().iscontrol())
               ctrl_alg_sessions++;
        }
    }
    EXPECT_EQ(alg_sessions, 1);
    EXPECT_EQ(ctrl_alg_sessions, 1);

    ret = session_delete(session, true);
    ASSERT_EQ(ret, HAL_RET_OK); 
}

TEST_F(rtsp_test, rtsp_session_setup_error_response)
{
    hal_ret_t                ret;
    hal::session_t          *session = NULL;
    SessionGetResponseMsg    resp;
    uint8_t                  alg_sessions = 0, ctrl_alg_sessions = 0;

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(RTSP_PORT, 3000);
    tcp.flags(Tins::TCP::SYN);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.mcast_en);
    EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.mcast_en);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.session()->rflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    session = ctx_.session();

    // TCP SYN re-transmit on ALG_CFLOW_LIFQ
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);


    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(3000, RTSP_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    //TCP ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(RTSP_PORT, 3000);
    tcp.flags(Tins::TCP::ACK);
    tcp.seq(1);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    // RTSP Setup
    tcp = Tins::TCP(RTSP_PORT, 3000) /
        Tins::RawPDU("SETUP rtsp://example.com/foo/bar/baz.rm RTSP/1.0\r\n"
                     "CSeq: 302\r\n"
                     "Transport: RTP/AVP;unicast;client_port=4588-4589\r\n"
                     "\r\n");
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    //RTSP Resp
    tcp = Tins::TCP(3000, RTSP_PORT) /
        Tins::RawPDU("RTSP/1.0 453 Not Enough Bandwidth\r\n"
                     "CSeq: 302\r\n"
                     "Server: foobarbaz\r\n"
                     "\r\n");
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    CHECK_DENY_UDP(client_eph, server_eph, 4588, 6256, "c:4588 <- s:6256");
    CHECK_DENY_UDP(client_eph, server_eph, 4589, 6257, "c:4589 <- s:6257");
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

    ret = hal::session_get_all(&resp);
    EXPECT_EQ(ret, HAL_RET_OK);
    for (int idx=0; idx<resp.response_size(); idx++) {
        SessionGetResponse rsp = resp.response(idx);
        if (rsp.status().alg() == nwsec::APP_SVC_RTSP) {
            alg_sessions++;
            if (rsp.status().rtsp_info().iscontrol())
                ctrl_alg_sessions++;
        }
    }
    EXPECT_EQ(alg_sessions, 1);
    EXPECT_EQ(ctrl_alg_sessions, 1);

    ret = session_delete(session, true);
    ASSERT_EQ(ret, HAL_RET_OK);
} 

TEST_F(rtsp_test, rtsp_session_cseq_error)
{
    hal_ret_t ret;
    hal::session_t *session = NULL;
    uint8_t         alg_sessions = 0, ctrl_alg_sessions = 0;
    SessionGetResponseMsg    resp;

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(RTSP_PORT, 4000);
    tcp.flags(Tins::TCP::SYN);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.mcast_en);
    EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.mcast_en);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.session()->rflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    session = ctx_.session();

    // TCP SYN re-transmit on ALG_CFLOW_LIFQ
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);


    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(4000, RTSP_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    //TCP ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(RTSP_PORT, 4000);
    tcp.flags(Tins::TCP::ACK);
    tcp.seq(1);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    // RTSP Setup
    tcp = Tins::TCP(RTSP_PORT, 4000) /
        Tins::RawPDU("SETUP rtsp://example.com/foo/bar/baz.rm RTSP/1.0\r\n"
                     "CSeq: 302\r\n"
                     "Transport: RTP/AVP;unicast;client_port=5588-5589\r\n"
                     "\r\n");
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    //RTSP Resp
    tcp = Tins::TCP(4000, RTSP_PORT) /
        Tins::RawPDU("RTSP/1.0 200 OK\r\n"
                     "Session: 47112344\r\n"
                     "Transport: RTP/AVP;unicast;\r\n"
                     "  source=10.0.0.2;destination=10.0.0.1;\r\n"
                     "  client_port=5588-5589;server_port=7256-7257\r\n"
                     "\r\n");
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    // Check expected flows
    CHECK_DENY_UDP(client_eph, server_eph, 5588, 7256, "c:5588 <- s:7256");
    CHECK_DENY_UDP(client_eph, server_eph, 5589, 7257, "c:5589 <- s:7257");
    CHECK_DENY_UDP(server_eph, client_eph, 7256, 5588, "c:5588 -> s:7256");
    CHECK_DENY_UDP(server_eph, client_eph, 7257, 5589, "c:5589 -> s:7257");

    CHECK_DENY_UDP(client_eph, server_eph, 5589, 7256, "c:5589 <- s:7256");
    CHECK_DENY_UDP(client_eph, server_eph, 5588, 7257, "c:5588 <- s:7257");
    CHECK_DENY_UDP(server_eph, client_eph, 7257, 5588, "c:5588 -> s:7257");
    CHECK_DENY_UDP(server_eph, client_eph, 7256, 5589, "c:5589 -> s:7256");

    CHECK_DENY_UDP(client_eph, server_eph, 7256, 5588, "s:5588 -> c:7256");
    CHECK_DENY_UDP(client_eph, server_eph, 7257 ,5589, "s:5589 -> c:7257");
    CHECK_DENY_UDP(server_eph, client_eph, 5588, 7256, "s:5588 <- c:7256");
    CHECK_DENY_UDP(server_eph, client_eph, 5589, 7257, "s:5589 <- c:7257");

    ret = hal::session_get_all(&resp);
    EXPECT_EQ(ret, HAL_RET_OK);
    for (int idx=0; idx<resp.response_size(); idx++) {
        SessionGetResponse rsp = resp.response(idx);
        if (rsp.status().alg() == nwsec::APP_SVC_RTSP) {
            if (rsp.status().rtsp_info().iscontrol()) {
                ctrl_alg_sessions++;
                EXPECT_EQ(rsp.status().rtsp_info().parse_errors(), 1);
            } else {
                alg_sessions++;
            }
        }
    }
    EXPECT_EQ(alg_sessions, 0);
    EXPECT_EQ(ctrl_alg_sessions, 1);

    ret = session_delete(session, true);
    ASSERT_EQ(ret, HAL_RET_OK);
} 

TEST_F(rtsp_test, rtsp_session_id_error)
{
    hal_ret_t ret;
    hal::session_t *session = NULL;
    uint8_t         alg_sessions = 0, ctrl_alg_sessions = 0;
    SessionGetResponseMsg    resp;

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(RTSP_PORT, 4100);
    tcp.flags(Tins::TCP::SYN);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.mcast_en);
    EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.mcast_en);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.session()->rflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    session = ctx_.session();

    // TCP SYN re-transmit on ALG_CFLOW_LIFQ
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);


    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(4100, RTSP_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    //TCP ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(RTSP_PORT, 4100);
    tcp.flags(Tins::TCP::ACK);
    tcp.seq(1);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    // RTSP Setup Track1
    tcp = Tins::TCP(RTSP_PORT, 4100) /
        Tins::RawPDU("SETUP rtsp://192.168.100.101/small.vob/track1 RTSP/1.0\r\n"
                     "CSeq: 4\r\n"
                     "User-Agent: openRTSP (LIVE555 Streaming Media v2013.11.26)\r\n"
                     "Transport: RTP/AVP;unicast;client_port=57692-57693\r\n");
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    //RTSP Resp
    tcp = Tins::TCP(4100, RTSP_PORT) /
        Tins::RawPDU("RTSP/1.0 200 OK\r\n"
                     "CSeq: 4\r\n"
                     "Date: Tue, Dec 18 2018 19:58:12 GMT\r\n"
                     "Transport: RTP/AVP;unicast;client_port=57692-57693;server_port=6970-6971\r\n"
                     "Session: B1A845B1\r\n"
                     "\r\n");
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    // RTSP TEARDOWN
    tcp = Tins::TCP(RTSP_PORT, 4100) /
        Tins::RawPDU("TEARDOWN rtsp://192.168.100.101/small.vob/ RTSP/1.0\r\n"
                     "CSeq: 5\r\n"
                     "User-Agent: openRTSP (LIVE555 Streaming Media v2013.11.26)\r\n"
                     "Session: B1A845\r\n"
                     "\r\n");
    tcp.seq(178);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    // Check expected flows
    CHECK_ALLOW_UDP(server_eph, client_eph, 6970, 57692, "c:57692 -> s:6970");
    CHECK_ALLOW_UDP(server_eph, client_eph, 6971, 57693, "c:57693 -> s:6971");
    CHECK_ALLOW_UDP(client_eph, server_eph, 57692, 6970, "c:57592 <- s:6970");
    CHECK_ALLOW_UDP(client_eph, server_eph, 57693, 6971, "c:57693 <- s:6971");

    CHECK_DENY_UDP(client_eph, server_eph, 57693, 6970, "c:57693 <- s:6970");
    CHECK_DENY_UDP(client_eph, server_eph, 57692, 6971, "c:57692 <- s:6971");
    CHECK_DENY_UDP(server_eph, client_eph, 6970, 57693, "c:6970 -> s:57693");
    CHECK_DENY_UDP(server_eph, client_eph, 6971, 57692, "c:6971 -> s:57692");

    CHECK_DENY_UDP(client_eph, server_eph, 6970, 57692, "s:57692 <- c:6970");
    CHECK_DENY_UDP(client_eph, server_eph, 6971, 57693, "s:57693 <- c:6971");
    CHECK_DENY_UDP(server_eph, client_eph, 57692, 6970, "c:6970 -> s:57692");
    CHECK_DENY_UDP(server_eph, client_eph, 57692, 6971, "c:6971 -> s:57693");
 
    ret = hal::session_get_all(&resp);
    EXPECT_EQ(ret, HAL_RET_OK);
    for (int idx=0; idx<resp.response_size(); idx++) {
        SessionGetResponse rsp = resp.response(idx);
        if (rsp.status().alg() == nwsec::APP_SVC_RTSP) {
            alg_sessions++;
            if (rsp.status().rtsp_info().iscontrol()) {
                ctrl_alg_sessions++;
            }
        }
    }
    EXPECT_EQ(alg_sessions, 3);
    EXPECT_EQ(ctrl_alg_sessions, 1);
}

TEST_F(rtsp_test, rtsp_session_session_delete)
{
    hal_ret_t ret;
    hal::session_t *session = NULL, *data_session=NULL;

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(RTSP_PORT, 1003);
    tcp.flags(Tins::TCP::SYN);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.mcast_en);
    EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.mcast_en);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.session()->rflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    session = ctx_.session();
    EXPECT_NE(session->sfw_rule_id, 0);
    EXPECT_EQ(session->skip_sfw_reval, 0);
    EXPECT_EQ(session->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.session()->idle_timeout, 0x30);

    tcp = Tins::TCP(1003, RTSP_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    //TCP ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(RTSP_PORT, 1003);
    tcp.flags(Tins::TCP::ACK);
    tcp.seq(1);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    // RTSP Setup
    tcp = Tins::TCP(RTSP_PORT, 1003) /
        Tins::RawPDU("SETUP rtsp://example.com/foo/bar/baz.rm RTSP/1.0\r\n"
                     "CSeq: 302\r\n"
                     "Transport: RTP/AVP;unicast;client_port=4588-4589\r\n"
                     "\r\n");
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    //RTSP Resp
    tcp = Tins::TCP(1003, RTSP_PORT) /
        Tins::RawPDU("RTSP/1.0 200 OK\r\n"
                     "CSeq: 302\r\n"
                     "Session: 47112344\r\n"
                     "Transport: RTP/AVP;unicast;\r\n"
                     "  source=10.0.0.2;destination=10.0.0.1;\r\n"
                     "  client_port=4688-4689;server_port=6256-6257\r\n"
                     "\r\n");
    tcp.seq(1);

    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());

    // Check expected flows
    CHECK_ALLOW_UDP(server_eph, client_eph, 6256, 4688, "c:4588 -> s:6256");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_RTSP);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    EXPECT_EQ(ctx_.session()->skip_sfw_reval, 1);
    EXPECT_EQ(ctx_.session()->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    data_session = ctx_.session();

    ret = session_delete(session);
    ASSERT_EQ(ret, HAL_RET_INVALID_CTRL_SESSION_OP);

    ret = session_delete(data_session, true);
    ASSERT_EQ(ret, HAL_RET_OK);
}
