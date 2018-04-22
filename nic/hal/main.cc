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

#include "nic/hal/svc/debug_svc.hpp"
#include "nic/hal/svc/table_svc.hpp"
#include "nic/hal/svc/rdma_svc.hpp"
#include "nic/hal/svc/session_svc.hpp"
#include "nic/hal/svc/wring_svc.hpp"
#include "nic/hal/svc/telemetry_svc.hpp"
#include "nic/hal/svc/rawrcb_svc.hpp"
#include "nic/hal/svc/event_svc.hpp"
#include "nic/hal/svc/quiesce_svc.hpp"
#include "nic/hal/svc/system_svc.hpp"
#include "nic/hal/svc/barco_rings_svc.hpp"
#include "nic/hal/svc/interface_svc.hpp"
#include "nic/hal/svc/proxy_svc.hpp"

#include "nic/gen/hal/svc/nw_svc_gen.hpp"
#include "nic/gen/hal/svc/tls_proxy_cb_svc_gen.hpp"
#include "nic/gen/hal/svc/tcp_proxy_cb_svc_gen.hpp"
#include "nic/gen/hal/svc/proxyccb_svc_gen.hpp"
#include "nic/gen/hal/svc/proxyrcb_svc_gen.hpp"
#include "nic/gen/hal/svc/vrf_svc_gen.hpp"
#include "nic/gen/hal/svc/l2segment_svc_gen.hpp"
#include "nic/gen/hal/svc/internal_svc_gen.hpp"
#include "nic/gen/hal/svc/endpoint_svc_gen.hpp"
#include "nic/gen/hal/svc/l4lb_svc_gen.hpp"
#include "nic/gen/hal/svc/nwsec_svc_gen.hpp"
#include "nic/gen/hal/svc/dos_svc_gen.hpp"
#include "nic/gen/hal/svc/qos_svc_gen.hpp"
#include "nic/gen/hal/svc/descriptor_aol_svc_gen.hpp"
#include "nic/gen/hal/svc/acl_svc_gen.hpp"
#include "nic/gen/hal/svc/ipseccb_svc_gen.hpp"
#include "nic/gen/hal/svc/cpucb_svc_gen.hpp"
#include "nic/gen/hal/svc/crypto_keys_svc_gen.hpp"
#include "nic/gen/hal/svc/rawccb_svc_gen.hpp"
#include "nic/gen/hal/svc/proxyrcb_svc_gen.hpp"
#include "nic/gen/hal/svc/proxyccb_svc_gen.hpp"
#include "nic/gen/hal/svc/crypto_apis_svc_gen.hpp"
#include "nic/gen/hal/svc/multicast_svc_gen.hpp"
#include "nic/gen/hal/svc/gft_svc_gen.hpp"

#include "nic/hal/lkl/lkl_api.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

