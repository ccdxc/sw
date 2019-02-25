//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include <string>
#include "svc/batch.hpp"
#include "svc/switch.hpp"
#include "svc/pcn.hpp"
#include "svc/subnet.hpp"
#include "svc/tunnel.hpp"
#include "svc/route.hpp"
#include "svc/vnic.hpp"
#include "svc/mapping.hpp"

using std::string;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

std::string g_grpc_server_addr;
#define GRPC_API_PORT    50054

static void
svc_reg (void)
{
    ServerBuilder     *server_builder;
    BatchSvcImpl      batch_svc;
    SwitchSvcImpl     switch_svc;
    PCNSvcImpl        pcn_svc;
    SubnetSvcImpl     subnet_svc;
    TunnelSvcImpl     tunnel_svc;
    RouteSvcImpl      route_svc;
    VnicSvcImpl       vnic_svc;
    MappingSvcImpl    mapping_svc;


    // do gRPC initialization
    grpc_init();
    g_grpc_server_addr = std::string("0.0.0.0:") + std::to_string(GRPC_API_PORT);
    server_builder = new ServerBuilder();
    server_builder->SetMaxReceiveMessageSize(INT_MAX);
    server_builder->SetMaxSendMessageSize(INT_MAX);
    server_builder->AddListeningPort(g_grpc_server_addr,
                                     grpc::InsecureServerCredentials());

    // register for all the services
    server_builder->RegisterService(&batch_svc);
    server_builder->RegisterService(&switch_svc);
    server_builder->RegisterService(&pcn_svc);
    server_builder->RegisterService(&subnet_svc);
    server_builder->RegisterService(&tunnel_svc);
    server_builder->RegisterService(&route_svc);
    server_builder->RegisterService(&vnic_svc);
    server_builder->RegisterService(&mapping_svc);

    std::unique_ptr<Server> server(server_builder->BuildAndStart());
    server->Wait();
}

static void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c|--config <cfg.json> [-p|--platform <catalog.json>] \n", argv[0]);
}

int
main (int argc, char **argv)
{
    int       oc;
    char      *cfg_file = NULL, *catalog_file = NULL;
    char      *cfg_path;
    string    cfg_path_str, catalog_file_str;

    struct option longopts[] = {
       { "config",    required_argument, NULL, 'c' },
       { "platform",  optional_argument, NULL, 'p' },
       { "help",      no_argument,       NULL, 'h' },
       { 0,           0,                 0,     0 }
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hc:p:W;", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            cfg_file = optarg;
            if (!cfg_file) {
                fprintf(stderr, "config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'p':
            if (optarg) {
                catalog_file = optarg;
            } else {
                fprintf(stderr, "platform catalog file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'h':
            print_usage(argv);
            exit(0);
            break;

        case ':':
            fprintf(stderr, "%s: option -%c requires an argument\n",
                    argv[0], optopt);
            print_usage(argv);
            exit(1);
            break;

        case '?':
        default:
            fprintf(stderr, "%s: option -%c is invalid, quitting ...\n",
                    argv[0], optopt);
            print_usage(argv);
            exit(1);
            break;
        }
    }

    // set the full path of the catalog file
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        cfg_path_str = std::string(cfg_path) + "/";
    } else {
        cfg_path_str = "./";
    }
    if (catalog_file) {
       catalog_file_str = cfg_path_str + "/" + std::string(catalog_file);
    } else {
        catalog_file_str = cfg_path_str + "/catalog.json";
    }
    // make sure catalog file exists
    if (access(catalog_file_str.c_str(), R_OK) < 0) {
        fprintf(stderr, "Catalog file %s has no read permissions\n",
                catalog_file_str.c_str());
        exit(1);
    }

    // register for all gRPC services
    svc_reg();

    // wait forver
    printf("Initialization done ...");
    while (1);

    return 0;
}
