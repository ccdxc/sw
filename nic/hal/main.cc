//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
// WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!
//
// HAL is compiled as both:
//
// 1. a separate executable (aka. process)
// 2. a .so library (running in another process's context)
//
// This file is included in the Makefile target when HAL is compiled as a
// process by itself. When HAL is compiled as shared library,
// hal_init()/hal_wait() are called by application using HAL library and this
// file will not be in the Makefile target for libhal.so
//
// To accomplish this, this file shouldn't contain anything other than main()
// and main() function should have a call to hal_init()/hal_wait(). If code is
// added to the main() function here, please make sure that it is absolutely NOT
// required in the library mode. Anything that is needed for HAL in both library
// mode and standalone binary mode should be put inside hal_init().
//
// WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!
//------------------------------------------------------------------------------

#include <string>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include "nic/hal/hal.hpp"

#include "nic/hal/svc/debug_svc.hpp"
#include "nic/hal/svc/table_svc.hpp"
#include "nic/hal/svc/event_svc.hpp"
#include "nic/hal/svc/system_svc.hpp"
#include "nic/hal/svc/proxy_svc.hpp"
#include "nic/hal/svc/nic_svc.hpp"
#include "nic/hal/svc/hal_ext.hpp"

#include "gen/hal/svc/vrf_svc_gen.hpp"
#include "gen/hal/svc/l2segment_svc_gen.hpp"
#include "gen/hal/svc/nw_svc_gen.hpp"
#include "gen/hal/svc/rdma_svc_gen.hpp"
#include "gen/hal/svc/nvme_svc_gen.hpp"
#include "nic/hal/svc/interface_svc.hpp"
#include "gen/hal/svc/endpoint_svc_gen.hpp"
#include "gen/hal/svc/session_svc_gen.hpp"
#include "gen/hal/svc/telemetry_svc_gen.hpp"
#include "gen/hal/svc/internal_svc_gen.hpp"
#include "gen/hal/svc/nwsec_svc_gen.hpp"
#include "gen/hal/svc/nat_svc_gen.hpp"
#include "gen/hal/svc/qos_svc_gen.hpp"
#include "gen/hal/svc/acl_svc_gen.hpp"
#include "gen/hal/svc/ipsec_svc_gen.hpp"
#include "gen/hal/svc/cpucb_svc_gen.hpp"
#include "gen/hal/svc/tcp_proxy_svc_gen.hpp"
#include "gen/hal/svc/multicast_svc_gen.hpp"
#include "gen/hal/svc/gft_svc_gen.hpp"
#include "gen/hal/svc/l4lb_svc_gen.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

std::string g_grpc_server_addr;
hal::hal_cfg_t    hal::g_hal_cfg;

