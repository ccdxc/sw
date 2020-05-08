//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//------------------------------------------------------------------------------

#include "grpc++/grpc++.h"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/include/sdk/globals.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/include/globals.hpp"
#include "nic/operd/daemon/operd_impl.hpp"
#include "nic/operd/daemon/svc/oper.hpp"

static sdk::event_thread::event_thread *g_grpc_svc_thread;

using grpc::Server;
using grpc::ServerBuilder;

static void
grpc_svc_init (void)
{
    ServerBuilder *server_builder;
    OperSvcImpl   oper_svc;
    std::string   grpc_server_addr;

    // do gRPC initialization
    grpc_init();
    grpc_server_addr =
        std::string("0.0.0.0:") + std::to_string(PDS_GRPC_PORT_OPERD);
    server_builder = new ServerBuilder();
    server_builder->SetMaxReceiveMessageSize(INT_MAX);
    server_builder->SetMaxSendMessageSize(INT_MAX);
    server_builder->AddListeningPort(grpc_server_addr,
                                     grpc::InsecureServerCredentials());

    // register for the oper services
    server_builder->RegisterService(&oper_svc);

    fprintf(stdout, "operd server listening on ... %s\n",
            grpc_server_addr.c_str());
    std::unique_ptr<Server> server(server_builder->BuildAndStart());
    server->Wait();
}

void grpc_svc_thread_init (void *ctxt)
{
    grpc_svc_init();
}

void grpc_svc_thread_exit (void *ctxt)
{
    return;
}

static sdk_ret_t
spawn_grpc_svc_thread (void)
{
    // spawn grpc server which handles oper stuff like techsupport
    g_grpc_svc_thread =
        sdk::event_thread::event_thread::factory(
            "grpc", SDK_IPC_ID_OPERD,
            sdk::lib::THREAD_ROLE_CONTROL, 0x0, grpc_svc_thread_init,
            grpc_svc_thread_exit, NULL,
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            true);

    SDK_ASSERT_TRACE_RETURN((g_grpc_svc_thread != NULL), SDK_RET_ERR,
                            "grpc svc thread create failure");
    g_grpc_svc_thread->start(g_grpc_svc_thread);

    return SDK_RET_OK;
}

void impl_svc_init (void)
{
    spawn_grpc_svc_thread();
}
