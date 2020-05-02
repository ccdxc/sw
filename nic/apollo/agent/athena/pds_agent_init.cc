//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the initialization for pdsctl athena
///
//----------------------------------------------------------------------------

#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/svc/port.hpp"
#include "nic/apollo/agent/svc/debug.hpp"
#include "nic/apollo/agent/svc/device.hpp"
#include "nic/apollo/agent/svc/svc_thread.hpp"
#include "nic/apollo/agent/core/core.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/agent/athena/grpc_thread.hpp"
#include "nic/apollo/api/include/athena/pds_base.h"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
std::string g_grpc_server_addr;
static sdk::event_thread::event_thread *g_svc_server_thread;
static sdk::event_thread::event_thread *g_grpc_reg_thread;

extern "C" {
//------------------------------------------------------------------------------
// spawn command server thread
//------------------------------------------------------------------------------
sdk_ret_t
spawn_svc_server_thread (void)
{
    // spawn periodic thread that does background tasks
    g_svc_server_thread =
        sdk::event_thread::event_thread::factory(
            "svc", core::PDS_AGENT_THREAD_ID_SVC_SERVER,
            sdk::lib::THREAD_ROLE_CONTROL, 0x0, core::svc_server_thread_init,
            core::svc_server_thread_exit, NULL, // message
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            true);

    SDK_ASSERT_TRACE_RETURN((g_svc_server_thread != NULL), SDK_RET_ERR,
                            "Service server thread create failure");
    g_svc_server_thread->start(g_svc_server_thread);

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// spawn grpc reg thread
//------------------------------------------------------------------------------
sdk_ret_t
spawn_grpc_reg_thread (void)
{
    // spawn periodic thread that does background tasks
    g_grpc_reg_thread =
        sdk::event_thread::event_thread::factory(
            "grpc", core::PDS_AGENT_THREAD_ID_GRPC_REG,
            sdk::lib::THREAD_ROLE_CONTROL, 0x0, core::grpc_reg_thread_init,
            core::grpc_reg_thread_exit, NULL, // message
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            true);

    SDK_ASSERT_TRACE_RETURN((g_grpc_reg_thread != NULL), SDK_RET_ERR,
                            "Grpc registrition thread create failure");
    g_grpc_reg_thread->start(g_grpc_reg_thread);

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// initialize the agent
//------------------------------------------------------------------------------
pds_ret_t
pds_agent_init()
{
    sdk_ret_t    ret;

    // init agent state
    ret = core::agent_state::init();
    if (ret != SDK_RET_OK) {
        return (pds_ret_t)ret;
    }

    // spawn service server thread
    ret = spawn_svc_server_thread();
    if (ret != SDK_RET_OK) {
        return (pds_ret_t)ret;
    }

    // register for all gRPC services
    ret = spawn_grpc_reg_thread();
    if (ret != SDK_RET_OK) {
        return (pds_ret_t)ret;
    }

    return (pds_ret_t)ret;
}
}
