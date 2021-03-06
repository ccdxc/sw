//-----------------------------------------------------------------------------
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "nic/hal/plugins/sfw/alg_dns/core.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "dns_test.hpp"

#include <tins/tins.h>

using namespace hal::plugins::alg_dns;
using namespace session;
using namespace fte;

hal_handle_t dns_test::client_eph, dns_test::server_eph;

TEST_F(dns_test, dns_session)
{
    SessionGetRequest     req;
    SessionGetResponseMsg rsp;
    hal_ret_t             ret;
    uint8_t            dns_query[] = {0x10, 0x32, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 
                                      0x00, 0x00, 0x00, 0x00, 0x06, 0x67, 0x6f, 0x6f,
                                      0x67, 0x6c, 0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 
                                      0x00, 0x10, 0x00, 0x01};
    uint8_t            dns_query_rsp[] = {0x10, 0x32, 0x81, 0x80, 0x00, 0x01, 0x00, 0x01, 
                                          0x00, 0x00, 0x00, 0x00, 0x06, 0x67, 0x6f, 0x6f,
                                          0x67, 0x6c, 0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00, 
                                          0x00, 0x10, 0x00, 0x01, 0xc0, 0x0c, 0x00, 0x10,
                                          0x00, 0x01, 0x00, 0x00, 0x01, 0x0e, 0x00, 0x10, 
                                          0x0f, 0x76, 0x3d, 0x73, 0x70, 0x66, 0x31, 0x20,
                                          0x70, 0x74, 0x72, 0x20, 0x3f, 0x61, 0x6c, 0x6c};


    // Create session
    // Send traffic on DNS_PORT as dport
    Tins::UDP udp = Tins::UDP(DNS_PORT, 1000) /
                    Tins::RawPDU(dns_query, sizeof(dns_query));
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, udp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_DNS);
    hal::session_t *session = ctx_.session();

    //send response
    udp = Tins::UDP(1000, DNS_PORT) /
          Tins::RawPDU(dns_query_rsp, sizeof(dns_query_rsp));
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, udp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    req.set_session_handle(session->hal_handle);
    ret = hal::session_get(req, &rsp);
    EXPECT_NE(ret, HAL_RET_OK);
}

TEST_F(dns_test, app_sess_force_delete) {
    SessionGetResponseMsg  resp1, resp2;
    hal::session_t        *session = NULL;
    hal_ret_t              ret;
    hal_handle_t           sess_hdl = 0;
    uint32_t               alg_sessions = 0;
    uint8_t                dns_query[] = {0x10, 0x32, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x06, 0x67, 0x6f, 0x6f,
                                      0x67, 0x6c, 0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00,
                                      0x00, 0x10, 0x00, 0x01};

    // Create session
    // Send traffic on DNS_PORT as dport
    Tins::UDP udp = Tins::UDP(DNS_PORT, 1000) /
                    Tins::RawPDU(dns_query, sizeof(dns_query));
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, udp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_DNS);
    session = ctx_.session();

    ret = hal::session_get_all(&resp1);
    EXPECT_EQ(ret, HAL_RET_OK);
    for (int idx=0; idx<resp1.response_size(); idx++) {
        SessionGetResponse rsp = resp1.response(idx);
        if (rsp.status().alg() == nwsec::APP_SVC_DNS)
            sess_hdl = rsp.status().session_handle();
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
        if (rsp.status().alg() == nwsec::APP_SVC_DNS) 
            alg_sessions++;
    }

    EXPECT_EQ(alg_sessions, 0);
}

TEST_F(dns_test, dns_session_rflow_flow_miss)
{
    SessionGetRequest     req;
    SessionGetResponseMsg rsp;
    hal_ret_t             ret;
    uint8_t            dns_query[] = {0x10, 0x32, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x06, 0x67, 0x6f, 0x6f,
                                      0x67, 0x6c, 0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00,
                                      0x00, 0x10, 0x00, 0x01};
    uint8_t            dns_query_rsp[] = {0x10, 0x32, 0x81, 0x80, 0x00, 0x01, 0x00, 0x01,
                                          0x00, 0x00, 0x00, 0x00, 0x06, 0x67, 0x6f, 0x6f,
                                          0x67, 0x6c, 0x65, 0x03, 0x63, 0x6f, 0x6d, 0x00,
                                          0x00, 0x10, 0x00, 0x01, 0xc0, 0x0c, 0x00, 0x10,
                                          0x00, 0x01, 0x00, 0x00, 0x01, 0x0e, 0x00, 0x10,
                                          0x0f, 0x76, 0x3d, 0x73, 0x70, 0x66, 0x31, 0x20,
                                          0x70, 0x74, 0x72, 0x20, 0x3f, 0x61, 0x6c, 0x6c};


    // Create session
    // Send traffic on DNS_PORT as dport
    Tins::UDP udp = Tins::UDP(DNS_PORT, 1001) /
                    Tins::RawPDU(dns_query, sizeof(dns_query));
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, udp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_DNS);
    hal::session_t *session = ctx_.session();

    //send response on flow-miss lifq
    udp = Tins::UDP(1001, DNS_PORT) /
          Tins::RawPDU(dns_query_rsp, sizeof(dns_query_rsp));
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, client_eph, server_eph, udp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    req.set_session_handle(session->hal_handle);
    ret = hal::session_get(req, &rsp);
    EXPECT_EQ(ret, HAL_RET_OK);

    //send response on cflow lifq
    udp = Tins::UDP(1001, DNS_PORT) /
          Tins::RawPDU(dns_query_rsp, sizeof(dns_query_rsp));
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, udp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    req.set_session_handle(session->hal_handle);
    ret = hal::session_get(req, &rsp);
    EXPECT_NE(ret, HAL_RET_OK); 
} 
