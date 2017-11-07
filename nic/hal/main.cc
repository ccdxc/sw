//------------------------------------------------------------------------------
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
#include "nic/hal/hal.hpp"
#include "nic/hal/svc/tenant_svc.hpp"
#include "nic/hal/svc/network_svc.hpp"
#include "nic/hal/svc/interface_svc.hpp"
#include "nic/hal/svc/l2segment_svc.hpp"
#include "nic/hal/svc/port_svc.hpp"
#include "nic/hal/svc/debug_svc.hpp"
#include "nic/hal/svc/internal_svc.hpp"
#include "nic/hal/svc/rdma_svc.hpp"
#include "nic/hal/svc/endpoint_svc.hpp"
#include "nic/hal/svc/session_svc.hpp"
#include "nic/hal/svc/l4lb_svc.hpp"
#include "nic/hal/svc/nwsec_svc.hpp"
#include "nic/hal/svc/tlscb_svc.hpp"
#include "nic/hal/svc/tcpcb_svc.hpp"
#include "nic/hal/svc/qos_svc.hpp"
#include "nic/hal/svc/descriptor_aol_svc.hpp"
#include "nic/hal/svc/wring_svc.hpp"
#include "nic/hal/svc/proxy_svc.hpp"
#include "nic/hal/svc/acl_svc.hpp"
#include "nic/hal/svc/telemetry_svc.hpp"
#include "nic/hal/svc/ipseccb_svc.hpp"
#include "nic/hal/svc/cpucb_svc.hpp"
#include "nic/hal/svc/crypto_keys_svc.hpp"
#include "nic/hal/lkl/lkl_api.hpp"
#include "nic/hal/svc/rawrcb_svc.hpp"
#include "nic/hal/svc/rawccb_svc.hpp"
#include "nic/hal/svc/crypto_apis_svc.hpp"
#include "nic/hal/svc/event_svc.hpp"
#include "nic/hal/svc/quiesce_svc.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// TODO - port should come from some global cfg file or
//        command line argument
const std::string&    hal_svc_endpoint_("localhost:50052");

void
svc_reg (const std::string& server_addr)
{
    TenantServiceImpl        tenant_svc;
    NetworkServiceImpl       network_svc;
    InterfaceServiceImpl     if_svc;
    InternalServiceImpl      internal_svc;
    RdmaServiceImpl          rdma_svc;
    L2SegmentServiceImpl     l2seg_svc;
    PortServiceImpl          port_svc;
    DebugServiceImpl         debug_svc;
    SessionServiceImpl       session_svc;
    EndpointServiceImpl      endpoint_svc;
    L4LbServiceImpl          l4lb_svc;
    NwSecurityServiceImpl    nwsec_svc;
    QOSServiceImpl           qos_svc;
    AclServiceImpl           acl_svc;
    TelemetryServiceImpl     telemetry_svc;
    ServerBuilder            server_builder;
    TlsCbServiceImpl         tlscb_svc;
    TcpCbServiceImpl         tcpcb_svc;
    DescrAolServiceImpl      descraol_svc;
    WRingServiceImpl         wring_svc;
    ProxyServiceImpl         proxy_svc;
    IpsecCbServiceImpl       ipseccb_svc;
    CpuCbServiceImpl         cpucb_svc;
    CryptoKeyServiceImpl     crypto_key_svc; 
    RawrCbServiceImpl        rawrcb_svc;
    RawcCbServiceImpl        rawccb_svc;
    CryptoApisServiceImpl    crypto_apis_svc;
    EventServiceImpl         event_svc;
    QuiesceServiceImpl       quiesce_svc;

    HAL_TRACE_DEBUG("Bringing gRPC server for all API services ...");

    // listen on the given address (no authentication)
    server_builder.AddListeningPort(server_addr,
                                    grpc::InsecureServerCredentials());

    // register all services
    server_builder.RegisterService(&tenant_svc);
    server_builder.RegisterService(&network_svc);
    server_builder.RegisterService(&if_svc);
    server_builder.RegisterService(&internal_svc);
    server_builder.RegisterService(&rdma_svc);
    server_builder.RegisterService(&l2seg_svc);
    server_builder.RegisterService(&port_svc);
    server_builder.RegisterService(&debug_svc);
    server_builder.RegisterService(&session_svc);
    server_builder.RegisterService(&endpoint_svc);
    server_builder.RegisterService(&l4lb_svc);
    server_builder.RegisterService(&nwsec_svc);
    server_builder.RegisterService(&tlscb_svc);
    server_builder.RegisterService(&tcpcb_svc);
    server_builder.RegisterService(&qos_svc);
    server_builder.RegisterService(&descraol_svc);
    server_builder.RegisterService(&wring_svc);
    server_builder.RegisterService(&proxy_svc);
    server_builder.RegisterService(&acl_svc);
    server_builder.RegisterService(&telemetry_svc);
    server_builder.RegisterService(&ipseccb_svc);
    server_builder.RegisterService(&cpucb_svc);
    server_builder.RegisterService(&crypto_key_svc);
    server_builder.RegisterService(&rawrcb_svc);
    server_builder.RegisterService(&rawccb_svc);
    server_builder.RegisterService(&crypto_apis_svc);
    server_builder.RegisterService(&event_svc);
    server_builder.RegisterService(&quiesce_svc);

    HAL_TRACE_DEBUG("gRPC server listening on ... {}", server_addr.c_str());

    // assemble the server
    std::unique_ptr<Server> server(server_builder.BuildAndStart());

    // wait for server to shutdown (some other thread must be responsible for
    // shutting down the server or else this call won't return)
    server->Wait();
}

