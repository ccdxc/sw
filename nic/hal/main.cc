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
#include <hal.hpp>
#include <tenant_svc.hpp>
#include <network_svc.hpp>
#include <interface_svc.hpp>
#include <l2segment_svc.hpp>
#include <endpoint_svc.hpp>
#include <session_svc.hpp>
#include <l4lb_svc.hpp>
#include <nwsec_svc.hpp>
#include <tlscb_svc.hpp>
#include <tcpcb_svc.hpp>
#include <qos_svc.hpp>
#include <descriptor_aol_svc.hpp>
#include <wring_svc.hpp>
#include <proxy_svc.hpp>
#include <acl_svc.hpp>
#include <telemetry_svc.hpp>

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
    L2SegmentServiceImpl     l2seg_svc;
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
 

    HAL_TRACE_DEBUG("Bringing gRPC server for all API services ...");

    // listen on the given address (no authentication)
    server_builder.AddListeningPort(server_addr,
                                    grpc::InsecureServerCredentials());

    // register all services
    server_builder.RegisterService(&tenant_svc);
    server_builder.RegisterService(&network_svc);
    server_builder.RegisterService(&if_svc);
    server_builder.RegisterService(&l2seg_svc);
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
    std::string      full_path;
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

    // initialize HAL
    if (hal::hal_init(&hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }

    // register for all gRPC services
    svc_reg(std::string("localhost:") + hal_cfg.grpc_port);

    // wait for HAL threads to cleanup
    hal::hal_wait();

    return 0;
}
