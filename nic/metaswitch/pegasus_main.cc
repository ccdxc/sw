//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Main entry point for the Pensando Distributed Services Agent (PDSA)
//---------------------------------------------------------------

#include <signal.h>
#include <iostream>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/internal_bgp_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/internal_evpn_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/internal_lim_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/internal_cp_route_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_interface.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_utils.hpp"
#include "nic/metaswitch/pegasus_trace.hpp"
#include "nic/apollo/agent/svc/interface.hpp"

using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// number of trace files to keep
#define TRACE_NUM_FILES                        10
#define TRACE_FILE_SIZE                        (100 << 20)
#define GRPC_API_PORT   50057
static sdk::lib::thread *g_nbase_thread;
sdk_logger::trace_cb_t   g_trace_cb;
std::string g_grpc_server_addr;

// Pegasus needs to implement the Interface proto for loopback interfaces.
// Metaswitch requires RRs to have a route reachability to the
// BGP NLRI nexthops foe the RR to accept and reflect them.
// This is achieved by creating a static default route pointing to a
// loopback interface.
Status
IfSvcImpl::InterfaceCreate(ServerContext *context,
                           const pds::InterfaceRequest *proto_req,
                           pds::InterfaceResponse *proto_rsp) {
    if ((proto_req == NULL) || (proto_req->request_size() == 0)) {
        proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_INVALID_ARG);
        return Status::CANCELLED;
    }

    sdk_ret_t ret;
    pds_batch_ctxt_t bctxt;

    PDS_TRACE_INFO("Received gRPC Interface Request Create");

    bctxt = proto_req->batchctxt().batchcookie();
    for (int i = 0; i < proto_req->request_size(); i ++) {

        pds_if_spec_t api_if_spec = {0};
        auto api_spec = &api_if_spec;
        auto request = proto_req->request(i);

        if (request.type() != pds::IF_TYPE_LOOPBACK) {
            PDS_TRACE_ERR("Cannot create non-loopback interfaces on Pegasus");
            proto_rsp->set_apistatus(
                types::ApiStatus::API_STATUS_OPERATION_NOT_ALLOWED);
            return Status::CANCELLED;
        }
        memcpy(api_spec->key.id, request.id().data(),
               MIN(request.id().length(), PDS_MAX_KEY_LEN));

        api_spec->admin_state = PDS_IF_STATE_UP;
        api_spec->type = PDS_IF_TYPE_LOOPBACK;

        // call the metaswitch api
        if ((ret = pds_ms::interface_create(api_spec, bctxt)) != SDK_RET_OK) {
            PDS_TRACE_ERR("Failed to create interface %s, err %d",
                          api_spec->key.str(), ret);
            proto_rsp->set_apistatus(pds_ms_sdk_ret_to_api_status(ret));
            return Status::CANCELLED;
        }
    }
    proto_rsp->set_apistatus(pds_ms_sdk_ret_to_api_status(ret));
    return Status::OK;
}

Status
IfSvcImpl::InterfaceUpdate(ServerContext *context,
                           const pds::InterfaceRequest *proto_req,
                           pds::InterfaceResponse *proto_rsp) {
    proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OPERATION_NOT_ALLOWED);
    return Status::CANCELLED;
}

Status
IfSvcImpl::InterfaceDelete(ServerContext *context,
                           const pds::InterfaceDeleteRequest *proto_req,
                           pds::InterfaceDeleteResponse *proto_rsp) {
    proto_rsp->add_apistatus(types::ApiStatus::API_STATUS_OPERATION_NOT_ALLOWED);
    return Status::CANCELLED;
}

Status
IfSvcImpl::InterfaceGet(ServerContext *context,
                        const pds::InterfaceGetRequest *proto_req,
                        pds::InterfaceGetResponse *proto_rsp) {
    proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
    return Status::OK;
}

Status
IfSvcImpl::LifGet(ServerContext *context,
                  const pds::LifGetRequest *proto_req,
                  pds::LifGetResponse *proto_rsp) {
    proto_rsp->set_apistatus(types::ApiStatus::API_STATUS_OK);
    return Status::OK;
}

