//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains test cases for configuring VPP flow plugin thru IPC
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/core/msg.h"
#include "nic/apollo/api/core/msg.h"
#include "nic/apollo/test/api/utils/api_base.hpp"

namespace test {
namespace ipc {

//----------------------------------------------------------------------------
// flow plugin IPC config test class
//----------------------------------------------------------------------------

class ipc_flow_test : public ::pds_test_base {
protected:
    ipc_flow_test() {}
    virtual ~ipc_flow_test() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
    static void SetUpTestCase() {
        if (!agent_mode())
            pds_test_base::SetUpTestCase(g_tc_params);
    }
    static void TearDownTestCase() {
        if (!agent_mode())
            pds_test_base::TearDownTestCase();
    }
};

/// \brief callback function used to validate return value
static void
response_handler_cb(sdk::ipc::ipc_msg_ptr msg, const void *request_cookie) {
    ASSERT_TRUE(msg->length() == sizeof(sdk::sdk_ret_t));
    sdk::sdk_ret_t *ret_val = (sdk::sdk_ret_t *)request_cookie;

    // store the value in a global, so the test can validate
    *ret_val = *(sdk::sdk_ret_t *)msg->data();
}

typedef struct reply_data_s {
    bool is_msg;
    union {
        pds_msg_t msg;
        sdk::sdk_ret_t code;
    };
} reply_data_t;

/// \brief callback function for reading, used with command msg type
static void
read_handler_cb(sdk::ipc::ipc_msg_ptr msg, const void *request_cookie) {
    reply_data_t *reply = (reply_data_t *)request_cookie;

    if (msg->length() == sizeof(sdk::sdk_ret_t)) {
        reply->is_msg = false;
        reply->code = *(sdk::sdk_ret_t *)msg->data();
        return;
    }

    if (msg->length() == sizeof(pds_msg_t)) {
        reply->is_msg = true;
        memcpy(&reply->msg, msg->data(), sizeof(pds_msg_t));
        return;
    }

    // unknown response size
    ASSERT_TRUE(0);
}


/// \brief read flow plugin config and verify that default timeouts are setup
//         correctly
TEST_F(ipc_flow_test, ipc_flow_cfg_read) {
    if (!apulu()) return;
    pds_msg_t request;
    reply_data_t reply;

    memset(&request.cfg_msg.security_profile, 0,
           sizeof(pds_security_profile_cfg_msg_t));
    request.id = PDS_CFG_MSG_ID_SECURITY_PROFILE;
    request.cfg_msg.op = API_OP_NONE;
    request.cfg_msg.security_profile.key.id = 9999;

    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CMD, &request,
                      sizeof(pds_msg_t), read_handler_cb, (const void *)&reply);
    EXPECT_TRUE(reply.is_msg);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.key.id == 9999);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.tcp_idle_timeout ==
                PDS_DEFAULT_TCP_IDLE_TIMEOUT);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.udp_idle_timeout ==
                PDS_DEFAULT_UDP_IDLE_TIMEOUT);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.icmp_idle_timeout ==
                PDS_DEFAULT_ICMP_IDLE_TIMEOUT);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.other_idle_timeout ==
                PDS_DEFAULT_OTHER_IDLE_TIMEOUT);
}

/// \brief configure flow plugin using a security profile and a CREATE op.
///        This test uses a singleton configuration message
TEST_F(ipc_flow_test, ipc_flow_cfg_reate) {
    if (!apulu()) return;
    pds_msg_t *msg;
    sdk::sdk_ret_t ret_val;
    reply_data_t reply;

    msg = new pds_msg_t;
    msg->id = PDS_CFG_MSG_ID_SECURITY_PROFILE;
    msg->cfg_msg.op = API_OP_CREATE;
    msg->cfg_msg.security_profile.spec.tcp_idle_timeout = 100;
    msg->cfg_msg.security_profile.spec.udp_idle_timeout = 200;
    msg->cfg_msg.security_profile.spec.icmp_idle_timeout = 300;
    msg->cfg_msg.security_profile.spec.other_idle_timeout = 400;

    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CFG, msg, sizeof(pds_msg_t),
                      response_handler_cb, (const void *)&ret_val);
    EXPECT_TRUE(ret_val == sdk::SDK_RET_OK);

    // Read data and verify that it is configured value
    memset(&msg->cfg_msg.security_profile, 0,
           sizeof(pds_security_profile_cfg_msg_t));
    msg->id = PDS_CFG_MSG_ID_SECURITY_PROFILE;
    msg->cfg_msg.op = API_OP_NONE;
    msg->cfg_msg.security_profile.key.id = 1234;

    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CMD, msg, sizeof(pds_msg_t),
                      read_handler_cb, (const void *)&reply);
    EXPECT_TRUE(reply.is_msg);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.key.id == 1234);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.tcp_idle_timeout ==
                100);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.udp_idle_timeout ==
                200);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.icmp_idle_timeout ==
                300);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.other_idle_timeout ==
                400);

    delete msg;
}

