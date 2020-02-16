//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include "nic/apollo/upgrade/svc/upgrade.hpp"
#include "nic/apollo/upgrade/logger.hpp"

using grpc::Server;
using grpc::ServerBuilder;

#define GRPC_UPG_PORT    8888

namespace upg {

sdk::operd::logger_ptr g_upg_log = sdk::operd::logger::create(UPG_LOG_NAME);

}    // namespace upg

static void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : ");
}

static void
svc_init (void)
{
    ServerBuilder           *server_builder;
    UpgSvcImpl              upg_svc;
    std::string             g_grpc_server_addr;

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

