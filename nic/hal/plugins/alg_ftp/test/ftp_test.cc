#include <gtest/gtest.h>
#include "nic/fte/test/fte_base_test.hpp"
#include "nic/hal/plugins/alg_ftp/core.hpp"
#include "nic/p4/iris/include/defines.h"

#include <tins/tins.h>

using namespace hal::plugins::alg_ftp;
using namespace session;
using namespace fte;

#define FTP_PORT 21
#define FTP_DATA_PORT 20

class ftp_test : public fte_base_test {
protected:
    ftp_test() {}

    virtual ~ftp_test() {}

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
                               dport_low: FTP_PORT, dport_high: FTP_PORT,
                               alg: nwsec::APP_SVC_FTP } },
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
};

hal_handle_t ftp_test::client_eph, ftp_test::server_eph;

TEST_F(ftp_test, ftp_session)
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
    ret = inject_ipv4_pkt(fte::FLOW_MISS_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_FALSE(ctx_.drop_flow());
    EXPECT_TRUE(ctx_.session()->iflow->pgm_attrs.mcast_en);
    EXPECT_TRUE(ctx_.session()->rflow->pgm_attrs.mcast_en);
    EXPECT_EQ(ctx_.session()->iflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    EXPECT_EQ(ctx_.session()->rflow->pgm_attrs.mcast_ptr, P4_NW_MCAST_INDEX_FLOW_REL_COPY);
    hal::session_t *session = ctx_.session();

    // TCP SYN/ACK on ALG_CFLOW_LIFQ
    tcp = Tins::TCP(2000, FTP_PORT);
    tcp.flags(Tins::TCP::SYN | Tins::TCP::ACK);
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(ctx_.session(), session);

    //FTP PORT Command
    tcp = Tins::TCP(FTP_PORT, 2000) /
          Tins::RawPDU(ftp_port, sizeof(ftp_port));
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, server_eph, client_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    //FTP PORT Response
    tcp = Tins::TCP(2000, FTP_PORT) /
         Tins::RawPDU(ftp_port_rsp, sizeof(ftp_port_rsp));
    ret = inject_ipv4_pkt(fte::ALG_CFLOW_LIFQ, client_eph, server_eph, tcp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_FALSE(ctx_.drop());
    EXPECT_EQ(ctx_.session(), session);

    CHECK_ALLOW_TCP(client_eph, server_eph, 37075, FTP_DATA_PORT, "c:20 -> s:37075");
    CHECK_DENY_TCP(client_eph, server_eph, 37075, 2000, "c:2000 -> s:37075");
    CHECK_DENY_TCP(server_eph, client_eph,  37075, 2000, "c:2000 -> s:37075"); 
    CHECK_DENY_TCP(client_eph, server_eph, 37075, 2001, "c:2001 -> s:37075");

    req.set_session_handle(session->hal_handle);
    req.mutable_meta()->set_vrf_id(ctx_.svrf()->vrf_id);
    ret = hal::session_get(req, &rsp);
    EXPECT_EQ(ret, HAL_RET_OK);
    EXPECT_EQ(rsp.status().alg(), nwsec::APP_SVC_FTP);
    EXPECT_TRUE(rsp.status().has_ftp_info());
    EXPECT_EQ(rsp.status().ftp_info().parse_error(), 0);
}

TEST_F(ftp_test, app_sess_force_delete) {
    SessionGetResponseMsg  resp;
    hal::session_t        *session = NULL;
    hal_ret_t              ret;
    hal_handle_t           sess_hdl;

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
