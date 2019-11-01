//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Main entry point for the Pensando Distributed Services Agent (PDSA)
//---------------------------------------------------------------

#include <iostream>
#include "pdsa_state_init.hpp"
#include "pdsa_hal_init.hpp"
#include "pdsa_mgmt_init.hpp"

#include "nic/metaswitch/svc/bgp.hpp"

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
    return; //TODO: remove this later
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


int main ()
{
#ifdef NAPLES_BUILD
    if(!pdsa_stub::state_init()) {
        return -1;
    }
    if(!pdsa_stub::hal_init()) {
        goto error;
    }
#elif VENICE_BUILD
//  pdsa_stub_hal_dummy_init();
#endif
    if(!pdsa_stub_mgmt_init()) {
        goto error;
    }
    svc_reg();
    return 0;

error:
#ifdef NAPLES_BUILD
    pdsa_stub::state_deinit();
    pdsa_stub::hal_deinit();
#endif
    return -1;
}
