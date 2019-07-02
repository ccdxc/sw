#include <gtest/gtest.h>
#include "nic/hal/plugins/alg_ftp/core.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "ftp_test.hpp"

#include <tins/tins.h>

using namespace hal::plugins::alg_ftp;
using namespace session;
using namespace fte;

hal_handle_t ftp_test::client_eph, ftp_test::server_eph, ftp_test::client_eph1, ftp_test::server_eph1;
hal_handle_t ftp_test::client_eph2, ftp_test::server_eph2;

TEST_F(ftp_test, ftp_session)
{
    SessionGetRequest       req;
    SessionGetResponseMsg   rsp;
    hal_ret_t          ret;
    uint8_t            ftp_port[] = {0x50, 0x4f, 0x52, 0x54, 0x20, 0x31, 0x30, 0x2c, 
                                   0x30, 0x2c, 0x30, 0x2c, 0x31, 0x2c, 0x31, 0x34,
                                   0x34, 0x2c, 0x32, 0x31, 0x31, 0x0d, 0x0a};
    uint8_t            ftp_port_rsp[] = {0x32, 0x30, 0x30, 0x20, 0x50, 0x4f, 0x52, 0x54,
                                       0x20, 0x63, 0x6f, 0x6d, 0x6d, 0x61, 0x6e, 0x64,
                                       0x20, 0x73, 0x75, 0x63, 0x63, 0x65, 0x73, 0x73,
                                       0x66, 0x75, 0x6c, 0x2e, 0x20, 0x43, 0x6f, 0x6e,
                                       0x73, 0x69, 0x64, 0x65, 0x72, 0x20, 0x75, 0x73,
                                       0x69, 0x6e, 0x67, 0x20, 0x50, 0x41, 0x53, 0x56,
                                       0x2e, 0x0d, 0x0a};

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(FTP_PORT, 2000);
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
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_FTP);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    hal::session_t *session = ctx_.session();
    EXPECT_NE(session->sfw_rule_id, 0);
    EXPECT_EQ(session->skip_sfw_reval, 0);
    EXPECT_EQ(session->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.session()->idle_timeout, 30);

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(2000, FTP_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);
 
    //TCP ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(FTP_PORT, 2000);
    tcp.flags(Tins::TCP::ACK);
    tcp.seq(1);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //FTP PORT Command
    tcp = Tins::TCP(FTP_PORT, 2000) /
          Tins::RawPDU(ftp_port, sizeof(ftp_port));
    tcp.seq(1);
    //tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    //tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    //tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //FTP PORT Response
    tcp = Tins::TCP(2000, FTP_PORT) /
         Tins::RawPDU(ftp_port_rsp, sizeof(ftp_port_rsp));
    tcp.seq(1);
    //tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    //tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    //tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    CHECK_ALLOW_TCP(client_eph, server_eph, 37075, FTP_DATA_PORT, "c:20 -> s:37075");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_FTP);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    EXPECT_EQ(ctx_.session()->skip_sfw_reval, 1);
    EXPECT_EQ(ctx_.session()->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.session()->sfw_rule_id, 0);
    EXPECT_EQ(ctx_.session()->idle_timeout, 30);
    CHECK_DENY_TCP(client_eph, server_eph, 37075, 2000, "c:2000 -> s:37075");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_DENY);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_NONE);
    EXPECT_EQ(ctx_.session()->skip_sfw_reval, 0);
    EXPECT_EQ(ctx_.session()->sfw_action, nwsec::SECURITY_RULE_ACTION_DENY);
    EXPECT_NE(ctx_.session()->sfw_rule_id, 0);
    CHECK_DENY_TCP(server_eph, client_eph,  37075, 2000, "c:2000 -> s:37075"); 
    CHECK_DENY_TCP(client_eph, server_eph, 37075, 2001, "c:2001 -> s:37075");

    req.set_session_handle(session->hal_handle);
    ret = hal::session_get(req, &rsp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(rsp.response(0).status().alg(), nwsec::APP_SVC_FTP);
    EXPECT_TRUE(rsp.response(0).status().has_ftp_info());
    EXPECT_EQ(rsp.response(0).status().ftp_info().parse_error(), 0);
}

