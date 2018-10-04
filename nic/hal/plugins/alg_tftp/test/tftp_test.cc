#include <gtest/gtest.h>
#include "nic/hal/plugins/alg_tftp/core.hpp"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "tftp_test.hpp"

#include <tins/tins.h>

using namespace hal::plugins::alg_tftp;
using namespace session;
using namespace fte;

hal_handle_t tftp_test::client_eph, tftp_test::server_eph;

TEST_F(tftp_test, tftp_session)
{
    SessionGetRequest       req;
    SessionGetResponseMsg   rsp;
    hal_ret_t          ret;
    uint8_t            tftp_rrq[] = {0x00, 0x01, 0x72, 0x66, 0x63, 0x31, 0x33, 0x35,
                                     0x30, 0x2e, 0x74, 0x78, 0x74, 0x00, 0x6f, 0x63,
                                     0x74, 0x6, 0x74, 0x00};
    uint8_t            tftp_rrq_rsp[] = {0x00, 0x04, 0x00, 0x01, 0x0a, 0x0a, 0x0a, 0x0a};

    // Create session 
    // Send traffic on TFTP_PORT as dport
    Tins::UDP udp = Tins::UDP(TFTP_PORT, 1000) /
                    Tins::RawPDU(tftp_rrq, sizeof(tftp_rrq)); 
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, udp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_TFTP);
    hal::session_t *session = ctx_.session();

    //send response 
    udp = Tins::UDP(1000, 1001) /
          Tins::RawPDU(tftp_rrq_rsp, sizeof(tftp_rrq_rsp));
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, client_eph, server_eph, udp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_NE(ctx_.session(), session);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sfw_action, nwsec::SECURITY_RULE_ACTION_NONE);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg, nwsec::APP_SVC_TFTP);

    req.set_session_handle(session->hal_handle);
    ret = hal::session_get(req, &rsp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(rsp.response(0).status().alg(), nwsec::APP_SVC_TFTP);
    EXPECT_TRUE(rsp.response(0).status().has_tftp_info());
    EXPECT_EQ(rsp.response(0).status().tftp_info().parse_error(), 0);
    EXPECT_EQ(rsp.response(0).status().tftp_info().unknown_opcode(), 1);
}

TEST_F(tftp_test, app_sess_force_delete) {
    SessionGetResponseMsg  resp;
    hal::session_t        *session = NULL;
    hal_ret_t              ret;
    hal_handle_t           sess_hdl = 0;

    ret = hal::session_get_all(&resp);
    EXPECT_EQ(ret, HAL_RET_OK);
    for (int idx=0; idx<resp.response_size(); idx++) {
        SessionGetResponse rsp = resp.response(idx);
        if (rsp.status().has_tftp_info() && 
            rsp.status().tftp_info().iscontrol()) {
            sess_hdl = rsp.status().session_handle();
        }
    }
    
    // Invoke delete callback
    session = hal::find_session_by_handle(sess_hdl);
    ASSERT_TRUE(session != NULL);
    ret = session_delete(session, true);
    ASSERT_EQ(ret, HAL_RET_OK);
}
