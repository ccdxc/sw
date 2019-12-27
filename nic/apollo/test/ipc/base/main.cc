//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains test cases for validating the base IPC infra
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/ipc/ipc.hpp"
#include "nic/apollo/core/msg.h"
#include "nic/apollo/api/core/msg.h"
#include "nic/apollo/test/api/utils/api_base.hpp"

namespace test {
namespace ipc {

//----------------------------------------------------------------------------
// base IPC test class
//----------------------------------------------------------------------------

class ipc_base_test : public ::pds_test_base {
protected:
    ipc_base_test() {}
    virtual ~ipc_base_test() {}
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

/// \brief Dummy "OK" test. Send an event with ID "none". This returns an OK
///        code from VPP. This is a sanity test for bi-directiopnal
///        communication between test infra and VPP IPC infra
TEST_F(ipc_base_test, ipc_base_ok_test) {
    if (!apulu()) return;
    pds_msg_t *msg;
    sdk::sdk_ret_t ret_val;

    msg = new pds_msg_t;
    msg->id = PDS_MSG_ID_NONE;

    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_EVENT, msg,
                      sizeof(pds_msg_t), response_handler_cb,
                      (const void *)&ret_val);
    EXPECT_TRUE(ret_val == sdk::SDK_RET_OK);
    delete msg;
}

/// \brief Dummy "INVALID" test. Send a message with "none" type. This returns a
///        fail "INVALID" code from VPP. This is a sanity test for
///        bi-directiopnal communication between test infra and VPP IPC infra
TEST_F(ipc_base_test, ipc_base_invalid_test) {
    if (!apulu()) return;
    pds_msg_t *msg;
    sdk::sdk_ret_t ret_val;

    msg = new pds_msg_t;
    msg->id = PDS_MSG_ID_NONE;

    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_NONE, msg,
                      sizeof(pds_msg_t), response_handler_cb,
                      (const void *)&ret_val);
    EXPECT_TRUE(ret_val == sdk::SDK_RET_INVALID_OP);
    delete msg;
}

#define TEST_BATCH_SIZE 10

/// \brief batch sanity test. Send a batch of dummy messages with ID none,
///        which should be NO-ops. The batch should succeed with return code of
///        "OK"
TEST_F(ipc_base_test, ipc_base_batch_test) {
    if (!apulu()) return;
    pds_msg_list_t *msglist;
    size_t sz = sizeof(pds_msg_list_t) + (TEST_BATCH_SIZE * sizeof(pds_msg_t));
    sdk::sdk_ret_t ret_val;

    msglist = (pds_msg_list_t *)malloc(sz);
    msglist->num_msgs = TEST_BATCH_SIZE;
    for (int i = 0; i < msglist->num_msgs; i++) {
        pds_msg_t *msg = &msglist->msgs[i];
        msg->id = PDS_MSG_ID_NONE;
    }
    sdk::ipc::request(PDS_IPC_ID_VPP, PDS_MSG_TYPE_BATCH, msglist,
                      sz, response_handler_cb, (const void *)&ret_val);
    EXPECT_TRUE(ret_val == sdk::SDK_RET_OK);
    free(msglist);
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
