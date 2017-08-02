//------------------------------------------------------------------------------
// grpc service registration across all agents
//------------------------------------------------------------------------------
#include <iostream>
#include <memory>
#include <string>
#include <grpc++/grpc++.h>
#include "net.grpc.pb.h"
#include "svc_reg.hpp"
#include "nw_svc.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using net::VlanRequest;
using net::VlanResponse;
using net::Net;

void
RegistergRPCServers (const std::string& server_addr)
{
    NetServiceImpl            nw_svc;
    ServerBuilder             server_builder;

    // listen on the given address (no authentication)
    server_builder.AddListeningPort(server_addr,
                                    grpc::InsecureServerCredentials());

    // register all services
    server_builder.RegisterService(&nw_svc);

    // assemble the server
    std::unique_ptr<Server> server(server_builder.BuildAndStart());
    std::cout << "grpc Servers listening on ..." << server_addr << std::endl;

    // wait for server to shutdown (some other thread must be responsible for
    // shutting down the server or else this call won't return)
    server->Wait();
}
