//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/upgrade/svc/upgrade.hpp"
#include "nic/apollo/upgrade/upgrade.hpp"
#include "nic/apollo/upgrade/logger.hpp"

using grpc::Server;
using grpc::ServerBuilder;

#define GRPC_UPG_PORT       8888
#define UPG_THREAD_ID_EVENT 1
static sdk::event_thread::event_thread *g_upg_event_thread;

namespace upg {

sdk::operd::logger_ptr g_upg_log = sdk::operd::logger::create(UPG_LOG_NAME);

}    // namespace upg

static void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : ");
}

static void
upg_event_handler (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_event_msg_t *event = (upg_event_msg_t *)msg->data();

    printf("Received UPG IPC event stageid %s, status %s, thread %s\n",
           upg_stage2str(event->stage), upg_status2str(event->rsp_status),
           event->rsp_thread_name);
}

void
upg_event_thread_init (void *ctxt)
{
    upg_event_msg_t event;

    // subscribe upgrade client up events
    sdk::ipc::subscribe(UPG_EVENT_ID_RSP, upg_event_handler, NULL);

    // send client ready check
    event.stage = UPG_STAGE_READY;
    sdk::ipc::broadcast(UPG_EVENT_ID_REQ, &event, sizeof(event));
    return;
}

void
upg_event_thread_exit (void *ctxt)
{
    return;
}

//------------------------------------------------------------------------------
// spawn command server thread
//------------------------------------------------------------------------------
sdk_ret_t
spawn_upg_event_thread (void)
{
    // spawn periodic thread that does background tasks
    g_upg_event_thread =
        sdk::event_thread::event_thread::factory(
            "upg", UPG_THREAD_ID_EVENT,
            sdk::lib::THREAD_ROLE_CONTROL, 0x0, upg_event_thread_init,
            upg_event_thread_exit, NULL, // message
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            true);

    if (!g_upg_event_thread) {
        g_upg_log->err("Upgrade event server thread create failure");
        SDK_ASSERT(0);
    }
    g_upg_event_thread->start(g_upg_event_thread);

    return SDK_RET_OK;
}

static void
svc_init (void)
{
    ServerBuilder           *server_builder;
    UpgSvcImpl              upg_svc;
    std::string             g_grpc_server_addr;

    // spawn thread for upgrade event handling
    spawn_upg_event_thread();

    // do gRPC initialization
    grpc_init();
    g_grpc_server_addr =
        std::string("0.0.0.0:") + std::to_string(GRPC_UPG_PORT);
    server_builder = new ServerBuilder();
    server_builder->SetMaxReceiveMessageSize(INT_MAX);
    server_builder->SetMaxSendMessageSize(INT_MAX);
    server_builder->AddListeningPort(g_grpc_server_addr,
                                     grpc::InsecureServerCredentials());

    // register for all the services
    server_builder->RegisterService(&upg_svc);

    g_upg_log->info("gRPC server listening on ... %s",
                    g_grpc_server_addr.c_str());
    std::unique_ptr<Server> server(server_builder->BuildAndStart());
    server->Wait();
}

int
main (int argc, char **argv)
{
    svc_init();
}

