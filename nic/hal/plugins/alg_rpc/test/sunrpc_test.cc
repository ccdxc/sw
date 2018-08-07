#include "rpc_test.hpp"
#include <gtest/gtest.h>
#include "nic/hal/plugins/alg_rpc/core.hpp"
#include "nic/p4/iris/include/defines.h"

#include <tins/tins.h>

using namespace hal::plugins::alg_rpc;
using namespace session;
using namespace fte;

hal_handle_t rpc_test::client_eph, rpc_test::server_eph;

TEST_F(rpc_test, sunrpc_session)
{
    SessionGetRequest  req;
    SessionGetResponse rsp;
    hal_ret_t          ret;
    uint8_t            dump_call[] = {0x80, 0x00, 0x00, 0x28, 0xd0, 0x72, 0x28, 0x07, 
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
                                      0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x04, 
                                      0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00};

    uint8_t            dump_rsp[] = {\
                         0x80, 0x00, 0x03, 0x68, 0xd0, 0x72, 0x28, 0x07, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
                         0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x74, 0x63, 0x70, 0x36, 
                         0x00, 0x00, 0x00, 0x08, 0x3a, 0x3a, 0x2e, 0x30, 0x2e, 0x31, 0x31, 0x31, 0x00, 0x00, 0x00, 0x09, 
                         0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                         0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x74, 0x63, 0x70, 0x36, 
                         0x00, 0x00, 0x00, 0x08, 0x3a, 0x3a, 0x2e, 0x30, 0x2e, 0x31, 0x31, 0x31, 0x00, 0x00, 0x00, 0x09, 
                         0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                         0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x75, 0x64, 0x70, 0x36, 
                         0x00, 0x00, 0x00, 0x08, 0x3a, 0x3a, 0x2e, 0x30, 0x2e, 0x31, 0x31, 0x31, 0x00, 0x00, 0x00, 0x09, 
                         0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                         0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x75, 0x64, 0x70, 0x36, 
                         0x00, 0x00, 0x00, 0x08, 0x3a, 0x3a, 0x2e, 0x30, 0x2e, 0x31, 0x31, 0x31, 0x00, 0x00, 0x00, 0x09, 
                         0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                         0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x03, 0x74, 0x63, 0x70, 0x00, 
                         0x00, 0x00, 0x00, 0x0d, 0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x31, 
                         0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 
                         0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x03, 
                         0x00, 0x00, 0x00, 0x03, 0x74, 0x63, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x30, 0x2e, 0x30, 0x2e, 
                         0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x31, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 
                         0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                         0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x74, 0x63, 0x70, 0x00, 
                         0x00, 0x00, 0x00, 0x0d, 0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x31, 
                         0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 
                         0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x04, 
                         0x00, 0x00, 0x00, 0x03, 0x75, 0x64, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x30, 0x2e, 0x30, 0x2e, 
                         0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x31, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 
                         0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                         0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x03, 0x75, 0x64, 0x70, 0x00, 
                         0x00, 0x00, 0x00, 0x0d, 0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x31, 
                         0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 
                         0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x02, 
                         0x00, 0x00, 0x00, 0x03, 0x75, 0x64, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x30, 0x2e, 0x30, 0x2e, 
                         0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x31, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 
                         0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                         0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x05, 0x6c, 0x6f, 0x63, 0x61, 
                         0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x2f, 0x76, 0x61, 0x72, 0x2f, 0x72, 0x75, 0x6e, 
                         0x2f, 0x72, 0x70, 0x63, 0x62, 0x69, 0x6e, 0x64, 0x2e, 0x73, 0x6f, 0x63, 0x6b, 0x00, 0x00, 0x00, 
                         0x00, 0x00, 0x00, 0x09, 0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 0x72, 0x00, 0x00, 0x00, 
                         0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 
                         0x6c, 0x6f, 0x63, 0x61, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x2f, 0x76, 0x61, 0x72, 
                         0x2f, 0x72, 0x75, 0x6e, 0x2f, 0x72, 0x70, 0x63, 0x62, 0x69, 0x6e, 0x64, 0x2e, 0x73, 0x6f, 0x63, 
                         0x6b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 
                         0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x86, 0xb8, 0x00, 0x00, 0x00, 0x01, 
                         0x00, 0x00, 0x00, 0x03, 0x75, 0x64, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x30, 0x2e, 0x30, 0x2e, 
                         0x30, 0x2e, 0x30, 0x2e, 0x31, 0x32, 0x38, 0x2e, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 
                         0x32, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x86, 0xb8, 0x00, 0x00, 0x00, 0x01, 
                         0x00, 0x00, 0x00, 0x03, 0x74, 0x63, 0x70, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x30, 0x2e, 0x30, 0x2e, 
                         0x30, 0x2e, 0x30, 0x2e, 0x32, 0x31, 0x34, 0x2e, 0x31, 0x30, 0x36, 0x00, 0x00, 0x00, 0x00, 0x02, 
                         0x32, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x86, 0xb8, 0x00, 0x00, 0x00, 0x01, 
                         0x00, 0x00, 0x00, 0x04, 0x75, 0x64, 0x70, 0x36, 0x00, 0x00, 0x00, 0x08, 0x3a, 0x3a, 0x2e, 0x31, 
                         0x33, 0x30, 0x2e, 0x34, 0x00, 0x00, 0x00, 0x02, 0x32, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                         0x00, 0x01, 0x86, 0xb8, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x74, 0x63, 0x70, 0x36, 
                         0x00, 0x00, 0x00, 0x09, 0x3a, 0x3a, 0x2e, 0x31, 0x33, 0x37, 0x2e, 0x31, 0x38, 0x00, 0x00, 0x00, 
                         0x00, 0x00, 0x00, 0x02, 0x32, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(SUNRPC_PORT, 5000);
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
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_SUN_RPC);
    hal::session_t *session = ctx_.session();

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(5000, SUNRPC_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    // TCP ACK
    tcp = Tins::TCP(SUNRPC_PORT, 5000);
    tcp.flags(Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //SUNRPC DUMP CALL
    tcp = Tins::TCP(SUNRPC_PORT, 5000) /
          Tins::RawPDU(dump_call, sizeof(dump_call));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //MSRPC BINDRSP
    tcp = Tins::TCP(5000, SUNRPC_PORT) /
         Tins::RawPDU(dump_rsp, sizeof(dump_rsp));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    CHECK_ALLOW_TCP(server_eph, client_eph, 54890, 49153, "c:49153 -> s:54890");
    CHECK_ALLOW_UDP(server_eph, client_eph, 32776, 59374, "c:59374 -> s:32776");
    CHECK_DENY_UDP(server_eph, client_eph, 54890, 49153, "c:49153 -> s:54890");
    CHECK_DENY_TCP(server_eph, client_eph, 32776, 59374, "c:59374 -> s:32776");

    req.set_session_handle(session->hal_handle);
    req.mutable_meta()->set_vrf_id(ctx_.svrf()->vrf_id);
    ret = hal::session_get(req, &rsp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(rsp.status().alg(), nwsec::APP_SVC_SUN_RPC);
    EXPECT_TRUE(rsp.status().has_rpc_info());
    EXPECT_EQ(rsp.status().rpc_info().num_exp_flows(), 14);
    EXPECT_EQ(rsp.status().rpc_info().num_data_sess(), 2);
}

TEST_F(rpc_test, sunrpc_exp_flow_timeout) {
    app_session_t   *app_sess = NULL;
    l4_alg_status_t *exp_flow = NULL;
    hal::flow_key_t  app_sess_key, exp_flow_key;

    app_sess_key.proto = (types::IPProtocol)IP_PROTO_TCP;
    app_sess_key.svrf_id = 1;
    app_sess_key.dvrf_id = 1;
    app_sess_key.dir = 1;
    app_sess_key.sip.v4_addr = 0x16000001;
    app_sess_key.dip.v4_addr = 0x16000002;
    app_sess_key.sport = 12345;
    app_sess_key.dport = 111;

    g_rpc_state->alloc_and_init_app_sess(app_sess_key, &app_sess);

    exp_flow_key = app_sess_key;
    exp_flow_key.svrf_id = 1;
    exp_flow_key.dvrf_id = 1;
    exp_flow_key.flow_type = hal::FLOW_TYPE_V4;
    exp_flow_key.proto = types::IPPROTO_TCP;
    exp_flow_key.dport = 22345;
    g_rpc_state->alloc_and_insert_exp_flow(app_sess, exp_flow_key,
                                         &exp_flow, true, 8, true);
    sleep(10);
    ASSERT_EQ(dllist_count(&app_sess->exp_flow_lhead), 0);

    exp_flow_key.dport = 22346;
    g_rpc_state->alloc_and_insert_exp_flow(app_sess, exp_flow_key,
                                         &exp_flow, true, 5, true);
    exp_flow->entry.ref_count.count++;
    sleep(5);
    ASSERT_EQ(exp_flow->entry.deleting, true);
    exp_flow->entry.ref_count.count--;
    sleep(15);
    ASSERT_EQ(dllist_count(&app_sess->exp_flow_lhead), 0);

    l4_alg_status_t *exp_flow_old = NULL, *exp_flow_new = NULL;
    exp_flow_key.dport = 22347;
    g_rpc_state->alloc_and_insert_exp_flow(app_sess, exp_flow_key,
                                         &exp_flow_old, true, 5, true);
    ASSERT_EQ(exp_flow_old->entry.deleting, false);
    g_rpc_state->alloc_and_insert_exp_flow(app_sess, exp_flow_key,
                                         &exp_flow_new, true, 5, true);
    ASSERT_EQ(exp_flow_old, exp_flow_new);
    ASSERT_EQ(exp_flow_new->entry.deleting, false); 
    sleep(3);
    ASSERT_EQ(dllist_count(&app_sess->exp_flow_lhead), 0);
}

TEST_F(rpc_test, sunrpc_app_sess_force_delete) {
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