void
svc_reg (const std::string& server_addr,
         hal::hal_feature_set_t feature_set)
{
    VrfServiceImpl           vrf_svc;
    NetworkServiceImpl       nw_svc;
    InterfaceServiceImpl     if_svc;
    InternalServiceImpl      internal_svc;
    RdmaServiceImpl          rdma_svc;
    L2SegmentServiceImpl     l2seg_svc;
    DebugServiceImpl         debug_svc;
    TableServiceImpl         table_svc;
    SessionServiceImpl       session_svc;
    EndpointServiceImpl      endpoint_svc;
    L4LbServiceImpl          l4lb_svc;
    NwSecurityServiceImpl    nwsec_svc;
    DosServiceImpl           dos_svc;
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
    ProxyrCbServiceImpl      proxyrcb_svc;
    ProxycCbServiceImpl      proxyccb_svc;
    CryptoApisServiceImpl    crypto_apis_svc;
    EventServiceImpl         event_svc;
    QuiesceServiceImpl       quiesce_svc;
    BarcoRingsServiceImpl    barco_rings_svc;
    MulticastServiceImpl     multicast_svc;
    GftServiceImpl           gft_svc;
    SystemServiceImpl        system_svc;
    SoftwarePhvServiceImpl   swphv_svc;

    HAL_TRACE_DEBUG("Bringing gRPC server for all API services ...");

    // listen on the given address (no authentication)
    server_builder.AddListeningPort(server_addr,
                                    grpc::InsecureServerCredentials());

    // register all services
    if (feature_set == hal::HAL_FEATURE_SET_IRIS) {
        server_builder.RegisterService(&vrf_svc);
        server_builder.RegisterService(&nw_svc);
        server_builder.RegisterService(&if_svc);
        server_builder.RegisterService(&internal_svc);
        server_builder.RegisterService(&rdma_svc);
        server_builder.RegisterService(&l2seg_svc);
        server_builder.RegisterService(&debug_svc);
        server_builder.RegisterService(&table_svc);
        server_builder.RegisterService(&session_svc);
        server_builder.RegisterService(&endpoint_svc);
        server_builder.RegisterService(&l4lb_svc);
        server_builder.RegisterService(&nwsec_svc);
        server_builder.RegisterService(&dos_svc);
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
        server_builder.RegisterService(&proxyrcb_svc);
        server_builder.RegisterService(&proxyccb_svc);
        server_builder.RegisterService(&crypto_apis_svc);
        server_builder.RegisterService(&event_svc);
        server_builder.RegisterService(&quiesce_svc);
        server_builder.RegisterService(&barco_rings_svc);
        server_builder.RegisterService(&multicast_svc);
        server_builder.RegisterService(&system_svc);
        server_builder.RegisterService(&swphv_svc);
    } else if (feature_set == hal::HAL_FEATURE_SET_GFT) {
        server_builder.RegisterService(&vrf_svc);
        server_builder.RegisterService(&if_svc);
        server_builder.RegisterService(&rdma_svc);
        server_builder.RegisterService(&l2seg_svc);
        server_builder.RegisterService(&gft_svc);
        server_builder.RegisterService(&system_svc);
        // Revisit. DOL was not able to create Lif without qos class
        server_builder.RegisterService(&qos_svc);
        // Revisit. DOL was not able to create Tenant with security profile.
        server_builder.RegisterService(&nwsec_svc);
        server_builder.RegisterService(&dos_svc);
        server_builder.RegisterService(&endpoint_svc);
    }

    HAL_TRACE_DEBUG("gRPC server listening on ... {}", server_addr.c_str());
    hal::utils::hal_logger()->flush();
    HAL_SYSLOG_INFO("HAL-STATUS:UP");

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
    fprintf(stdout, "Usage : %s -c|--config <cfg.json> [-p|--platform <catalog.json>] \n", argv[0]);
}

// TODO: need to figure a clean way for HAL to exit
// 1. hook on to D-Bus esp. system bus
// 2. need to release all memory/resources and kill the FTE threads etc.
int
main (int argc, char **argv)
{
    int               oc;
    char              *cfg_file = NULL, *catalog_file = NULL;
    char              *default_config_dir = NULL;
    std::string       ini_file = "hal.ini";
    hal::hal_cfg_t    hal_cfg;

    bzero(&hal_cfg, sizeof(hal_cfg));
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

    // parse the HAL config file
    if (hal::hal_parse_cfg(cfg_file, &hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL config file parsing failed, quitting ...\n");
        exit(1);
    }

    // set the full path of the catalog file
    if (catalog_file) {
        hal_cfg.catalog_file =
            hal_cfg.cfg_path + "/" + std::string(catalog_file);
    } else {
        hal_cfg.catalog_file = hal_cfg.cfg_path + "/catalog.json";
    }

    // make sure catalog file exists
    if (access(hal_cfg.catalog_file.c_str(), R_OK) < 0) {
        fprintf(stderr, "Catalog file %s has no read permissions\n",
                hal_cfg.catalog_file.c_str());
        exit(1);
    }

    // TODO: HAL_PBC_INIT_CONFIG will have to go away
    default_config_dir = std::getenv("HAL_PBC_INIT_CONFIG");
    if (default_config_dir) {
        hal_cfg.default_config_dir = std::string(default_config_dir);
    } else {
        hal_cfg.default_config_dir = std::string("8x25_hbm");
    }

    // parse the ini file, if it exists
    hal::hal_parse_ini(ini_file.c_str(), &hal_cfg);

    // initialize HAL
    if (hal::hal_init(&hal_cfg) != HAL_RET_OK) {
        fprintf(stderr, "HAL initialization failed, quitting ...\n");
        exit(1);
    }

    if (hal_cfg.platform_mode != hal::HAL_PLATFORM_MODE_HAPS &&
        hal_cfg.features != hal::HAL_FEATURE_SET_GFT) {
        if (hal::pd::lkl_init() != HAL_RET_OK) {
            fprintf(stderr, "LKL initialization failed, quitting ...\n");
            exit(1);
        }
    }

    // register for all gRPC services
    svc_reg(std::string("0.0.0.0:") + hal_cfg.grpc_port, hal_cfg.features);

    // wait for HAL threads to cleanup
    hal::hal_wait();

    return 0;
}
