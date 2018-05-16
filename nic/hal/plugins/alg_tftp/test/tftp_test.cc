#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/plugins/alg_tftp/core.hpp"
#include "nic/p4/iris/include/defines.h"

#include <tins/tins.h>

using namespace hal::plugins::alg_tftp;
using namespace session;
using namespace fte;

#define TFTP_PORT 69

class tftp_test : public fte_base_test {
protected:
    tftp_test() {}

    virtual ~tftp_test() {}

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
                        app: { proto:IPPROTO_UDP,
                               dport_low: TFTP_PORT, dport_high: TFTP_PORT,
                               alg: nwsec::APP_SVC_TFTP } },
            v4_rule_t { action: nwsec::SECURITY_RULE_ACTION_DENY,
                        from: {},
                        to: {},
                        app: { proto:IPPROTO_UDP,
                               dport_low: 0, dport_high: 0xFFFF,
                               alg: nwsec::APP_SVC_NONE} },
        };

        add_nwsec_policy(vrfh, rules);
    }
    
    static hal_handle_t client_eph, server_eph;

public:
    static void tftp_session_create(void *ptr);
};

hal_handle_t tftp_test::client_eph, tftp_test::server_eph;

TEST_F(tftp_test, tftp_session)
{
    SessionGetRequest  req;
    SessionGetResponse rsp;
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
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sipv4(), ctx_.key().sip.v4_addr);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->dipv4(), ctx_.key().dip.v4_addr);
    EXPECT_EQ(((uint16_t)ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sport()), ctx_.key().sport);
    EXPECT_EQ(((uint16_t)ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->dport()), ctx_.key().dport);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->fwaction(), nwsec::SECURITY_RULE_ACTION_ALLOW);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg(), nwsec::APP_SVC_TFTP);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->session_id(), 0);
    hal::session_t *session = ctx_.session();

    //send response 
    udp = Tins::UDP(1000, 1001) /
          Tins::RawPDU(tftp_rrq_rsp, sizeof(tftp_rrq_rsp));
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, client_eph, server_eph, udp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_NE(ctx_.session(), session);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sipv4(), ctx_.key().sip.v4_addr);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->dipv4(), ctx_.key().dip.v4_addr);
    EXPECT_EQ(((uint16_t)ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->sport()), ctx_.key().sport);
    EXPECT_EQ(((uint16_t)ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->dport()), ctx_.key().dport);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->fwaction(), nwsec::SECURITY_RULE_ACTION_NONE);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->alg(), nwsec::APP_SVC_TFTP);
    EXPECT_EQ(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->parent_session_id(), session->hal_handle);
    EXPECT_NE(ctx_.flow_log(hal::FLOW_ROLE_INITIATOR)->session_id(), 0);

    req.set_session_handle(session->hal_handle);
    req.mutable_meta()->set_vrf_id(ctx_.svrf()->vrf_id);
    ret = hal::session_get(req, &rsp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(rsp.status().alg(), nwsec::APP_SVC_TFTP);
    EXPECT_TRUE(rsp.status().has_tftp_info());
    EXPECT_EQ(rsp.status().tftp_info().parse_error(), 0);
    EXPECT_EQ(rsp.status().tftp_info().unknown_opcode(), 1);
}

TEST_F(tftp_test, app_sess_force_delete) {
    SessionGetResponseMsg  resp;
    hal::session_t        *session = NULL;
    hal_ret_t              ret;
    hal_handle_t           sess_hdl;

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
