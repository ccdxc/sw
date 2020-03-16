//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains test cases for configuring VPP dhcp relay plugin thru IPC
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/core/msg.h"
#include "nic/apollo/api/core/msg.h"
#include "nic/apollo/test/api/utils/api_base.hpp"
#include "nic/apollo/api/include/pds_dhcp.hpp"

namespace test {
namespace ipc {

//----------------------------------------------------------------------------
// dhcp relay plugin IPC config test class
//----------------------------------------------------------------------------

class ipc_dhcp_relay_test : public ::pds_test_base {
protected:
    ipc_dhcp_relay_test() {}
    virtual ~ipc_dhcp_relay_test() {}
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

/// \brief configure dhcp relay plugin using a security profile and a CREATE op.
///        This test uses a singleton configuration message
TEST_F(ipc_dhcp_relay_test, ipc_dhcp_relay_cfg_create) {
    if (!apulu()) return;
    sdk::sdk_ret_t ret_val;
    pds_dhcp_policy_spec_t spec;
    pds_obj_key_t  key;

    spec.key = int2pdsobjkey(1234);
    spec.type = PDS_DHCP_POLICY_TYPE_RELAY;
    spec.relay_spec.vpc = int2pdsobjkey(200);
    str2ipaddr("1.2.3.4", &spec.relay_spec.server_ip);
    str2ipaddr("5.6.7.8", &spec.relay_spec.agent_ip);

    ret_val = pds_dhcp_policy_create(&spec);
    EXPECT_EQ(ret_val, sdk::SDK_RET_OK);

    // read back the same entry, it should succeed
    pds_msg_t request;
    reply_data_t reply;

    memset(&request.cfg_msg.dhcp_policy, 0,
           sizeof(pds_dhcp_policy_cfg_msg_t));
    request.id = PDS_CFG_MSG_ID_DHCP_POLICY;
    request.cfg_msg.op = API_OP_NONE;
    request.cfg_msg.obj_id = OBJ_ID_DHCP_POLICY;
    request.cfg_msg.dhcp_policy.key = int2pdsobjkey(1234);

    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CMD, &request,
                      sizeof(pds_msg_t), read_handler_cb, (const void *)&reply);
    EXPECT_TRUE(reply.is_msg);
    EXPECT_EQ(reply.msg.cfg_msg.dhcp_policy.spec.key, int2pdsobjkey(1234));
    EXPECT_EQ(reply.msg.cfg_msg.dhcp_policy.spec.relay_spec.vpc, int2pdsobjkey(200));
    EXPECT_EQ(reply.msg.cfg_msg.dhcp_policy.spec.relay_spec.server_ip.af, IP_AF_IPV4);
    EXPECT_EQ(reply.msg.cfg_msg.dhcp_policy.spec.relay_spec.agent_ip.af, IP_AF_IPV4);
    EXPECT_EQ(reply.msg.cfg_msg.dhcp_policy.spec.relay_spec.server_ip.addr.v4_addr,
              spec.relay_spec.server_ip.addr.v4_addr);
    EXPECT_EQ(reply.msg.cfg_msg.dhcp_policy.spec.relay_spec.agent_ip.addr.v4_addr,
              spec.relay_spec.agent_ip.addr.v4_addr);

    // now delete the entry, it should succeed
    key = int2pdsobjkey(1234);
    pds_dhcp_policy_delete(&key);
    EXPECT_EQ(ret_val, sdk::SDK_RET_OK);

    // now try to read the entry, it should fail
    request.id = PDS_CFG_MSG_ID_DHCP_POLICY;
    request.cfg_msg.op = API_OP_NONE;
    request.cfg_msg.obj_id = OBJ_ID_DHCP_POLICY;
    request.cfg_msg.dhcp_policy.key = int2pdsobjkey(1234);
    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_CMD, &request,
                      sizeof(pds_msg_t), read_handler_cb, (const void *)&reply);
    EXPECT_FALSE(reply.is_msg);
    EXPECT_EQ(reply.code, sdk::SDK_RET_ENTRY_NOT_FOUND);
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
