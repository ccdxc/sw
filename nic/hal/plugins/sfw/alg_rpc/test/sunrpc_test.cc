#include "rpc_test.hpp"
#include <gtest/gtest.h>
#include "nic/hal/plugins/sfw/alg_rpc/core.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"

#include <tins/tins.h>

using namespace hal::plugins::alg_rpc;
using namespace session;
using namespace fte;

hal_handle_t rpc_test::client_eph, rpc_test::server_eph;

TEST_F(rpc_test, sunrpc_session)
{
    SessionGetRequest       req;
    SessionGetResponseMsg   rsp;
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
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    hal::session_t *session = ctx_.session();
    EXPECT_NE(session->sfw_rule_id, 0);
    EXPECT_EQ(session->skip_sfw_reval, 0);
    EXPECT_EQ(session->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.session()->idle_timeout, 30);

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
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_SUN_RPC);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    EXPECT_EQ(ctx_.session()->skip_sfw_reval, 1);
    EXPECT_EQ(ctx_.session()->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.session()->sfw_rule_id, 0);
    EXPECT_EQ(ctx_.session()->idle_timeout, 100);
    CHECK_ALLOW_UDP(server_eph, client_eph, 32776, 59374, "c:59374 -> s:32776");
    CHECK_DENY_UDP(server_eph, client_eph, 54890, 49153, "c:49153 -> s:54890");
    CHECK_DENY_TCP(server_eph, client_eph, 32776, 59374, "c:59374 -> s:32776");

    req.set_session_handle(session->hal_handle);
    ret = hal::session_get(req, &rsp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(rsp.response(0).status().alg(), nwsec::APP_SVC_SUN_RPC);
    EXPECT_TRUE(rsp.response(0).status().has_rpc_info());
    EXPECT_EQ(rsp.response(0).status().rpc_info().num_exp_flows(), 6);
    EXPECT_EQ(rsp.response(0).status().rpc_info().num_data_sess(), 2);
}

TEST_F(rpc_test, sunrpc_exp_flow_timeout) {
    app_session_t   *app_sess = NULL;
    l4_alg_status_t *exp_flow1 = NULL, *exp_flow2 = NULL, *l4_sess = NULL;
    hal::flow_key_t  app_sess_key, exp_flow_key;

    app_sess_key.proto = (types::IPProtocol)IP_PROTO_TCP;
    app_sess_key.svrf_id = 1;
    app_sess_key.dvrf_id = 1;
    app_sess_key.sip.v4_addr = 0x16000001;
    app_sess_key.dip.v4_addr = 0x16000002;
    app_sess_key.sport = 12345;
    app_sess_key.dport = 111;

    g_rpc_state->alloc_and_init_app_sess(app_sess_key, &app_sess);

    g_rpc_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
    l4_sess->isCtrl = true;
    ASSERT_EQ(dllist_count(&app_sess->l4_sess_lhead), 1);

    exp_flow_key = app_sess_key;
    exp_flow_key.svrf_id = 1;
    exp_flow_key.dvrf_id = 1;
    exp_flow_key.flow_type = hal::FLOW_TYPE_V4;
    exp_flow_key.proto = types::IPPROTO_TCP;
    exp_flow_key.dport = 22345;
    g_rpc_state->alloc_and_insert_exp_flow(app_sess, exp_flow_key, 
                                         &exp_flow1, true, 8, true);
    exp_flow_key.dport = 22346;
    g_rpc_state->alloc_and_insert_exp_flow(app_sess, exp_flow_key,
                                         &exp_flow2, true, 5, true);
    exp_flow2->entry.ref_count.count++;
    sleep(10);
    ASSERT_EQ(exp_flow2->entry.deleting, true);
    exp_flow2->entry.ref_count.count--;
    sleep(30);
    ASSERT_EQ(dllist_count(&app_sess->exp_flow_lhead), 0);

    g_rpc_state->alloc_and_init_app_sess(app_sess_key, &app_sess);

    g_rpc_state->alloc_and_insert_l4_sess(app_sess, &l4_sess);
    ASSERT_EQ(dllist_count(&app_sess->l4_sess_lhead), 2);

    l4_alg_status_t *exp_flow_old = NULL, *exp_flow_new = NULL;
    exp_flow_key.dport = 22347;
    g_rpc_state->alloc_and_insert_exp_flow(app_sess, exp_flow_key,
                                         &exp_flow_old, true, 5, true);
    ASSERT_EQ(exp_flow_old->entry.deleting, false);
    g_rpc_state->alloc_and_insert_exp_flow(app_sess, exp_flow_key,
                                         &exp_flow_new, true, 5, true);
    ASSERT_EQ(exp_flow_old, exp_flow_new);
    ASSERT_EQ(exp_flow_new->entry.deleting, false); 
    sleep(30);
    ASSERT_EQ(dllist_count(&app_sess->exp_flow_lhead), 0);
}

