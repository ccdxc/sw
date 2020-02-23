//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------

#include <stdio.h>
#include <getopt.h>
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/apollo/api/pds_state.hpp"
#include "nic/apollo/upgrade/logger.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace upg {

sdk::operd::logger_ptr g_upg_log = sdk::operd::logger::create(UPG_LOG_NAME);

// TODO: may need to remove the below code once pds_init moved log operd
// based logging

static int
sdk_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    switch (tracel_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        g_upg_log->err("%s", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        g_upg_log->warn("%s", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        g_upg_log->info("%s", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
        g_upg_log->debug("%s", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
        g_upg_log->trace("%s", logbuf);
        break;
    default:
        break;
    }
    va_end(args);

    return 0;
}

}    // namespace upg

// this will be invokded during pre-upgrade stage.
// all the pre-upgrade validations should be done here
// upgrade manager should invoke this with running and new dsc top directory path

static string g_running_cfgpath;
static bool compcheck_status = false;

static sdk_ret_t
mpartition_validate (void)
{
    // this code is executed using new binaries and new topdir, and it also
    // need to read running conf/generated files. so we should avoid changing
    // the directory layout across upgrades. requires special handling otherwise.
    bool oper_table_persist = false; // TODO extract this from upg_shm() info
    sdk_ret_t ret;

    g_upg_log->debug("Compat check running cfgpath %s\n", g_running_cfgpath.c_str());

    // validates the memory config b/w new and running
    ret = api::g_pds_state.mempartition()->upg_regions(g_running_cfgpath.c_str(),
                                                       oper_table_persist);
    return ret;
}

static sdk_ret_t
compat_check (const pds_event_t *event)
{
    if (event->event_id != PDS_EVENT_ID_UPG) {
        return SDK_RET_OK;
    }
    const pds_upg_spec_t *spec = &event->upg_spec;
    SDK_ASSERT (spec->stage == UPG_STAGE_NONE);
    (void)mpartition_validate();
    // need to terminate the pds_init
    return SDK_RET_ERR;
}

static sdk_ret_t
init (std::string cfg_file, std::string profile, std::string pipeline)
{
    pds_init_params_t init_params;

    // soft init for safe side
    sdk::asic::set_init_type(sdk::asic::asic_init_type_t::ASIC_INIT_TYPE_SOFT);
    memset(&init_params, 0, sizeof(init_params));
    init_params.init_mode = PDS_INIT_MODE_COLD_START;
    init_params.trace_cb  = upg::sdk_logger;
    init_params.pipeline  = pipeline;
    init_params.cfg_file  = cfg_file;
    init_params.event_cb  = compat_check;
    init_params.scale_profile = PDS_SCALE_PROFILE_DEFAULT; // TODO: Read from existing config
    pds_init(&init_params);
    // cannot use pds_init return value as we are exiting the execution
    // by returning error.
    if (compcheck_status) {
        return SDK_RET_OK;
    }
    return SDK_RET_ERR;
}

static void inline
print_usage (char **argv)
{
    fprintf(stdout, "Usage : %s -i <running-cfgpath> -c <config file> \n", argv[0]);
}

int
main (int argc, char **argv)
{
    int oc, ret;
    string cfg_path, cfg_file, profile, pipeline, file;
    boost::property_tree::ptree pt;
    struct option longopts[] = {
       { "running-cfgpath", required_argument, NULL, 'r' },
       { "config",          required_argument, NULL, 'c' },
       { "profile",         required_argument, NULL, 'p' },
       { "help",            no_argument,       NULL, 'h' },
       { 0,                 0,                 0,     0 }
    };

    // parse CLI options
    while ((oc = getopt_long(argc, argv, ":hi:c:p:", longopts, NULL)) != -1) {
        switch (oc) {
        case 'i':
            if (optarg) {
                g_running_cfgpath = std::string(optarg);
            } else {
                fprintf(stderr, "running config path not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;
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
        default:
            // ignore all other options
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
    if (pipeline.compare("apulu") != 0) {
        fprintf(stderr, "unknown pipeline %s\n", pipeline.c_str());
        exit(1);
    }

    ret = init(cfg_file, profile, pipeline);
    ret == SDK_RET_OK ? exit(0) : exit(1);
}