// print help message showing usage of HAL
static void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c|--config <cfg.json>\n", argv[0]);
}

// TODO: need to figure a clean way for HAL to exit
// 1. hook on to D-Bus esp. system bus
// 2. handle signals
// 3. need to release all memory/resources and kill the FTE threads etc.
int
main (int argc, char **argv)
{
    int              oc;
    char             *cfg_file = NULL, *cfg_path;
    std::string      full_path, ini_full_path, ini_file = "hal.ini";
    hal::hal_cfg_t    hal_cfg = { 0 };
	struct option longopts[] = {
	   { "config",  required_argument, NULL, 'c' },
	   { "help",    no_argument,       NULL, 'h' },
	   { 0,         0,                 0,     0 }
	};

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hc:W;", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            cfg_file = optarg;
            if (!cfg_file) {
                fprintf(stderr, "config file is not specified\n");
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

    // makeup the full file path
    cfg_path = std::getenv("HAL_CONFIG_PATH");
    if (cfg_path) {
        full_path =  std::string(cfg_path) + "/" + std::string(cfg_file);
        std::cerr << "full path " << full_path << std::endl;
        ini_full_path = std::string(cfg_path) + "/" + ini_file;
        std::cerr << "ini file full path " << ini_full_path << std::endl;
    } else {
        full_path = std::string(cfg_file);
    }

    // make sure cfg file exists
    if (access(full_path.c_str(), R_OK) < 0) {
        fprintf(stderr, "config file %s has no read permissions\n",
                full_path.c_str());
        exit(1);
    }

    // parse the config
    if (hal::hal_parse_cfg(full_path.c_str(), &hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL config file parsing failed, quitting ...\n");
        exit(1);
    }

    // parse the ini
    if (hal::hal_parse_ini(ini_full_path.c_str(), &hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL ini file parsing failed, quitting ...\n");
        exit(1);
    }

    // initialize HAL
    if (hal::hal_init(&hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }

    if (hal::pd::lkl_init() != HAL_RET_OK) {
        fprintf(stderr, "LKL initialization failed, quitting ...\n");
        exit(1);
    }

    // register for all gRPC services
    svc_reg(std::string("localhost:") + hal_cfg.grpc_port);

    // wait for HAL threads to cleanup
    hal::hal_wait();

    return 0;
}
