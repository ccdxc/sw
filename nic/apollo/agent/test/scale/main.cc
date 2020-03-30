//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <getopt.h>
#include <inttypes.h>
#include <thread>
#include <iostream>
#include <fstream>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <google/protobuf/util/json_util.h>

#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/asic/port.hpp"
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include "nic/apollo/test/base/base.hpp"
#include "nic/apollo/test/base/utils.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/apollo/test/scale/test_common.hpp"
#include "nic/apollo/agent/test/client/app.hpp"
#include "nic/apollo/test/scale/test.hpp"

using std::string;
namespace pt = boost::property_tree;

static inline uint64_t
min(uint64_t a, uint64_t b) {

    return a < b ? a : b;
}

using std::string;
namespace pt = boost::property_tree;

char *g_input_cfg_file = NULL;
char *g_cfg_file = NULL;
bool g_daemon_mode = false;

// print help message showing usage of this gtest
static void inline print_usage(char **argv)
{
    fprintf(stdout, "Usage : %s -i <test-config.json>\n",
            argv[0]);
}

std::string  g_svc_endpoint_;

sdk_ret_t
test_app_push_configs (void)
{
    /* Create objects */
    create_objects();
    return SDK_RET_OK;
}

int
main (int argc, char **argv)
{
    int oc;
    std::string cmd;

    struct option longopts[] = {
        {"config", required_argument, NULL, 'c'},
        {"daemon", required_argument, NULL, 'd'},
        {"server", required_argument, NULL, 's'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hdc:i:s:W;",
                             longopts, NULL)) != -1) {
        switch (oc) {
        case 'd':
            g_daemon_mode = true;
            break;

        case 'i':
            g_input_cfg_file = optarg;
            if (!g_input_cfg_file) {
                fprintf(stderr, "test config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 's':
            g_svc_endpoint_ = optarg;
            if (g_svc_endpoint_.empty()) {
                fprintf(stderr, "server ip is not specified\n");
                print_usage(argv);
                exit(1);
            }
            g_svc_endpoint_ = g_svc_endpoint_ + ":50054";
            break;

        default:
            // ignore all other options
            break;
        }
    }

    // make sure cfg files exist
    if (access(g_input_cfg_file, R_OK) < 0) {
        fprintf(stderr, "Config file %s doesn't exist or not accessible\n",
                g_input_cfg_file);
        exit(1);
    }

    // copy the input cfg file for the agent to read
    cmd = "cp " + string(g_input_cfg_file) + " /tmp/" +
              string(get_cfg_json_name());
    system(cmd.c_str());

    // init app
    test_app_init();

    // push configs
    if (test_app_push_configs() != SDK_RET_OK) {
        fprintf(stderr, "Config push failed!\n");
        exit(1);
    }

    return 0;
}