/// \brief configure flow plugin using a security profile and an UPDATE op.
///        This test uses a singleton configuration message
TEST_F(ipc_flow_test, ipc_flow_update) {
    if (!apulu()) return;
    pds_msg_t *msg;
    sdk::sdk_ret_t ret_val;
    reply_data_t reply;

    msg = new pds_msg_t;
    msg->id = PDS_CFG_MSG_ID_SECURITY_PROFILE;
    msg->cfg_msg.op = API_OP_UPDATE;
    msg->cfg_msg.security_profile.spec.tcp_idle_timeout = 1000;
    msg->cfg_msg.security_profile.spec.udp_idle_timeout = 2000;
    msg->cfg_msg.security_profile.spec.icmp_idle_timeout = 3000;
    msg->cfg_msg.security_profile.spec.other_idle_timeout = 4000;

    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CFG, msg, sizeof(pds_msg_t),
                      response_handler_cb, (const void *)&ret_val);
    EXPECT_TRUE(ret_val == sdk::SDK_RET_OK);

    // Read data and verify that it is configured value
    memset(&msg->cfg_msg.security_profile, 0,
           sizeof(pds_security_profile_cfg_msg_t));
    msg->id = PDS_CFG_MSG_ID_SECURITY_PROFILE;
    msg->cfg_msg.op = API_OP_NONE;
    msg->cfg_msg.security_profile.key.id = 5555;

    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CMD, msg, sizeof(pds_msg_t),
                      read_handler_cb, (const void *)&reply);
    EXPECT_TRUE(reply.is_msg);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.key.id == 5555);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.tcp_idle_timeout ==
                1000);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.udp_idle_timeout ==
                2000);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.icmp_idle_timeout ==
                3000);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.other_idle_timeout ==
                4000);

    delete msg;
}

#define TEST_BATCH_SIZE 3

/// \brief Batch configuration for flow plugin. Send multiple CREATE ops, and
///        verify processing
TEST_F(ipc_flow_test, ipc_flow_batch_test) {
    if (!apulu()) return;
    pds_msg_list_t *msglist;
    size_t sz = sizeof(pds_msg_list_t) + (TEST_BATCH_SIZE * sizeof(pds_msg_t));
    sdk::sdk_ret_t ret_val;
    pds_msg_t request;
    reply_data_t reply;

    msglist = (pds_msg_list_t *)malloc(sz);
    msglist->num_msgs = TEST_BATCH_SIZE;
    for (int i = 0; i < msglist->num_msgs; i++) {
        pds_msg_t *msg = &msglist->msgs[i];
        msg->id = PDS_CFG_MSG_ID_SECURITY_PROFILE;
        msg->cfg_msg.op = API_OP_CREATE;
        msg->cfg_msg.security_profile.spec.tcp_idle_timeout = i * 100;
        msg->cfg_msg.security_profile.spec.udp_idle_timeout = i * 200;
        msg->cfg_msg.security_profile.spec.icmp_idle_timeout = i * 300;
        msg->cfg_msg.security_profile.spec.other_idle_timeout = i * 400;
    }
    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CFG, msglist,
                      sz, response_handler_cb, (const void *)&ret_val);
    EXPECT_TRUE(ret_val == sdk::SDK_RET_OK);
    free(msglist);

    // Read data and verify that it is last configured value
    memset(&request.cfg_msg.security_profile, 0,
           sizeof(pds_security_profile_cfg_msg_t));
    request.id = PDS_CFG_MSG_ID_SECURITY_PROFILE;
    request.cfg_msg.op = API_OP_NONE;
    request.cfg_msg.security_profile.key.id = 2222;

    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CMD, &request,
                      sizeof(pds_msg_t), read_handler_cb, (const void *)&reply);
    EXPECT_TRUE(reply.is_msg);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.key.id == 2222);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.tcp_idle_timeout ==
                (TEST_BATCH_SIZE - 1) * 100);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.udp_idle_timeout ==
                (TEST_BATCH_SIZE - 1) * 200);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.icmp_idle_timeout ==
                (TEST_BATCH_SIZE - 1) * 300);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.other_idle_timeout ==
                (TEST_BATCH_SIZE - 1) * 400);

}

/// \brief configure flow plugin using a security profile and a DELETE op.
///        This test uses a singleton configuration message. The expectation is
///        the configuration values are restored to default, and the spec
///        contents are ignored
TEST_F(ipc_flow_test, ipc_flow_delete) {
    if (!apulu()) return;
    pds_msg_t *msg;
    sdk::sdk_ret_t ret_val;
    reply_data_t reply;

    msg = new pds_msg_t;
    msg->id = PDS_CFG_MSG_ID_SECURITY_PROFILE;
    msg->cfg_msg.op = API_OP_DELETE;
    msg->cfg_msg.security_profile.key.id = 4321;

    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CFG, msg, sizeof(pds_msg_t),
                      response_handler_cb, (const void *)&ret_val);
    EXPECT_TRUE(ret_val == sdk::SDK_RET_OK);

    // read back the values and they should be default again

    memset(&msg->cfg_msg.security_profile, 0,
           sizeof(pds_security_profile_cfg_msg_t));
    msg->id = PDS_CFG_MSG_ID_SECURITY_PROFILE;
    msg->cfg_msg.op = API_OP_NONE;
    msg->cfg_msg.security_profile.key.id = 4321;

    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CMD, msg,
                      sizeof(pds_msg_t), read_handler_cb, (const void *)&reply);
    EXPECT_TRUE(reply.is_msg);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.key.id == 4321);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.tcp_idle_timeout ==
                PDS_DEFAULT_TCP_IDLE_TIMEOUT);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.udp_idle_timeout ==
                PDS_DEFAULT_UDP_IDLE_TIMEOUT);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.icmp_idle_timeout ==
                PDS_DEFAULT_ICMP_IDLE_TIMEOUT);
    EXPECT_TRUE(reply.msg.cfg_msg.security_profile.spec.other_idle_timeout ==
                PDS_DEFAULT_OTHER_IDLE_TIMEOUT);
}


}    // namespace ipc
}    // namespace test

//----------------------------------------------------------------------------
// Entry point
//----------------------------------------------------------------------------

int
main (int argc, char **argv)
{
    return api_test_program_run(argc, argv);
}
