//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <algorithm>
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/include/globals.hpp"
#include "nic/sdk/upgrade/core/fsm.hpp"
#include "nic/sdk/upgrade/include/ev.hpp"
#include "nic/apollo/upgrade/svc/upgrade.hpp"
#include "nic/apollo/upgrade/ipc_peer/ipc_peer.hpp"

using grpc::Server;
using grpc::ServerBuilder;

static sdk::event_thread::event_thread *g_upg_event_thread;
static std::string g_tools_dir;
static ipc_peer_ctx *g_ipc_peer_ctx;
static sdk::ipc::ipc_msg_ptr g_ipc_msg_in_ptr;
// for interactive stage handling
static upg_event_msg_t g_upg_event_msg_in;

#define UPGRADE_IPC_PEER_SOCK_NAME "/tmp/upgrade_ipc_peer.sock"

namespace sdk {
namespace upg {

sdk::operd::logger_ptr g_upg_log = sdk::operd::logger::create(UPGRADE_LOG_NAME);
const char *g_upg_log_pfx;

}   // namespace upg
}   // namespace sdk

// fsm completion handler
static void
upg_fsm_completion_hdlr (upg_status_t status)
{
    if (g_ipc_msg_in_ptr) {
        sdk::ipc::respond(g_ipc_msg_in_ptr, &status, sizeof(status));
    }

    if (status == UPG_STATUS_OK) {
        UPG_TRACE_INFO("Upgrade finished successfully");
    } else {
        UPG_TRACE_ERR("Upgrade failed !!");
    }
    // this process is no more needed as the upgrade stages are done now
    sleep(5);
    exit(0);
}

static void
upg_ipc_peer_error_hdlr (ipc_peer_ctx *ctx)
{
    UPG_TRACE_INFO("Upgrade peer commands done");
    // TODO look the status and decide the upgrade ok/fail
    sleep(5);
    exit(0);
}

static void
upg_interactive_fsm_completion_hdlr (upg_status_t status)
{
    UPG_TRACE_INFO("Hitless interactive stage completed, status %u",
                   status);
    g_ipc_peer_ctx->reply(&status, sizeof(status));
}


// request from A to B during hitless upgrade
// execute the particular request and send back the response
static void
upg_interactive_request (const void *data, const size_t size)
{
    upg_stage_t stage = *(upg_stage_t *)data;
    sdk_ret_t ret;

    if (size != sizeof(upg_stage_t)) {
        UPG_TRACE_ERR("Invalid request, size %lu, expected size %lu",
                      size, sizeof(upg_stage_t));
        return;
    }
    UPG_TRACE_DEBUG("Hitless interactive request stage %s",
                    upg_stage2str(stage));
    ret = sdk::upg::upg_interactive_stage_exec(stage);
    if (ret != SDK_RET_IN_PROGRESS) {
        upg_interactive_fsm_completion_hdlr(UPG_STATUS_FAIL);
    }
}


// response from B to A during hitless upgrade
// status of the particular request sent by A
static void
upg_interactive_response (const void *data, const size_t size)
{
    upg_event_msg_t *msg_in = &g_upg_event_msg_in;

    if (size != sizeof(upg_status_t)) {
        UPG_TRACE_ERR("Invalid request, size %lu, expected size %lu",
                      size,  sizeof(upg_status_t));
        return;
    }
    msg_in->rsp_status = *(upg_status_t *)data;
    UPG_TRACE_INFO("Hitless interactive response stage %s, status %u",
                   upg_stage2str(msg_in->stage), msg_in->rsp_status);
    // respond to upgmgr, send it as an ipc event it is registerd for
    sdk::upg::upg_event_handler(msg_in);
}

static sdk_ret_t
upg_peer_init (bool client)
{
    sdk_ret_t ret;

    g_ipc_peer_ctx = ipc_peer_ctx::factory(UPGRADE_IPC_PEER_SOCK_NAME,
                                           g_upg_event_thread->ev_loop());
    if (!g_ipc_peer_ctx) {
        UPG_TRACE_ERR("Peer IPC create failed");
        return SDK_RET_OOM;
    }
    if (!client) {
        UPG_TRACE_INFO("Opening a listening socket");
        ret = g_ipc_peer_ctx->listen();
        if (ret != SDK_RET_OK) {
            UPG_TRACE_ERR("Peer IPC listen failed");
            goto err_exit;
        }
        g_ipc_peer_ctx->recv_cb = upg_interactive_request;
    } else {
        UPG_TRACE_INFO("Connecting to the peer");
        ret = g_ipc_peer_ctx->connect();
        if (ret != SDK_RET_OK) {
            UPG_TRACE_ERR("Peer IPC connection failed");
            goto err_exit;
        }
        g_ipc_peer_ctx->recv_cb = upg_interactive_response;
    }
    g_ipc_peer_ctx->err_cb = upg_ipc_peer_error_hdlr;
    return SDK_RET_OK;

err_exit:
    ipc_peer_ctx::destroy(g_ipc_peer_ctx);
    g_ipc_peer_ctx = NULL;
    return ret;
}

