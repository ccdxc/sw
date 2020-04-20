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
#include "nic/sdk/upgrade/include/ev.hpp"
#include "nic/apollo/upgrade/svc/upgrade.hpp"

using grpc::Server;
using grpc::ServerBuilder;

static sdk::event_thread::event_thread *g_upg_event_thread;
static std::string g_tools_dir;

namespace sdk {
namespace upg {

sdk::operd::logger_ptr g_upg_log = sdk::operd::logger::create(UPGRADE_LOG_NAME);

}   // namespace upg
}   // namespace sdk

// fsm completion handler
void
fsm_completion_hdlr (upg_status_t status, sdk::ipc::ipc_msg_ptr msg_in)
{
    if (msg_in) {
        sdk::ipc::respond(msg_in, &status, sizeof(status));
    }

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

static void
upg_fsm_init(upg_mode_t mode, sdk::ipc::ipc_msg_ptr msg,
             upg_stage_t entry_stage, std::string fw_pkgname)
{
    sdk::upg::fsm_init_params_t params;
    sdk_ret_t ret;

    memset(&params, 0, sizeof(params));
    params.upg_mode = mode;
    params.ev_loop = g_upg_event_thread->ev_loop();
    params.fsm_completion_cb = fsm_completion_hdlr;
    params.msg_in = msg;
    params.entry_stage = entry_stage;
    params.fw_pkgname = fw_pkgname;
    params.tools_dir = g_tools_dir;

    ret = sdk::upg::init(&params);
    // returns here only in failure
    if (ret != SDK_RET_OK) {
        fsm_completion_hdlr(UPG_STATUS_FAIL, msg);
    }
    SDK_ASSERT(ret == SDK_RET_OK);
}

// request from the grpc main thread to processing thread
static void
upg_ev_req_handler (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_ev_req_msg_t *req = (upg_ev_req_msg_t *)msg->data();

    if (req->id == UPG_REQ_MSG_ID_START) {
        upg_fsm_init(req->upg_mode, msg, UPG_STAGE_COMPAT_CHECK,
                     req->fw_pkgname);
    } else {
        UPG_TRACE_ERR("Unknown request id %u", req->id);
        fsm_completion_hdlr(UPG_STATUS_FAIL, msg);
    }
}

void
upg_event_thread_init (void *ctxt)
{
    upg_mode_t mode = sdk::upg::upg_init_mode();

    // if it is an graceful upgrade restart, need to continue the stages
    // from previous run
    if (sdk::platform::upgrade_mode_graceful(mode)) {
        upg_fsm_init(mode, NULL, UPG_STAGE_READY, NULL);
    } else {
        // register for upgrade request from grpc thread
        sdk::ipc::reg_request_handler(UPG_REQ_MSG_ID_START,
                                      upg_ev_req_handler, NULL);
    }
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

static void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -t <tools-directory>\n", argv[0]);
}


int
main (int argc, char **argv)
{
    int oc;
    struct option longopts[] = {
        { "tools-dir",       required_argument, NULL, 't' },
        { "help",            no_argument,       NULL, 'h' },
        { 0,                 0,                 0,     0 }
    };

    while ((oc = getopt_long(argc, argv, ":ht:;", longopts, NULL)) != -1) {
        switch (oc) {
        case 't':
            if (optarg) {
                g_tools_dir = std::string(optarg);
            } else {
                fprintf(stderr, "tools directory is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;
        case 'h':
        default:
            print_usage(argv);
            exit(0);
            break;
        }
    }
    if (g_tools_dir.empty()) {
        fprintf(stderr, "tools directory is not specified\n");
        exit(1);
    }
    grpc_svc_init();
}

