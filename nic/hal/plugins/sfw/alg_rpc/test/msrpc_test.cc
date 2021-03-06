#include "rpc_test.hpp"
#include <gtest/gtest.h>
#include "nic/hal/plugins/sfw/alg_rpc/core.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include <tins/tins.h>

using namespace hal::plugins::alg_rpc;
using namespace session;
using namespace fte;

//extern hal_handle_t rpc_test::client_eph, rpc_test::server_eph;

TEST_F(rpc_test, msrpc_session)
{
    SessionGetRequest       req;
    SessionGetResponseMsg   rsp;
    hal_ret_t          ret;
    uint8_t bind_req[] = {0x05, 0x00, 0x0b, 0x03, 0x10, 0x00, 0x00, 0x00,
                          0x48, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                          0xd0, 0x16, 0xd0, 0x16, 0x00, 0x00, 0x00, 0x00,
                          0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
                          0x08, 0x83, 0xaf, 0xe1, 0x1f, 0x5d, 0xc9, 0x11,
                          0x91, 0xa4, 0x08, 0x00, 0x2b, 0x14, 0xa0, 0xfa,
                          0x03, 0x00, 0x00, 0x00, 0x04, 0x5d, 0x88, 0x8a,
                          0xeb, 0x1c, 0xc9, 0x11, 0x9f, 0xe8, 0x08, 0x00,
                          0x2b, 0x10, 0x48, 0x60, 0x02, 0x00, 0x00, 0x00};

    uint8_t bind_rsp[] = {0x05, 0x00, 0x0c, 0x03, 0x10, 0x00, 0x00, 0x00,
                          0x3c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                          0xd0, 0x16, 0xd0, 0x16, 0x95, 0xb7, 0x00, 0x00,
                          0x04, 0x00, 0x31, 0x33, 0x35, 0x00, 0x9d, 0x4d,
                          0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x04, 0x5d, 0x88, 0x8a, 0xeb, 0x1c, 0xc9, 0x11,
                          0x9f, 0xe8, 0x08, 0x00, 0x2b, 0x10, 0x48, 0x60,
                          0x02, 0x00, 0x00, 0x00};
    
    uint8_t epm_req[] = {0x05, 0x00, 0x00, 0x03, 0x10, 0x00, 0x00, 0x00,
                         0x9c, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                         0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
                         0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                         0x4b, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x00, 0x00,
                         0x05, 0x00, 0x13, 0x00, 0x0d, 0x78, 0x57, 0x34,
                         0x12, 0x34, 0x12, 0xcd, 0xab, 0xef, 0x00, 0x01,
                         0x23, 0x45, 0x67, 0x89, 0xac, 0x01, 0x00, 0x02,
                         0x00, 0x00, 0x00, 0x13, 0x00, 0x0d, 0x04, 0x5d,
                         0x88, 0x8a, 0xeb, 0x1c, 0xc9, 0x11, 0x9f, 0xe8,
                         0x08, 0x00, 0x2b, 0x10, 0x48, 0x60, 0x02, 0x00,
                         0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0b, 0x02,
                         0x00, 0x00, 0x00, 0x01, 0x00, 0x07, 0x02, 0x00,
                         0x00, 0x00, 0x01, 0x00, 0x09, 0x04, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x01, 0x00, 0x00, 0x00};

    uint8_t epm_rsp[] = {0x05, 0x00, 0x02, 0x03, 0x10, 0x00, 0x00, 0x00,
                         0x98, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                         0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x33, 0xff, 0xf1, 0x31,
                         0x1f, 0xe4, 0x64, 0x4a, 0x94, 0xd0, 0x75, 0x99,
                         0xe8, 0x82, 0x4a, 0x22, 0x01, 0x00, 0x00, 0x00,
                         0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
                         0x4b, 0x00, 0x00, 0x00, 0x4b, 0x00, 0x00, 0x00,
                         0x05, 0x00, 0x13, 0x00, 0x0d, 0x78, 0x57, 0x34,
                         0x12, 0x34, 0x12, 0xcd, 0xab, 0xef, 0x00, 0x01,
                         0x23, 0x45, 0x67, 0x89, 0xac, 0x01, 0x00, 0x02,
                         0x00, 0x00, 0x00, 0x13, 0x00, 0x0d, 0x04, 0x5d,
                         0x88, 0x8a, 0xeb, 0x1c, 0xc9, 0x11, 0x9f, 0xe8,
                         0x08, 0x00, 0x2b, 0x10, 0x48, 0x60, 0x02, 0x00,
                         0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0b, 0x02,
                         0x00, 0x00, 0x00, 0x01, 0x00, 0x07, 0x02, 0x00,
                         0xc0, 0x02, 0x01, 0x00, 0x09, 0x04, 0x00, 0x0a,
                         0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(MSRPC_PORT, 1000);
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
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_MSFT_RPC);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    hal::session_t *session = ctx_.session();
    EXPECT_NE(session->sfw_rule_id, 0);
    EXPECT_EQ(session->skip_sfw_reval, 0);
    EXPECT_EQ(session->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);

    // TCP SYN re-transmit on ALG_CFLOW_LIFQ
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);


    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(1000, MSRPC_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(MSRPC_PORT, 1000);
    tcp.flags(Tins::TCP::ACK);
    tcp.seq(1);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);  

    //MSRPC BINDREQ 
    tcp = Tins::TCP(MSRPC_PORT, 1000) /
          Tins::RawPDU(bind_req, sizeof(bind_req));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //MSRPC BINDRSP
    tcp = Tins::TCP(1000, MSRPC_PORT) /
         Tins::RawPDU(bind_rsp, sizeof(bind_rsp));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //MSRPC EPM REQ
    tcp = Tins::TCP(MSRPC_PORT, 1000) /
          Tins::RawPDU(epm_req, sizeof(epm_req));
    tcp.seq(73);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop()); 
    EXPECT_EQ(ctx_.session(), session);

    // MSRPC EPM RSP
    tcp = Tins::TCP(1000, MSRPC_PORT) /
         Tins::RawPDU(epm_rsp, sizeof(epm_rsp));
    tcp.seq(61);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    CHECK_ALLOW_TCP(server_eph, client_eph, 49154, 49153, "c:49153 -> s:49154");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_MSFT_RPC);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    EXPECT_EQ(ctx_.session()->skip_sfw_reval, 1);
    EXPECT_EQ(ctx_.session()->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.session()->sfw_rule_id, 0);
    CHECK_ALLOW_TCP(server_eph, client_eph, 49154, 59374, "c:59374 -> s:49154");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_MSFT_RPC);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    EXPECT_EQ(ctx_.session()->skip_sfw_reval, 1);
    EXPECT_EQ(ctx_.session()->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.session()->sfw_rule_id, 0);
    CHECK_DENY_UDP(server_eph, client_eph, 49154, 49153, "c:49153 -> s:49154");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_DENY);
    CHECK_DENY_UDP(server_eph, client_eph, 49154, 59374, "c:59374 -> s:49154");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_DENY);

    req.set_session_handle(session->hal_handle);
    ret = hal::session_get(req, &rsp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(rsp.response(0).status().alg(), nwsec::APP_SVC_MSFT_RPC);
    EXPECT_TRUE(rsp.response(0).status().has_rpc_info());
    EXPECT_EQ(rsp.response(0).status().rpc_info().num_exp_flows(), 1);
    EXPECT_EQ(rsp.response(0).status().rpc_info().num_data_sess(), 2);
}