TEST_F(ftp_test, ftp_session_allow_mismatch)
{
    SessionGetRequest       req;
    SessionGetResponseMsg   rsp;
    hal_ret_t          ret;
    uint8_t            ftp_port[] = {0x50, 0x4f, 0x52, 0x54, 0x20, 0x31, 0x30, 0x2c,
                                   0x30, 0x2c, 0x30, 0x2c, 0x31, 0x2c, 0x31, 0x34,
                                   0x34, 0x2c, 0x32, 0x31, 0x31, 0x0d, 0x0a};
    uint8_t            ftp_port_rsp[] = {0x32, 0x30, 0x30, 0x20, 0x50, 0x4f, 0x52, 0x54,
                                       0x20, 0x63, 0x6f, 0x6d, 0x6d, 0x61, 0x6e, 0x64,
                                       0x20, 0x73, 0x75, 0x63, 0x63, 0x65, 0x73, 0x73,
                                       0x66, 0x75, 0x6c, 0x2e, 0x20, 0x43, 0x6f, 0x6e,
                                       0x73, 0x69, 0x64, 0x65, 0x72, 0x20, 0x75, 0x73,
                                       0x69, 0x6e, 0x67, 0x20, 0x50, 0x41, 0x53, 0x56,
                                       0x2e, 0x0d, 0x0a};

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(FTP_PORT, 2000);
    tcp.flags(Tins::TCP::SYN);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph1, client_eph1, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.mcast_en);
    EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.mcast_en);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.session()->rflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_FTP);
    hal::session_t *session = ctx_.session();

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(2000, FTP_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph1, server_eph1, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //TCP ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(FTP_PORT, 2000);
    tcp.flags(Tins::TCP::ACK);
    tcp.seq(1);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph1, client_eph1, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //FTP PORT Command
    tcp = Tins::TCP(FTP_PORT, 2000) /
          Tins::RawPDU(ftp_port, sizeof(ftp_port));
    tcp.seq(1);
    //tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    //tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    //tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph1, client_eph1, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //FTP PORT Response
    tcp = Tins::TCP(2000, FTP_PORT) /
         Tins::RawPDU(ftp_port_rsp, sizeof(ftp_port_rsp));
    tcp.seq(1);
    //tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    //tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    //tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph1, server_eph1, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    CHECK_ALLOW_TCP(client_eph, server_eph1, 37075, FTP_DATA_PORT, "c:20 -> s:37075");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_FTP);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    CHECK_DENY_TCP(client_eph, server_eph1, 37075, 2000, "c:2000 -> s:37075");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_DENY);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_NONE);
    CHECK_DENY_TCP(server_eph1, client_eph,  37075, 2000, "c:2000 -> s:37075");
    CHECK_DENY_TCP(client_eph, server_eph1, 37075, 2001, "c:2001 -> s:37075");

    req.set_session_handle(session->hal_handle);
    ret = hal::session_get(req, &rsp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(rsp.response(0).status().alg(), nwsec::APP_SVC_FTP);
    EXPECT_TRUE(rsp.response(0).status().has_ftp_info());
    EXPECT_EQ(rsp.response(0).status().ftp_info().parse_error(), 0);
}

TEST_F(ftp_test, ftp_session_not_allow_mismatch)
{
    SessionGetRequest  req;
    SessionGetResponse rsp;
    hal_ret_t          ret;
    uint8_t            ftp_port[] = {0x50, 0x4f, 0x52, 0x54, 0x20, 0x31, 0x30, 0x2c,
                                   0x30, 0x2c, 0x30, 0x2c, 0x31, 0x2c, 0x31, 0x34,
                                   0x34, 0x2c, 0x32, 0x31, 0x31, 0x0d, 0x0a};
    uint8_t            ftp_port_rsp[] = {0x32, 0x30, 0x30, 0x20, 0x50, 0x4f, 0x52, 0x54,
                                       0x20, 0x63, 0x6f, 0x6d, 0x6d, 0x61, 0x6e, 0x64,
                                       0x20, 0x73, 0x75, 0x63, 0x63, 0x65, 0x73, 0x73,
                                       0x66, 0x75, 0x6c, 0x2e, 0x20, 0x43, 0x6f, 0x6e,
                                       0x73, 0x69, 0x64, 0x65, 0x72, 0x20, 0x75, 0x73,
                                       0x69, 0x6e, 0x67, 0x20, 0x50, 0x41, 0x53, 0x56,
                                       0x2e, 0x0d, 0x0a};

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(FTP_PORT, 2000);
    tcp.flags(Tins::TCP::SYN);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph2, client_eph2, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.mcast_en);
    EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.mcast_en);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.session()->rflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_FTP);
    hal::session_t *session = ctx_.session();

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(2000, FTP_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph2, server_eph2, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //TCP ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(FTP_PORT, 2000);
    tcp.flags(Tins::TCP::ACK);
    tcp.seq(1);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph2, client_eph2, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //FTP PORT Command
    tcp = Tins::TCP(FTP_PORT, 2000) /
          Tins::RawPDU(ftp_port, sizeof(ftp_port));
    tcp.seq(1);
    //tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    //tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    //tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph2, client_eph2, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //FTP PORT Response
    tcp = Tins::TCP(2000, FTP_PORT) /
         Tins::RawPDU(ftp_port_rsp, sizeof(ftp_port_rsp));
    tcp.seq(1);
    //tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    //tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    //tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph2, server_eph2, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    CHECK_DENY_TCP(client_eph, server_eph2, 37075, FTP_DATA_PORT, "c:20 -> s:37075");
    CHECK_DENY_TCP(server_eph2, client_eph,  37075, 2000, "c:2000 -> s:37075");
    CHECK_DENY_TCP(client_eph, server_eph2, 37075, 2001, "c:2001 -> s:37075");

}

