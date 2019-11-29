//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Main entry point for the Pensando Distributed Services Agent (PDSA)
//---------------------------------------------------------------

#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/metaswitch/stubs/mgmt/pdsa_mgmt_init.hpp"
#include "nic/metaswitch/stubs/mgmt/gen/svc/bgp_gen.hpp"

#include <iostream>

using namespace std;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

std::string g_grpc_server_addr;
#define GRPC_API_PORT   50057

static void
svc_reg (void)
{
    ServerBuilder         *server_builder;
    BGPSvcImpl            bgp_svc;

    grpc_init();
    g_grpc_server_addr =
        std::string("0.0.0.0:") + std::to_string(GRPC_API_PORT);
    server_builder = new ServerBuilder();
    server_builder->SetMaxReceiveMessageSize(INT_MAX);
    server_builder->SetMaxSendMessageSize(INT_MAX);
    server_builder->AddListeningPort(g_grpc_server_addr,
                                     grpc::InsecureServerCredentials());

    server_builder->RegisterService(&bgp_svc);

    std::unique_ptr<Server> server(server_builder->BuildAndStart());
    server->Wait();
}

enum {
    THREAD_ID_AGENT_NONE = 0,
    THREAD_ID_NBASE      = 1,
    THREAD_ID_AGENT_MAX  = 2,
};

static sdk::lib::thread *g_nbase_thread;

void *pdsa_nbase_thread_init (void *ctxt)
{
    if (!pdsa_stub_mgmt_init()) {
        SDK_ASSERT("pdsa init failed!");
    }
    return NULL;
}

sdk_ret_t
spawn_nbase_thread (void)
{
    g_nbase_thread =
        sdk::lib::thread::factory(
            "nbase", THREAD_ID_NBASE, sdk::lib::THREAD_ROLE_CONTROL,
            0x0, &pdsa_nbase_thread_init,
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
    svc_reg();

    return 0;
}