TEST_F(rpc_test, msrpc_session_nd464bit)
{
    SessionGetRequest       req;
    SessionGetResponseMsg   rsp;
    hal_ret_t               ret;
    uint8_t bind_req[] = {0x05, 0x00, 0x0b, 0x03, 0x10, 0x00, 0x00, 0x00,
                          0xa0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                          0xd0, 0x16, 0xd0, 0x16, 0x00, 0x00, 0x00, 0x00,
                          0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
                          0x08, 0x83, 0xaf, 0xe1, 0x1f, 0x5d, 0xc9, 0x11,
                          0x91, 0xa4, 0x08, 0x00, 0x2b, 0x14, 0xa0, 0xfa,
                          0x03, 0x00, 0x00, 0x00, 0x04, 0x5d, 0x88, 0x8a,
                          0xeb, 0x1c, 0xc9, 0x11, 0x9f, 0xe8, 0x08, 0x00,
                          0x2b, 0x10, 0x48, 0x60, 0x02, 0x00, 0x00, 0x00,
                          0x01, 0x00, 0x01, 0x00, 0x08, 0x83, 0xaf, 0xe1,
                          0x1f, 0x5d, 0xc9, 0x11, 0x91, 0xa4, 0x08, 0x00,
                          0x2b, 0x14, 0xa0, 0xfa, 0x03, 0x00, 0x00, 0x00,
                          0x33, 0x05, 0x71, 0x71, 0xba, 0xbe, 0x37, 0x49,
                          0x83, 0x19, 0xb5, 0xdb, 0xef, 0x9c, 0xcc, 0x36,
                          0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00,
                          0x08, 0x83, 0xaf, 0xe1, 0x1f, 0x5d, 0xc9, 0x11,
                          0x91, 0xa4, 0x08, 0x00, 0x2b, 0x14, 0xa0, 0xfa,
                          0x03, 0x00, 0x00, 0x00, 0x2c, 0x1c, 0xb7, 0x6c,
                          0x12, 0x98, 0x40, 0x45, 0x03, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
     uint8_t bind_rsp[]= {0x05, 0x00, 0x0c, 0x03, 0x10, 0x00, 0x00, 0x00,
                          0x6c, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                          0xd0, 0x16, 0xd0, 0x16, 0x37, 0x1f, 0x00, 0x00,
                          0x04, 0x00, 0x31, 0x33, 0x35, 0x00, 0x38, 0x00,
                          0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x33, 0x05, 0x71, 0x71, 0xba, 0xbe, 0x37, 0x49,
                          0x83, 0x19, 0xb5, 0xdb, 0xef, 0x9c, 0xcc, 0x36,
                          0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00};
    uint8_t epm_req[] = {0x05, 0x00, 0x00, 0x03, 0x10, 0x00, 0x00, 0x00,
                         0xa8, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                         0x90, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00,
                         0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x4b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x4b, 0x00, 0x00, 0x00, 0x05, 0x00, 0x13, 0x00,
                         0x0d, 0x35, 0x42, 0x51, 0xe3, 0x06, 0x4b, 0xd1,
                         0x11, 0xab, 0x04, 0x00, 0xc0, 0x4f, 0xc2, 0xdc,
                         0xd2, 0x04, 0x00, 0x02, 0x00, 0x00, 0x00, 0x13,
                         0x00, 0x0d, 0x04, 0x5d, 0x88, 0x8a, 0xeb, 0x1c,
                         0xc9, 0x11, 0x9f, 0xe8, 0x08, 0x00, 0x2b, 0x10,
                         0x48, 0x60, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00,
                         0x01, 0x00, 0x0b, 0x02, 0x00, 0x00, 0x00, 0x01,
                         0x00, 0x07, 0x02, 0x00, 0x00, 0x87, 0x01, 0x00,
                         0x09, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00};

    uint8_t epm_rsp[] = {0x05, 0x00, 0x02, 0x03, 0x10, 0x00, 0x00, 0x00,
                         0xac, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                         0x94, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                         0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x4b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                         0x4b, 0x00, 0x00, 0x00, 0x05, 0x00, 0x13, 0x00,
                         0x0d, 0x35, 0x42, 0x51, 0xe3, 0x06, 0x4b, 0xd1,
                         0x11, 0xab, 0x04, 0x00, 0xc0, 0x4f, 0xc2, 0xdc,
                         0xd2, 0x04, 0x00, 0x02, 0x00, 0x00, 0x00, 0x13,
                         0x00, 0x0d, 0x04, 0x5d, 0x88, 0x8a, 0xeb, 0x1c,
                         0xc9, 0x11, 0x9f, 0xe8, 0x08, 0x00, 0x2b, 0x10,
                         0x48, 0x60, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00,
                         0x01, 0x00, 0x0b, 0x02, 0x00, 0x00, 0x00, 0x01,
                         0x00, 0x07, 0x02, 0x00, 0xc2, 0x03, 0x01, 0x00,
                         0x09, 0x04, 0x00, 0x0a, 0x00, 0x00, 0x02, 0x00,
                         0x00, 0x00, 0x00, 0x00};
    
    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(MSRPC_PORT, 1001);
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
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_MSFT_RPC);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    hal::session_t *session = ctx_.session();
    EXPECT_NE(session->sfw_rule_id, 0);
    EXPECT_EQ(session->skip_sfw_reval, 0);
    EXPECT_EQ(session->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);

    // TCP SYN re-transmit on ALG_CFLOW_LIFQ
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(1001, MSRPC_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(MSRPC_PORT, 1001);
    tcp.flags(Tins::TCP::ACK);
    tcp.seq(1);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //MSRPC BINDREQ
    tcp = Tins::TCP(MSRPC_PORT, 1001) /
          Tins::RawPDU(bind_req, sizeof(bind_req));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //MSRPC BINDRSP
    tcp = Tins::TCP(1001, MSRPC_PORT) /
         Tins::RawPDU(bind_rsp, sizeof(bind_rsp));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //MSRPC EPM REQ
    tcp = Tins::TCP(MSRPC_PORT, 1001) /
          Tins::RawPDU(epm_req, sizeof(epm_req));
    tcp.seq(161);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    // MSRPC EPM RSP
    tcp = Tins::TCP(1001, MSRPC_PORT) /
         Tins::RawPDU(epm_rsp, sizeof(epm_rsp));
    tcp.seq(109);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    CHECK_ALLOW_TCP(server_eph, client_eph, 49667, 49666, "c:49666 -> s:49667");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_MSFT_RPC);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    EXPECT_EQ(ctx_.session()->skip_sfw_reval, 1);
    EXPECT_EQ(ctx_.session()->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.session()->sfw_rule_id, 0);
    EXPECT_FALSE(ctx_.drop());
    CHECK_ALLOW_TCP(server_eph, client_eph, 49667, 59374, "c:59374 -> s:49667");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_MSFT_RPC);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    EXPECT_EQ(ctx_.session()->skip_sfw_reval, 1);
    EXPECT_EQ(ctx_.session()->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.session()->sfw_rule_id, 0);
    CHECK_DENY_UDP(server_eph, client_eph, 49667, 49153, "c:49153 -> s:49667");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_DENY);
    CHECK_DENY_UDP(server_eph, client_eph, 49667, 59374, "c:59374 -> s:49667");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_DENY);

    req.set_session_handle(session->hal_handle);
    ret = hal::session_get(req, &rsp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(rsp.response(0).status().alg(), nwsec::APP_SVC_MSFT_RPC);
    EXPECT_TRUE(rsp.response(0).status().has_rpc_info());
    EXPECT_EQ(rsp.response(0).status().rpc_info().num_exp_flows(), 1);
    EXPECT_EQ(rsp.response(0).status().rpc_info().num_data_sess(), 2); 
}

