//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Main entry point for the Pensando Distributed Services Agent (PDSA)
//---------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/bgp_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/evpn_gen.hpp"
#include "nic/metaswitch/stubs/mgmt/pds_ms_mgmt_init.hpp"

using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

#define GRPC_API_PORT   50057
static sdk::lib::thread *g_nbase_thread;
std::string g_grpc_server_addr;

static void
svc_reg (void)
{
    ServerBuilder         *server_builder;
    BGPSvcImpl            bgp_svc;
    EvpnSvcImpl           evpn_svc;

    grpc_init();
    g_grpc_server_addr =
        std::string("0.0.0.0:") + std::to_string(GRPC_API_PORT);
    server_builder = new ServerBuilder();
    server_builder->SetMaxReceiveMessageSize(INT_MAX);
    server_builder->SetMaxSendMessageSize(INT_MAX);
    server_builder->AddListeningPort(g_grpc_server_addr,
                                     grpc::InsecureServerCredentials());

    server_builder->RegisterService(&bgp_svc);
    server_builder->RegisterService(&evpn_svc);

    std::unique_ptr<Server> server(server_builder->BuildAndStart());
    server->Wait();
}

void *pds_ms_nbase_thread_init (void *ctxt)
{
    // opting for graceful termination
    SDK_THREAD_DFRD_TERM_INIT(ctxt);

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
    svc_reg();

    return 0;
}
