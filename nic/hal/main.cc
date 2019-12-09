//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
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


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

std::string g_grpc_server_addr;
hal::hal_cfg_t    hal::g_hal_cfg;

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