TEST_F(rpc_test, msrpc_session_nd464bit_2)
{
    SessionGetRequest       req;
    SessionGetResponseMsg   rsp;
    hal_ret_t               ret;
    uint8_t bind_req[] =        {0x05, 0x00, 0x0b, 0x03, 0x10, 0x00, 0x00, 0x00, 
                                 0xa0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                                 0xd0, 0x16, 0xd0, 0x16, 0x00, 0x00, 0x00, 0x00, 
                                 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
                                 0x08, 0x83, 0xaf, 0xe1, 0x1f, 0x5d, 0xc9, 0x11, 
                                 0x91, 0xa4, 0x08, 0x00, 0x2b, 0x14, 0xa0, 0xfa,
                                 0x03, 0x00, 0x00, 0x00, 0x04, 0x5d, 0x88, 0x8a, 
                                 0xeb, 0x1c, 0xc9, 0x11, 0x9f, 0xe8, 0x08, 0x00,
                                 0x2b, 0x10, 0x48, 0x60, 0x02, 0x00, 0x00, 0x00, 
                                 0x01, 0x00, 0x01, 0x00, 0x08, 0x83, 0xaf, 0xe1,
                                 0x1f, 0x5d, 0xc9, 0x11, 0x91, 0xa4, 0x08, 0x00, 
                                 0x2b, 0x14, 0xa0, 0xfa, 0x03, 0x00, 0x00, 0x00,
                                 0x33, 0x05, 0x71, 0x71, 0xba, 0xbe, 0x37, 0x49, 
                                 0x83, 0x19, 0xb5, 0xdb, 0xef, 0x9c, 0xcc, 0x36,
                                 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 
                                 0x08, 0x83, 0xaf, 0xe1, 0x1f, 0x5d, 0xc9, 0x11,
                                 0x91, 0xa4, 0x08, 0x00, 0x2b, 0x14, 0xa0, 0xfa, 
                                 0x03, 0x00, 0x00, 0x00, 0x2c, 0x1c, 0xb7, 0x6c,
                                 0x12, 0x98, 0x40, 0x45, 0x03, 0x00, 0x00, 0x00, 
                                 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    uint8_t bind_rsp[] =        {0x05, 0x00, 0x0c, 0x03, 0x10, 0x00, 0x00, 0x00, 
                                 0x6c, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                                 0xd0, 0x16, 0xd0, 0x16, 0xcb, 0x10, 0x00, 0x00, 
                                 0x04, 0x00, 0x31, 0x33, 0x35, 0x00, 0x00, 0x00,
                                 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x33, 0x05, 0x71, 0x71, 0xba, 0xbe, 0x37, 0x49, 
                                 0x83, 0x19, 0xb5, 0xdb, 0xef, 0x9c, 0xcc, 0x36,
                                 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                 0x00, 0x00, 0x00, 0x00};
    uint8_t epm_req[] =    {0x05, 0x00, 0x00, 0x03, 0x10, 0x00, 0x00, 0x00, 
                            0xa8, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                            0x90, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 
                            0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x74, 0xc0, 0xd8, 0xcc, 0xe5, 0xd0, 0x40, 0x4a, 
                            0x92, 0xb4, 0xd0, 0x74, 0xfa, 0xa6, 0xba, 0x28,
                            0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                            0x4b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x4b, 0x00, 0x00, 0x00, 0x05, 0x00, 0x13, 0x00, 
                            0x0d, 0x74, 0xc0, 0xd8, 0xcc, 0xe5, 0xd0, 0x40,
                            0x4a, 0x92, 0xb4, 0xd0, 0x74, 0xfa, 0xa6, 0xba, 
                            0x28, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x13,
                            0x00, 0x0d, 0x04, 0x5d, 0x88, 0x8a, 0xeb, 0x1c, 
                            0xc9, 0x11, 0x9f, 0xe8, 0x08, 0x00, 0x2b, 0x10,
                            0x48, 0x60, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 
                            0x01, 0x00, 0x0b, 0x02, 0x00, 0x00, 0x00, 0x01,
                            0x00, 0x07, 0x02, 0x00, 0x00, 0x87, 0x01, 0x00, 
                            0x09, 0x04, 0x00, 0x0a, 0x00, 0x00, 0x02, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00};
    uint8_t epm_rsp[] =     {0x05, 0x00, 0x02, 0x03, 0x10, 0x00, 0x00, 0x00, 
                             0xac, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
                             0x94, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                             0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
                             0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                             0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x4b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x4b, 0x00, 0x00, 0x00, 0x05, 0x00, 0x13, 0x00,
                             0x0d, 0x74, 0xc0, 0xd8, 0xcc, 0xe5, 0xd0, 0x40, 
                             0x4a, 0x92, 0xb4, 0xd0, 0x74, 0xfa, 0xa6, 0xba,
                             0x28, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x13, 
                             0x00, 0x0d, 0x04, 0x5d, 0x88, 0x8a, 0xeb, 0x1c,
                             0xc9, 0x11, 0x9f, 0xe8, 0x08, 0x00, 0x2b, 0x10, 
                             0x48, 0x60, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00,
                             0x01, 0x00, 0x0b, 0x02, 0x00, 0x00, 0x00, 0x01, 
                             0x00, 0x07, 0x02, 0x00, 0xc0, 0x96, 0x01, 0x00,
                             0x09, 0x04, 0x00, 0x0a, 0x00, 0x00, 0x02, 0x00, 
                             0x00, 0x00, 0x00, 0x00};

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(MSRPC_PORT, 1002);
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
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_MSFT_RPC);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    hal::session_t *session = ctx_.session();
    EXPECT_NE(session->sfw_rule_id, 0);
    EXPECT_EQ(session->skip_sfw_reval, 0);
    EXPECT_EQ(session->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.session()->idle_timeout, 30);

    // TCP SYN re-transmit on ALG_CFLOW_LIFQ
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(1002, MSRPC_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(MSRPC_PORT, 1002);
    tcp.flags(Tins::TCP::ACK);
    tcp.seq(1);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp, true);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //MSRPC BINDREQ
    tcp = Tins::TCP(MSRPC_PORT, 1002) /
          Tins::RawPDU(bind_req, sizeof(bind_req));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //MSRPC BINDRSP
    tcp = Tins::TCP(1002, MSRPC_PORT) /
         Tins::RawPDU(bind_rsp, sizeof(bind_rsp));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //MSRPC EPM REQ
    tcp = Tins::TCP(MSRPC_PORT, 1002) /
          Tins::RawPDU(epm_req, sizeof(epm_req));
    tcp.seq(161);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    // MSRPC EPM RSP
    tcp = Tins::TCP(1002, MSRPC_PORT) /
         Tins::RawPDU(epm_rsp, sizeof(epm_rsp));
    tcp.seq(109);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    CHECK_ALLOW_TCP(server_eph, client_eph, 49302, 49301, "c:49301 -> s:49302");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_MSFT_RPC);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    EXPECT_EQ(ctx_.session()->skip_sfw_reval, 1);
    EXPECT_EQ(ctx_.session()->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.session()->sfw_rule_id, 0);
    EXPECT_EQ(ctx_.session()->idle_timeout, 200);
    EXPECT_FALSE(ctx_.drop());    
    CHECK_DENY_UDP(server_eph, client_eph, 49302, 49301, "c:49301 -> s:49302");
}

