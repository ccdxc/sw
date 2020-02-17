//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Main entry point for the Pensando Distributed Services Agent (PDSA)
//---------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/agent/svc/specs.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/bgp_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/evpn_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_state.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/cp_route_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_interface.hpp"
#include "nic/apollo/agent/svc/interface.hpp"

using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

#define GRPC_API_PORT   50057
static sdk::lib::thread *g_nbase_thread;
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

    SDK_TRACE_INFO("Received gRPC Interface Request Create");

    bctxt = proto_req->batchctxt().batchcookie();
    for (int i = 0; i < proto_req->request_size(); i ++) {

        pds_if_spec_t api_if_spec = {0};
        auto api_spec = &api_if_spec;
        auto request = proto_req->request(i);

        if (request.type() != pds::IF_TYPE_LOOPBACK) {
            SDK_TRACE_ERR("Cannot create non-loopback interfaces on Pegasus");
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
            SDK_TRACE_ERR("Failed to create interface %s, err %d",
                          api_spec->key.str(), ret);
            proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
            return Status::CANCELLED;
        }
    }
    proto_rsp->set_apistatus(sdk_ret_to_api_status(ret));
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

int main(void)
{
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
