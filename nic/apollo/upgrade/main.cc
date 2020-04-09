//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/include/globals.hpp"
#include "nic/sdk/upgrade/core/fsm.hpp"
#include "nic/apollo/upgrade/svc/upgrade.hpp"

using grpc::Server;
using grpc::ServerBuilder;

static sdk::event_thread::event_thread *g_upg_event_thread;

// TODO: need to chnange when sysmgr change is ready
// static upg_stage_t fsm_entry_stage = UPG_STAGE_NONE;
 static upg_stage_t fsm_entry_stage = UPG_STAGE_COMPAT_CHECK;
//static upg_stage_t fsm_entry_stage = UPG_STAGE_READY;

namespace sdk {
namespace upg {

sdk::operd::logger_ptr g_upg_log = sdk::operd::logger::create(UPGRADE_LOG_NAME);

}   // namespace upg
}   // namespace sdk

// fsm completion handler
void
fsm_completion_hdlr (upg_status_t status, sdk::ipc::ipc_msg_ptr msg_in)
{
    sdk::ipc::respond(msg_in, &status, sizeof(status));

    if (status == UPG_STATUS_OK) {
        UPG_TRACE_INFO("Upgrade finished successfully");
    } else {
        UPG_TRACE_ERR("Upgrade failed !!");
    }
    // upgmgr not expecting any return from here. sysmgr will kill
    // this process as the upgrade stages are done now
    while (1) {
        sleep(1);
    }
}

// request from the grpc main thread to processing thread
static void
upg_ev_req_handler (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_ev_req_msg_t *req = (upg_ev_req_msg_t *)msg->data();
    sdk_ret_t ret;
    sdk::upg::fsm_init_params_t params;

    memset(&params, 0, sizeof(params));
    params.upg_mode = req->upg_mode;
    params.ev_loop = g_upg_event_thread->ev_loop();
    params.fsm_completion_cb = fsm_completion_hdlr;
    params.msg_in = msg;
    params.entry_stage = fsm_entry_stage;

    if (req->id == UPG_REQ_MSG_ID_START) {
        ret = sdk::upg::init(&params);
    } else {
        UPG_TRACE_ERR("Unknown request id %u", req->id);
        ret = SDK_RET_ERR;
    }
    if (ret != SDK_RET_OK) {
        fsm_completion_hdlr(UPG_STATUS_FAIL, msg);
    }
}

void
upg_event_thread_init (void *ctxt)
{
    // register for upgrade request from grpc thread
    sdk::ipc::reg_request_handler(UPG_REQ_MSG_ID_START, upg_ev_req_handler, NULL);
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
            "upg", SDK_IPC_ID_UPGMGR,
            sdk::lib::THREAD_ROLE_CONTROL, 0x0, upg_event_thread_init,
            upg_event_thread_exit, NULL, // message
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            true);

    if (!g_upg_event_thread) {
        UPG_TRACE_ERR("Upgrade event server thread create failure");
        SDK_ASSERT(0);
    }
    g_upg_event_thread->start(g_upg_event_thread);

    return SDK_RET_OK;
}

static void
grpc_svc_init (void)
{
    ServerBuilder           *server_builder;
    UpgSvcImpl              upg_svc;
    std::string             g_grpc_server_addr;

    // spawn thread for upgrade event handling
    spawn_upg_event_thread();

    // do gRPC initialization
    grpc_init();
    g_grpc_server_addr =
        std::string("0.0.0.0:") + std::to_string(PDS_GRPC_PORT_UPGMGR);
    server_builder = new ServerBuilder();
    server_builder->SetMaxReceiveMessageSize(INT_MAX);
    server_builder->SetMaxSendMessageSize(INT_MAX);
    server_builder->AddListeningPort(g_grpc_server_addr,
                                     grpc::InsecureServerCredentials());

    // register for all the services
    server_builder->RegisterService(&upg_svc);

    UPG_TRACE_INFO("gRPC server listening on ... %s",
                    g_grpc_server_addr.c_str());
    std::unique_ptr<Server> server(server_builder->BuildAndStart());
    server->Wait();
}

int
main (int argc, char **argv)
{
    grpc_svc_init();
}

