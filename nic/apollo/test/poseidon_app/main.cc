//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// This file contains the main for the poseidon_app daemon
///
//----------------------------------------------------------------------------

#include <unistd.h>
#include <getopt.h>
#include <limits.h>
#include <string>
#include <iostream>
#include <stdarg.h>
#include "nic/apollo/api/include/pds.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/apollo/api/include/pds_batch.hpp"
#include "nic/apollo/api/include/pds_device.hpp"
#include "nic/sdk/platform/capri/capri_p4.hpp"
#include "nic/sdk/model_sim/include/lib_model_client.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -c|--config <cfg.json>\n", argv[0]);
}

int
main (int argc, char **argv)
{
    int          oc;
    string       cfg_path, cfg_file, profile, pipeline, file;
    boost::property_tree::ptree pt;
    sdk_ret_t    ret;

    struct option longopts[] = {
       { "config",    required_argument, NULL, 'c' },
       { "profile",   required_argument, NULL, 'p' },
       { "feature",   required_argument, NULL, 'f' },
       { "help",      no_argument,       NULL, 'h' },
       { 0,           0,                 0,     0 }
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hc:p:f:W;", longopts, NULL)) != -1) {
        switch (oc) {
        case 'c':
            if (optarg) {
                cfg_file = std::string(optarg);
            } else {
                fprintf(stderr, "config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'p':
            if (optarg) {
                profile = std::string(optarg);
            } else {
                fprintf(stderr, "profile is not specified\n");
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

    // form the full path to the config directory
    cfg_path = std::string(std::getenv("CONFIG_PATH"));
    if (cfg_path.empty()) {
        cfg_path = std::string("./");
    } else {
        cfg_path += "/";
    }

    // read pipeline.json file to figure out pipeline
    file = cfg_path + "/pipeline.json";
    if (access(file.c_str(), R_OK) < 0) {
        fprintf(stderr, "pipeline.json doesn't exist or not accessible\n");
        exit(1);
    }

    // parse pipeline.json to determine pipeline
    try {
        std::ifstream json_cfg(file.c_str());
        read_json(json_cfg, pt);
        pipeline = pt.get<std::string>("pipeline");
    } catch (...) {
        fprintf(stderr, "pipeline.json doesn't have pipeline field\n");
        exit(1);
    }
    if ((pipeline.compare("apollo") != 0) &&
        (pipeline.compare("artemis") != 0) &&
        (pipeline.compare("poseidon") != 0) &&
        (pipeline.compare("apulu") != 0)) {
        fprintf(stderr, "Unknown pipeline %s\n", pipeline.c_str());
        exit(1);
    }

   // make sure the cfg file exists
    file = cfg_path + "/" + pipeline + "/" + std::string(cfg_file);
    if (access(file.c_str(), R_OK) < 0) {
        fprintf(stderr, "Config file %s doesn't exist or not accessible\n",
                file.c_str());
        exit(1);
    }

    // Initialize the PDS functionality
    pds_init_params_t init_params;

    memset(&init_params, 0, sizeof(init_params));
    init_params.init_mode = PDS_INIT_MODE_COLD_START;
    init_params.trace_cb  = NULL;
    init_params.pipeline  = pipeline;
    init_params.cfg_file  = cfg_file;
    init_params.scale_profile = PDS_SCALE_PROFILE_DEFAULT;
    if (!profile.empty()) {
        if (profile.compare("p1") == 0) {
            init_params.scale_profile = PDS_SCALE_PROFILE_P1;
        } else if (profile.compare("p2") == 0) {
            init_params.scale_profile = PDS_SCALE_PROFILE_P2;
        }
    }

    pds_init(&init_params);

    // wait forver
    printf("Initialization done ...");
    while (1);

    return 0;
}

