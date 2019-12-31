//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains VPP mock support
///
//----------------------------------------------------------------------------

#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/core/msg.h"
#include "nic/apollo/test/base/vpp_mock.hpp"

using namespace sdk::lib;
namespace event = sdk::event_thread;

static void
vpp_cfg_msg_cb (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    sdk_ret_t ret = SDK_RET_OK;

    SDK_TRACE_DEBUG("Sending OK response from vpp mock");
    sdk::ipc::respond(msg, (const void *)&ret, sizeof(ret));
}

static void
vpp_init_func (void *ctxt)
{
    sdk::ipc::reg_request_handler(PDS_MSG_TYPE_CFG, vpp_cfg_msg_cb, NULL);
}

sdk_ret_t
vpp_thread_create (void)
{
    event::event_thread *vpp;

    vpp = event::event_thread::factory("vpp", PDS_IPC_ID_VPP,
              THREAD_ROLE_CONTROL, 0x0, vpp_init_func,
              NULL, NULL,
              sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
              sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
              NULL);
    SDK_ASSERT(vpp != NULL);
    vpp->start(vpp);

    return SDK_RET_OK;
}