TEST_F(rpc_test, msrpc_exp_flow_timeout) {
    app_session_t   *app_sess = NULL;
    l4_alg_status_t *exp_flow1 = NULL, *exp_flow2 = NULL, *l4_sess = NULL;
    hal::flow_key_t  app_sess_key, exp_flow_key;

    app_sess_key.proto = (types::IPProtocol)IP_PROTO_TCP;
    app_sess_key.sip.v4_addr = 0x14000001;
    app_sess_key.dip.v4_addr = 0x14000002;
    app_sess_key.sport = 12345;
    app_sess_key.dport = 135;

    g_rpc_state->alloc_and_init_app_sess(app_sess_key, &app_sess);

    g_rpc_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
    l4_sess->isCtrl = true;
    ASSERT_EQ(dllist_count(&app_sess->l4_sess_lhead), 1);

    exp_flow_key = app_sess_key;
    exp_flow_key.dport = 22345;
    g_rpc_state->alloc_and_insert_exp_flow(app_sess, exp_flow_key,
                                         &exp_flow1, true, 2);
    exp_flow_key.dport = 22346;
    g_rpc_state->alloc_and_insert_exp_flow(app_sess, exp_flow_key,
                                         &exp_flow2, true, 5);
    exp_flow2->entry.ref_count.count++;
    sleep(15);
    ASSERT_EQ(exp_flow2->entry.deleting, true);
    exp_flow2->entry.ref_count.count--;
    sleep(15);
    ASSERT_EQ(dllist_count(&app_sess->exp_flow_lhead), 0);
}