TEST_F(rpc_test, sunrpc_exp_flow_refcount) {
    app_session_t   *app_sess = NULL;
    l4_alg_status_t *exp_flow1 = NULL, *exp_flow2 = NULL, *l4_sess = NULL;
    hal::flow_key_t  app_sess_key, exp_flow_key;

    app_sess_key.proto = (types::IPProtocol)IP_PROTO_TCP;
    app_sess_key.sip.v4_addr = 0x14000005;
    app_sess_key.dip.v4_addr = 0x14000006;
    app_sess_key.sport = 22246;
    app_sess_key.dport = 111;

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

TEST_F(rpc_test, sunrpc_app_sess_force_delete) {
    SessionGetResponseMsg  resp;
    hal::session_t        *session = NULL;
    hal_ret_t              ret;
    hal_handle_t           sess_hdl = 0;
    SessionDeleteResponseMsg delrsp;

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

    for (int i=0; i<10; i++) {
         hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
         hal::session_delete_all(&delrsp);
         hal::hal_cfg_db_close();
    }
}

TEST_F(rpc_test, sunrpc_session_fragmentation)
{
    SessionGetRequest       req;
    SessionGetResponseMsg   rsp;
    hal_ret_t               ret;
    uint8_t                 dump_call[] = {0x80, 0x00, 0x00, 0x28, 0x2e, 0x93, 0x19, 0x20, 
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
                                           0x00, 0x01, 0x86, 0xa0, 0x00, 0x00, 0x00, 0x03, 
                                           0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                           0x00, 0x00, 0x00, 0x00};
    uint8_t                 dump_rsp1[] = {0x80, 0x00, 0x0A, 0xBC, 0x5C, 0xDF, 0xBF, 0x04, 
                                           0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                                           0x00, 0x08, 0x6A, 0x00, 0x00, 0x00, 0x00, 0x40, 
                                           0x00, 0x00, 0x00, 0x47, 0x46, 0x37, 0x03, 0x60, 
                                           0x00, 0x00, 0x00, 0x83, 0xA3, 0xA2, 0xE3, 0x02, 
                                           0xE3, 0x10};
    uint8_t                 dump_rsp2[] = {0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                                           0x00, 0x01, 0x86, 0xA5, 0x00, 0x00, 0x00, 0x03, 
                                           0x00, 0x00, 0x00, 0x04, 0x74, 0x63, 0x70, 0x36, 
                                           0x00, 0x00, 0x00, 0x08, 0x3A, 0x3A, 0x2E, 0x37, 
                                           0x38, 0x2E, 0x38, 0x30, 0x00, 0x00, 0x00, 0x09,
                                           0x73, 0x75, 0x70, 0x65, 0x72, 0x75, 0x73, 0x65, 
                                           0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 
                                           0x00, 0x01};
   
    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(SUNRPC_PORT, 5001);
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
    tcp = Tins::TCP(5001, SUNRPC_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    tcp.seq(442004268);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    // TCP ACK
    tcp = Tins::TCP(SUNRPC_PORT, 5001);
    tcp.flags(Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //SUNRPC DUMP CALL
    tcp = Tins::TCP(SUNRPC_PORT, 5001) /
          Tins::RawPDU(dump_call, sizeof(dump_call));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //SUNRPC DUMP REPLY
    tcp = Tins::TCP(5001, SUNRPC_PORT) /
         Tins::RawPDU(dump_rsp1, sizeof(dump_rsp1));
    tcp.seq(442004280);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session); 

    tcp = Tins::TCP(5001, SUNRPC_PORT) /
         Tins::RawPDU(dump_rsp2, sizeof(dump_rsp2));
    tcp.seq(442004280 + sizeof(dump_rsp1));
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);
}

TEST_F(rpc_test, sunrpc_getport_fragmentation)
{
    SessionGetRequest       req;
    SessionGetResponseMsg   rsp;
    hal_ret_t               ret;
    uint8_t                 getport_call[] = {0x80, 0x00, 0x00, 0x64, 0xEA, 0x1B, 0x76, 0xA2, 
                                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
                                              0x00, 0x01, 0x86, 0xA0, 0x00, 0x00, 0x00, 0x02, 
                                              0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 
                                              0x00, 0x00, 0x00, 0x2C, 0x00, 0x41, 0xA1, 0xAA, 
                                              0x00, 0x00, 0x00, 0x15, 0x6C, 0x6F, 0x63, 0x61, 
                                              0x6C, 0x68, 0x6F, 0x73, 0x74, 0x2E, 0x6C, 0x6F, 
                                              0x63, 0x61, 0x00, 0x41, 0xA1, 0xAA, 0x00, 0x00, 
                                              0x00, 0x15, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x68, 
                                              0x6F, 0x73, 0x74, 0x2E, 0x6C, 0x6F, 0x63, 0x61, 
                                              0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2C, 
                                              0x00, 0x41, 0xA1, 0xAA, 0x00, 0x00, 0x00, 0x15, 
                                              0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x68, 0x6F, 0x73};
    uint8_t                 getport_call2[] = {0x00, 0x41, 0xA1, 0xAA, 0x00, 0x00, 0x00, 0x15, 
                                               0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x68, 0x6F, 0x73,
                                               0x00, 0x41, 0xA1, 0xAA, 0x00, 0x00, 0x00, 0x15, 
                                               0x6C, 0x6F, 0x63, 0x61, 0x00, 0x01, 0x86, 0xa0, 
                                               0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x11, 
                                               0x00, 0x00, 0x00, 0x00};
    uint8_t                 getport_rsp[] = {0x80, 0x00, 0x00, 0x1C, 0xEA, 0x1B, 0x76, 0xA2,
                                             0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 
                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6f};

    
    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(SUNRPC_PORT, 5003);
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
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    hal::session_t *session = ctx_.session();

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(5003, SUNRPC_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    // TCP ACK
    tcp = Tins::TCP(SUNRPC_PORT, 5003);
    tcp.flags(Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //SUNRPC Getport call1
    tcp = Tins::TCP(SUNRPC_PORT, 5003) /
          Tins::RawPDU(getport_call, sizeof(getport_call));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //SUNRPC Getport call2
    tcp = Tins::TCP(SUNRPC_PORT, 5003) /
          Tins::RawPDU(getport_call2, sizeof(getport_call2));
    tcp.seq(105);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //
    tcp = Tins::TCP(5003, SUNRPC_PORT) /
         Tins::RawPDU(getport_rsp, sizeof(getport_rsp));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    CHECK_ALLOW_TCP(server_eph, client_eph, 111, 54890, "c:54890 -> s:111");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_SUN_RPC);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    EXPECT_EQ(ctx_.session()->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    CHECK_DENY_TCP(server_eph, client_eph, 32776, 59374, "c:59374 -> s:32776");
}

TEST_F(rpc_test, sunrpc_getport_fragmentation_padding)
{
    SessionGetRequest       req;
    SessionGetResponseMsg   rsp;
    hal_ret_t               ret;
    uint8_t                 getport_call[] = {0x80, 0x00, 0x00, 0x64, 0xEA, 0x1B, 0x76, 0xA2,
                                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
                                              0x00, 0x01, 0x86, 0xA0, 0x00, 0x00, 0x00, 0x02,
                                              0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01,
                                              0x00, 0x00, 0x00, 0x2C, 0x00, 0x41, 0xA1, 0xAA,
                                              0x00, 0x00, 0x00, 0x15, 0x6C, 0x6F, 0x63, 0x61,
                                              0x6C, 0x68, 0x6F, 0x73, 0x74, 0x2E, 0x6C, 0x6F,
                                              0x63, 0x61, 0x00, 0x41, 0xA1, 0xAA, 0x00, 0x00,
                                              0x00, 0x15, 0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x68,
                                              0x6F, 0x73, 0x74, 0x2E, 0x6C, 0x6F, 0x63, 0x61,
                                              0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2C,
                                              0x00, 0x41, 0xA1, 0xAA, 0x00, 0x00, 0x00, 0x15,
                                              0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x68, 0x6F, 0x73};
    uint8_t                 getport_call2[] = {0x00, 0x41, 0xA1, 0xAA, 0x00, 0x00, 0x00, 0x15,
                                               0x6C, 0x6F, 0x63, 0x61, 0x6C, 0x68, 0x6F, 0x73,
                                               0x00, 0x41, 0xA1, 0xAA, 0x00, 0x00, 0x00, 0x15,
                                               0x6C, 0x6F, 0x63, 0x61, 0x00, 0x01, 0x86, 0xa0,
                                               0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x11,
                                               0x00, 0x00, 0x00, 0x00};
    uint8_t                 getport_rsp[] = {0x80, 0x00, 0x00, 0x1C, 0xEA, 0x1B, 0x76, 0xA2,
                                             0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc3, 0x23};


    // Create TCP control session
    // TCP SYN
    Tins::TCP tcp = Tins::TCP(SUNRPC_PORT, 5004);
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
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    hal::session_t *session = ctx_.session();

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(5004, SUNRPC_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    // TCP ACK
    tcp = Tins::TCP(SUNRPC_PORT, 5004);
    tcp.flags(Tins::TCP::ACK);
    tcp.add_option(Tins::TCP::option(Tins::TCP::SACK_OK));
    tcp.add_option(Tins::TCP::option(Tins::TCP::NOP));
    tcp.mss(1200);
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp, true);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //SUNRPC Getport call1
    tcp = Tins::TCP(SUNRPC_PORT, 5004) /
          Tins::RawPDU(getport_call, sizeof(getport_call));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //SUNRPC Getport call2
    tcp = Tins::TCP(SUNRPC_PORT, 5004) /
          Tins::RawPDU(getport_call2, sizeof(getport_call2));
    tcp.seq(105);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //
    tcp = Tins::TCP(5004, SUNRPC_PORT) /
         Tins::RawPDU(getport_rsp, sizeof(getport_rsp));
    tcp.seq(1);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    CHECK_ALLOW_UDP(server_eph, client_eph, 49955, 54890, "c:54890 -> s:49955");
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_SUN_RPC);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->rule_id, 0);
    EXPECT_EQ(ctx_.session()->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    CHECK_DENY_UDP(server_eph, client_eph, 32775, 59374, "c:59374 -> s:32775");
}  