static void
svc_reg (hal::hal_cfg_t *hal_cfg)
{
    ServerBuilder            *server_builder = (ServerBuilder *)hal_cfg->server_builder;
    VrfServiceImpl           vrf_svc;
    NetworkServiceImpl       nw_svc;
    InterfaceServiceImpl     if_svc;
    InternalServiceImpl      internal_svc;
    L2SegmentServiceImpl     l2seg_svc;
    DebugServiceImpl         debug_svc;
    TableServiceImpl         table_svc;
    NicServiceImpl           nic_svc;
    NatServiceImpl           nat_svc;
    SessionServiceImpl       session_svc;
    EndpointServiceImpl      endpoint_svc;
    NwSecurityServiceImpl    nwsec_svc;
    QOSServiceImpl           qos_svc;
    AclServiceImpl           acl_svc;
    TelemetryServiceImpl     telemetry_svc;
    ProxyServiceImpl         proxy_svc;
    IpsecServiceImpl         ipsec_svc;
    CpuCbServiceImpl         cpucb_svc;
    TcpProxyServiceImpl      tcp_proxy_svc;
    EventServiceImpl         event_svc;
    MulticastServiceImpl     multicast_svc;
    GftServiceImpl           gft_svc;
    SystemServiceImpl        system_svc;
    SoftwarePhvServiceImpl   swphv_svc;
    L4LbServiceImpl          l4lb_svc;
    //DosServiceImpl           dos_svc;

    HAL_TRACE_DEBUG("Bringing gRPC server for all API services ...");
    // register all services
    if (hal_cfg->features == hal::HAL_FEATURE_SET_IRIS) {
        server_builder->RegisterService(&internal_svc);
        server_builder->RegisterService(&debug_svc);
        server_builder->RegisterService(&table_svc);
        server_builder->RegisterService(&nic_svc);
        server_builder->RegisterService(&proxy_svc);
        //server_builder->RegisterService(&ipsec_svc);
        server_builder->RegisterService(&cpucb_svc);
        server_builder->RegisterService(&tcp_proxy_svc);
        server_builder->RegisterService(&event_svc);
        server_builder->RegisterService(&system_svc);
        server_builder->RegisterService(&swphv_svc);
    } else if (hal_cfg->features == hal::HAL_FEATURE_SET_GFT) {
        server_builder->RegisterService(&system_svc);
    }

    HAL_TRACE_DEBUG("gRPC server listening on ... {}",
                    g_grpc_server_addr.c_str());
    hal::utils::hal_logger()->flush();
    HAL_SYSLOG_INFO("HAL-STATUS:UP");

    // notify sysmgr that we are up
    hal::svc::hal_init_done();
    hal::svc::set_hal_status(hal::HAL_STATUS_UP);

    // assemble the server
    std::unique_ptr<Server> server(server_builder->BuildAndStart());

    // wait for server to shutdown (some other thread must be responsible for
    // shutting down the server or else this call won't return)
    server->Wait();
}

// print help message showing usage of HAL
static void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c|--config <cfg.json> [-p|--platform <catalog.json>] \n", argv[0]);
}

// TODO: need to figure a clean way for HAL to exit
// 1. need to release all memory/resources and kill the FTE threads etc.
int
main (int argc, char **argv)
{
    int               oc;
    char              *cfg_file = NULL, *catalog_file = NULL;
    char              *default_config_dir = NULL;
    std::string       ini_file = "hal.ini";
    ServerBuilder     *server_builder;

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

    bzero(&hal::g_hal_cfg, sizeof(hal::g_hal_cfg));
    // parse the HAL config file
    if (hal::hal_parse_cfg(cfg_file, &hal::g_hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL config file parsing failed, quitting ...\n");
        exit(1);
    }

    // listen on the given address (no authentication)
    grpc_init();
    g_grpc_server_addr = std::string("localhost:") + hal::g_hal_cfg.grpc_port;
    hal::g_hal_cfg.server_builder = server_builder = new ServerBuilder();
    server_builder->SetMaxReceiveMessageSize(INT_MAX);
    server_builder->SetMaxSendMessageSize(INT_MAX);
    server_builder->AddListeningPort(g_grpc_server_addr,
                                     grpc::InsecureServerCredentials());

    // set the full path of the catalog file
    if (catalog_file) {
        hal::g_hal_cfg.catalog_file =
            hal::g_hal_cfg.cfg_path + "/" + std::string(catalog_file);

        // make sure catalog file exists
        if (access(hal::g_hal_cfg.catalog_file.c_str(), R_OK) < 0) {
            fprintf(stderr, "Catalog file %s has no read permissions\n",
                    hal::g_hal_cfg.catalog_file.c_str());
            exit(1);
        }
    }

    // TODO: HAL_PBC_INIT_CONFIG will have to go away
    default_config_dir = std::getenv("HAL_PBC_INIT_CONFIG");
    if (default_config_dir) {
        hal::g_hal_cfg.default_config_dir = std::string(default_config_dir);
    } else {
        hal::g_hal_cfg.default_config_dir = std::string("8x25_hbm");
    }

    // parse the ini file, if it exists
    // hal::hal_parse_ini(ini_file.c_str(), &hal::g_hal_cfg);

    // initialize HAL
    if (hal::hal_init(&hal::g_hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        return 1;
    }

    // register for all gRPC services
    svc_reg(&hal::g_hal_cfg);

    // wait for HAL threads to cleanup
    hal::hal_wait();

    return 0;
}