TEST_F(rpc_test, msrpc_exp_flow_refcount) {
    app_session_t   *app_sess = NULL;
    l4_alg_status_t *exp_flow1 = NULL, *exp_flow2 = NULL, *l4_sess = NULL;
    hal::flow_key_t  app_sess_key, exp_flow_key;

    app_sess_key.proto = (types::IPProtocol)IP_PROTO_TCP;
    app_sess_key.sip.v4_addr = 0x14000003;
    app_sess_key.dip.v4_addr = 0x14000004;
    app_sess_key.sport = 12346;
    app_sess_key.dport = 135;

    g_rpc_state->alloc_and_init_app_sess(app_sess_key, &app_sess);

    g_rpc_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
    l4_sess->isCtrl = true;
    ASSERT_EQ(dllist_count(&app_sess->l4_sess_lhead), 1);

    exp_flow_key = app_sess_key;
    exp_flow_key.dport = 33345;
    g_rpc_state->alloc_and_insert_exp_flow(app_sess, exp_flow_key,
                                         &exp_flow1, false, 0, true);
    EXPECT_EQ(exp_flow1->entry.ref_count.count, 1);
    exp_flow_key.dport = 33345;
    g_rpc_state->alloc_and_insert_exp_flow(app_sess, exp_flow_key,
                                         &exp_flow2, false, 0, true);
    EXPECT_EQ(exp_flow1, exp_flow2);
    EXPECT_EQ(exp_flow2->entry.ref_count.count, 1);
    EXPECT_EQ(exp_flow1->entry.ref_count.count, 1);
}

TEST_F(rpc_test, msrpc_app_sess_force_delete) {
    SessionGetResponseMsg  resp;
    hal::session_t        *session = NULL;
    hal_ret_t              ret;
    hal_handle_t           sess_hdl = 0;

    ret = hal::session_get_all(&resp);
    EXPECT_EQ(ret, HAL_RET_OK);
    for (int idx=0; idx<resp.response_size(); idx++) {
        SessionGetResponse rsp = resp.response(idx);
        if (rsp.status().has_rpc_info() && 
            rsp.status().rpc_info().iscontrol()) {
            sess_hdl = rsp.status().session_handle();
        }
    }
    
    // Invoke delete callback
    session = hal::find_session_by_handle(sess_hdl);
    ASSERT_TRUE(session != NULL);
    ret = session_delete(session, true);
    ASSERT_EQ(ret, HAL_RET_OK);
}