// called by A to send request to B during A to B hitless upgrade
static void
upg_event_send_to_peer_hdlr (upg_stage_t stage, std::string svc_name,
                             uint32_t svc_id)
{
    static bool connected = false;
    sdk_ret_t ret;
    upg_event_msg_t *msg = &g_upg_event_msg_in;

    UPG_TRACE_INFO("Request peer notification stage %s, svcname %s, svcid %u",
                   upg_stage2str(stage), svc_name.c_str(), svc_id);
    // new process should be booted up now and also upgmgr
    // is ready to recieve
    memset(msg, 0, sizeof(*msg));
    // save this information for replying later. as there is only one
    // outstanding request, we can save it in a global variable
    msg->stage = stage;
    msg->rsp_svc_ipc_id = svc_id;
    strncpy(msg->rsp_svc_name, svc_name.c_str(), sizeof(msg->rsp_svc_name));

    if (!connected) {
        ret = upg_peer_init(true);
        if (ret != SDK_RET_OK) {
            upg_status_t status = UPG_STATUS_FAIL;
            upg_interactive_response(&status, sizeof(status));
        }
        connected = true;
    }
      // send the stage to be executed
    g_ipc_peer_ctx->send((void *)&stage, sizeof(stage));
}

static void
upg_fsm_init (upg_mode_t mode, upg_stage_t entry_stage, std::string fw_pkgname,
              bool upg_request_new)
{
    sdk::upg::fsm_init_params_t params;
    sdk_ret_t ret;

    UPG_TRACE_INFO("FSM init mode %s entry-stage %s new-request %u",
                   sdk::platform::upgrade_mode_hitless(mode) ? "hitless" : "graceful",
                   upg_stage2str(entry_stage), upg_request_new);

    memset(&params, 0, sizeof(params));
    params.upg_mode = mode;
    params.ev_loop = g_upg_event_thread->ev_loop();
    params.fsm_completion_cb = upg_fsm_completion_hdlr;

    params.entry_stage = entry_stage;
    params.fw_pkgname = fw_pkgname;
    params.tools_dir = g_tools_dir;

    // register for peer communication in hitless boot
    if (sdk::platform::upgrade_mode_hitless(mode)) {
        // if it is a new upgrade request
        if (upg_request_new) {
            params.upg_event_fwd_cb = upg_event_send_to_peer_hdlr;
        } else {
            params.interactive_mode = true;
            params.fsm_completion_cb = upg_interactive_fsm_completion_hdlr;
        }
    }

    ret = sdk::upg::init(&params);
    // returns here only in failure
    if (ret != SDK_RET_OK) {
        params.fsm_completion_cb(UPG_STATUS_FAIL);
    }
    SDK_ASSERT(ret == SDK_RET_OK);
}

// request from the grpc main thread to processing thread
static void
upg_ev_request_hdlr (sdk::ipc::ipc_msg_ptr msg, const void *ctxt)
{
    upg_ev_req_msg_t *req = (upg_ev_req_msg_t *)msg->data();
    g_ipc_msg_in_ptr = msg;

    if (req->id == UPG_REQ_MSG_ID_START) {
        upg_fsm_init(req->upg_mode, UPG_STAGE_COMPAT_CHECK,
                     req->fw_pkgname, true);
    } else {
        UPG_TRACE_ERR("Unknown request id %u", req->id);
        upg_fsm_completion_hdlr(UPG_STATUS_FAIL);
    }
}

static void
upg_event_thread_init (void *ctxt)
{
    upg_mode_t mode = sdk::upg::upg_init_mode();

    // if it is an graceful upgrade restart, need to continue the stages
    // from previous run
    if (sdk::platform::upgrade_mode_graceful(mode)) {
        upg_fsm_init(mode, UPG_STAGE_READY, "none", false);
    } else if (sdk::platform::upgrade_mode_hitless(mode)) {
        g_upg_log_pfx = "peer";
        // spawn for a hitless upgrade
        upg_peer_init(false);
        upg_fsm_init(mode, UPG_STAGE_NONE, "none", false);
    } else {
        // register for upgrade request from grpc thread
        sdk::ipc::reg_request_handler(UPG_REQ_MSG_ID_START,
                                      upg_ev_request_hdlr, NULL);
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
    ServerBuilder *server_builder;
    UpgSvcImpl upg_svc;
    std::string g_grpc_server_addr;
    upg_mode_t mode = sdk::upg::upg_init_mode();

    // spawn thread for upgrade event handling
    spawn_upg_event_thread();

    // register for grpc only if it is fresh upgrade
    if (!sdk::platform::upgrade_mode_none(mode)) {
        while (1) {
            sleep(10);
        }
    }

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
    bool dir_given = false;
    struct option longopts[] = {
        { "tools-dir",       required_argument, NULL, 't' },
        { "help",            no_argument,       NULL, 'h' },
        { 0,                 0,                 0,     0 }
    };

    while ((oc = getopt_long(argc, argv, ":ht:;", longopts, NULL)) != -1) {
        switch (oc) {
        case 't':
            if (optarg) {
                dir_given = true;
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
    if (!dir_given) {
        fprintf(stderr, "tools directory is not specified\n");
        exit(1);
    }
    grpc_svc_init();
}