static void
svc_reg (void)
{
    ServerBuilder         *server_builder;
    BGPSvcImpl            bgp_svc;
    EvpnSvcImpl           evpn_svc;
    IfSvcImpl             intf_svc;
    LimSvcImpl            lim_svc;
    CPRouteSvcImpl        route_svc;


    grpc_init();
    g_grpc_server_addr =
        std::string("0.0.0.0:") + std::to_string(GRPC_API_PORT);
    server_builder = new ServerBuilder();
    server_builder->SetMaxReceiveMessageSize(INT_MAX);
    server_builder->SetMaxSendMessageSize(INT_MAX);
    server_builder->AddListeningPort(g_grpc_server_addr,
                                     grpc::InsecureServerCredentials());

    server_builder->RegisterService(&intf_svc);
    server_builder->RegisterService(&route_svc);
    server_builder->RegisterService(&bgp_svc);
    server_builder->RegisterService(&evpn_svc);
    server_builder->RegisterService(&lim_svc);

    std::unique_ptr<Server> server(server_builder->BuildAndStart());
    server->Wait();
}

void *pds_ms_nbase_thread_init (void *ctxt)
{
    // opting for graceful termination
    SDK_THREAD_INIT(ctxt);

    if (!pds_ms_mgmt_init()) {
        SDK_ASSERT("pdsa init failed!");
    }

    return NULL;
}

sdk_ret_t
spawn_nbase_thread (void)
{
    g_nbase_thread =
        sdk::lib::thread::factory(
            "nbase", 1, sdk::lib::THREAD_ROLE_CONTROL,
            0x0, &pds_ms_nbase_thread_init,
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            false);

    SDK_ASSERT_TRACE_RETURN((g_nbase_thread != NULL), SDK_RET_ERR,
                            "n-base thread create failure");
    g_nbase_thread->start(g_nbase_thread);

    return SDK_RET_OK;
}

static inline string
log_file (const char *logdir, const char *logfile)
{
    struct stat st = { 0 };

    if (!logdir) {
        return std::string(logfile);
    }

    // check if this log dir exists
    if (stat(logdir, &st) == -1) {
        // doesn't exist, try to create
        if (mkdir(logdir, 0755) < 0) {
            fprintf(stderr,
                    "Log directory %s/ doesn't exist, failed to create "
                    "one\n", logdir);
            return std::string("");
        }
    } else {
        // log dir exists, check if we have write permissions
        if (access(logdir, W_OK) < 0) {
            // don't have permissions to create this directory
            fprintf(stderr,
                    "No permissions to create log file in %s/\n",
                    logdir);
            return std::string("");
        }
    }
    return logdir + std::string(logfile);
}

//------------------------------------------------------------------------------
// initialize the logger
//------------------------------------------------------------------------------
static inline sdk_ret_t
logger_init (void)
{
    std::string logfile, err_logfile;

    logfile = log_file(std::getenv("LOG_DIR"), "./pegasus.log");
    err_logfile = log_file(std::getenv("LOG_DIR"), "./pegasus-err.log");

    if (logfile.empty() || err_logfile.empty()) {
        return SDK_RET_ERR;
    }

    // initialize the logger
    core::trace_init("pegasus", 0x1, true, err_logfile.c_str(), logfile.c_str(),
                     TRACE_FILE_SIZE, TRACE_NUM_FILES, utils::trace_debug);

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// logger callback passed to SDK and PDS lib
//------------------------------------------------------------------------------
static int
pegasus_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    switch (tracel_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        PEGASUS_TRACE_ERR_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        PEGASUS_TRACE_WARN_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        PEGASUS_TRACE_INFO_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
        PEGASUS_TRACE_DEBUG_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
        PEGASUS_TRACE_VERBOSE_NO_META("{}", logbuf);
        break;
    default:
        break;
    }
    va_end(args);

    return 0;
}

int main(void)
{
    // initialize the logger instance
    logger_init();
    register_trace_cb(pegasus_logger);
    auto ret = spawn_nbase_thread();
    if (ret != SDK_RET_OK) {
        return ret;
    }
    // Wait for nbase to be ready
    while (!g_nbase_thread->ready()) {
        pthread_yield();
    }
    // set rr_mode
    {
        auto mgmt_ctxt = pds_ms::mgmt_state_t::thread_context();
        mgmt_ctxt.state()->set_rr_mode(true);
    }
    svc_reg();

    return 0;
}