TEST_F(ftp_test, app_sess_force_delete) {
    SessionGetResponseMsg  resp;
    hal::session_t        *session = NULL;
    hal_ret_t              ret;
    hal_handle_t           sess_hdl = 0;

    ret = hal::session_get_all(&resp);
    EXPECT_EQ(ret, HAL_RET_OK);
    for (int idx=0; idx<resp.response_size(); idx++) {
        SessionGetResponse rsp = resp.response(idx);
        if (rsp.status().has_ftp_info() && 
            rsp.status().ftp_info().iscontrol()) {
            sess_hdl = rsp.status().session_handle();
        }
    }
    
    // Invoke delete callback
    session = hal::find_session_by_handle(sess_hdl);
    ASSERT_TRUE(session != NULL);
    ret = session_delete(session, true);
    ASSERT_EQ(ret, HAL_RET_OK);
}

TEST_F(ftp_test, ssh_ftp_alg_negative)
{
    SessionGetRequest       req;
    SessionGetResponseMsg   rsp;
    hal_ret_t          ret;
    uint8_t            pkt1[] = {0xFE, 0xC0, 0xBC, 0xC6, 0x51, 0x3E, 0x8B, 0xAC, 0x1F, 0xF8, 
                                 0xAE, 0x45, 0xFF, 0x41, 0x30, 0x5A, 0xBC, 0x62, 0x5D, 0x01, 
                                 0x96, 0x6A, 0x8D, 0x01, 0x42, 0x8A, 0x6E, 0xAC, 0xD1, 0xEF, 
                                 0x2B, 0x89, 0x49, 0x7E, 0x35, 0x5D, 0xD0, 0xB7, 0x2E, 0x4D, 
                                 0xDB, 0x00, 0x62, 0xD7, 0xAF, 0x68, 0x91, 0x83};
    uint8_t            pkt2[] = {0x3E, 0xC2, 0x7A, 0xEB, 0x46, 0x3B, 0x7A, 0xDD, 0x28, 0xB6, 
                                 0x85, 0x83, 0x5F, 0x61, 0x23, 0xE2, 0x5C, 0xD6, 0x5A, 0xD3, 
                                 0xE1, 0xE4, 0xD7, 0xCF, 0x77, 0x19, 0x98, 0x51, 0xD9, 0xAF, 
                                 0x49, 0x43, 0x2B, 0xE4, 0xD2, 0x9E, 0x0C, 0xFB, 0x18, 0x0A, 
                                 0xD5, 0xD0, 0x3C, 0x63, 0x66, 0x57, 0xE7, 0x5C};
    uint8_t            pkt3[] = {0x9E, 0x8F, 0x04, 0x43, 0x24, 0x8C, 0x55, 0xF6, 0x6B, 0x80, 
                                 0x6E, 0x31, 0xE6, 0xA4, 0x10, 0x38, 0x4B, 0xD8, 0x61, 0x74, 
                                 0xF3, 0x40, 0xED, 0x56, 0xCB, 0xB3, 0x77, 0xB3, 0xDF, 0x4D, 
                                 0x67, 0x03, 0xC9, 0x78, 0xF8, 0x6C, 0x65, 0xF3, 0x5A, 0xB8, 
                                 0x47, 0xB6, 0xEA, 0xBB, 0x9E, 0x1A, 0x27, 0x27};
    uint8_t            pkt4[] = {0xDA, 0x01, 0x1B, 0xC4, 0x3D, 0x4C, 0x9F, 0x5D, 0x7D, 0x7F, 
                                 0xD9, 0x75, 0xE1, 0xF0, 0xD3, 0xE3, 0x26, 0x8F, 0xBC, 0x44, 
                                 0xA6, 0x15, 0x7E, 0xF1, 0xCF, 0xB3, 0x89, 0x3E, 0xD4, 0x90, 
                                 0xFC, 0xE7, 0xEF, 0x5E, 0xFE, 0x43, 0x35, 0x8A, 0xAE, 0x22, 
                                 0x9C, 0xE5, 0x33, 0x8A, 0xFE, 0xD8, 0x65, 0x12};
    uint8_t            pkt5[] = {0x27, 0x0C, 0x94, 0x57, 0xFE, 0xB6, 0xE4, 0x6B, 0x89, 0x89, 
                                 0xC0, 0x8B, 0x3B, 0xFC, 0xC5, 0xFF, 0xA1, 0x6E, 0x30, 0xA5, 
                                 0x22, 0x55, 0x4C, 0x36, 0xB3, 0xFB, 0x74, 0x8F, 0xA8, 0xDD, 
                                 0x0A, 0x6D, 0xD0, 0x4C, 0x62, 0xD9, 0x89, 0xC6, 0x88, 0x94, 
                                 0x84, 0x4D, 0xB6, 0xBF, 0xAD, 0xF9, 0x28, 0x20};
    uint8_t            pkt6[] = {0x33, 0x19, 0x59, 0xD0, 0xFC, 0x3C, 0x65, 0x1E, 0xEE, 0xD8, 
                                 0x63, 0x74, 0x4B, 0x9B, 0x76, 0xB8, 0x61, 0x58, 0x68, 0x27, 
                                 0x01, 0x75, 0x5E, 0xDC, 0x5A, 0x20, 0xE5, 0x32, 0x07, 0xE4, 
                                 0x83, 0x79, 0xDC, 0xDF, 0xEC, 0x91, 0x99, 0x65, 0x74, 0x4D, 
                                 0x2B, 0xBA, 0xC6, 0xDC, 0x71, 0xC8, 0xEB, 0xFE};
    uint8_t            pkt7[] = {0x39, 0xEF, 0xB7, 0x7C, 0xD4, 0xCB, 0x9C, 0x04, 0x6A, 0x4F, 
                                 0x1C, 0x1D, 0xF9, 0xCF, 0x04, 0x0B, 0x05, 0x65, 0xF1, 0x8C, 
                                 0x1E, 0xA9, 0x77, 0x58, 0x35, 0x75, 0xDA, 0x33, 0xD2, 0x73, 
                                 0x19, 0xF0, 0xBB, 0x08, 0xF6, 0x18, 0x73, 0x37, 0x02, 0xAC, 
                                 0x8A, 0x8E, 0xF4, 0x01, 0x2F, 0x66, 0x05, 0xB4};
    uint8_t            pkt8[] = {0x58, 0x7A, 0x76, 0xCB, 0xBB, 0x24, 0xA1, 0x2F, 0x8C, 0xDC, 
                                 0x58, 0xF8, 0x7D, 0x96, 0x72, 0xC4, 0x0E, 0x59, 0x58, 0x4C,
                                 0x4B, 0x56, 0x1C, 0xC1, 0x99, 0x85, 0x6A, 0x00, 0xC9, 0x3F,
                                 0x64, 0x9B, 0x26, 0xB7, 0xDF, 0x5E, 0x2D, 0xB7, 0x1A, 0xCE,
                                 0xBB, 0x3D, 0xF5, 0xEC, 0x35, 0x63, 0x71, 0x50};

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(22, 44418);
    tcp.flags(Tins::TCP::SYN);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    tcp.seq(1119625129);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.mcast_en);
    EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.mcast_en);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.session()->rflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_FTP);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    hal::session_t *session = ctx_.session();
    EXPECT_NE(session->sfw_rule_id, 0);
    EXPECT_EQ(session->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(44418, 22);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //TCP ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(22, 44418);
    tcp.flags(Tins::TCP::ACK);
    tcp.seq(1119625130);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    tcp = Tins::TCP(22, 44418) /
          Tins::RawPDU(pkt1, sizeof(pkt1));
    tcp.seq(1119626062);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    tcp = Tins::TCP(22, 44418) /
          Tins::RawPDU(pkt2, sizeof(pkt2));
    tcp.seq(1119626110);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    tcp = Tins::TCP(22, 44418) /
          Tins::RawPDU(pkt3, sizeof(pkt3));
    tcp.seq(1119626254);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    tcp = Tins::TCP(22, 44418) /
          Tins::RawPDU(pkt4, sizeof(pkt4));
    tcp.seq(1119626446);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    tcp = Tins::TCP(22, 44418) /
          Tins::RawPDU(pkt5, sizeof(pkt5));
    tcp.seq(1119626542);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
   
    tcp = Tins::TCP(22, 44418) /
          Tins::RawPDU(pkt6, sizeof(pkt6));
    tcp.seq(1119626638);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    tcp = Tins::TCP(22, 44418) /
          Tins::RawPDU(pkt7, sizeof(pkt7));
    tcp.seq(1119626830);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    tcp = Tins::TCP(22, 44418) /
          Tins::RawPDU(pkt8, sizeof(pkt8));
    tcp.seq(1119626974);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
}
