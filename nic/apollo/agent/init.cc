//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#include <signal.h>
#include <iostream>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/sdk/lib/event_thread/event_thread.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/agent/core/core.hpp"
#include "nic/apollo/agent/core/cmd.hpp"
#include "nic/apollo/agent/core/state.hpp"
#include "nic/apollo/api/include/pds_init.hpp"
#include "nic/metaswitch/stubs/pdsa_stubs_init.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace core {

//TODO: Move these to global store
static sdk::event_thread::event_thread *g_cmd_server_thread;
static sdk::lib::thread *g_routing_thread;
sdk_ret_t spawn_routing_thread(void);

#define DEVICE_CONF_FILE    "/sysconfig/config0/device.conf"

// number of trace files to keep
#define TRACE_NUM_FILES                        5
#define TRACE_FILE_SIZE                        (20 << 20)
typedef void (*sig_handler_t)(int sig, siginfo_t *info, void *ptr);

//------------------------------------------------------------------------------
// handler invoked during normal termnination
//------------------------------------------------------------------------------
static void
atexit_handler (void)
{
    PDS_TRACE_DEBUG("Flushing logs before exiting ...");
    if (trace_logger()) {
        trace_logger()->flush();
    }
}

//------------------------------------------------------------------------------
// initialize all the signal handlers
//------------------------------------------------------------------------------
static void
pds_sig_handler (int sig, siginfo_t *info, void *ptr)
{
    switch (sig) {
    case SIGINT:
    case SIGTERM:
    case SIGQUIT:
        raise(SIGKILL);
        break;

    case SIGUSR1:
        spawn_routing_thread();
        break;

    case SIGUSR2:
    case SIGHUP:
    case SIGCHLD:
    case SIGURG:
    default:
        break;
    }
}

//------------------------------------------------------------------------------
// initialize all the signal handlers
//------------------------------------------------------------------------------
sdk_ret_t
pds_sig_init (sig_handler_t sig_handler)
{
    struct sigaction    act;

    if (sig_handler == NULL) {
        return SDK_RET_ERR;
    }

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = sig_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);
    sigaction(SIGURG, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// logger callback passed to SDK and PDS lib
//------------------------------------------------------------------------------
static int
sdk_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    switch (tracel_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        PDS_TRACE_ERR_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        PDS_TRACE_WARN_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        PDS_TRACE_INFO_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
        PDS_TRACE_DEBUG_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
        PDS_TRACE_VERBOSE_NO_META("{}", logbuf);
        break;
    default:
        break;
    }
    va_end(args);

    return 0;
}

//------------------------------------------------------------------------------
// initialize PDS library
//------------------------------------------------------------------------------
static inline sdk_ret_t
init_pds (std::string cfg_file, std::string profile, std::string pipeline)
{
    sdk_ret_t ret;
    pds_init_params_t init_params;

    memset(&init_params, 0, sizeof(init_params));
    init_params.init_mode = PDS_INIT_MODE_COLD_START;
    init_params.trace_cb  = sdk_logger;
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
    ret = pds_init(&init_params);
    return ret;
}

static inline string
log_file (const char *logdir, const char *logfile)
{
    struct stat st = { 0 };

    if (!logdir) {
        return std::string(logfile);
    }

    // check if this log dir exists
    if (stat(logdir, &st) == -1) {
        // doesn't exist, try to create
        if (mkdir(logdir, 0755) < 0) {
            fprintf(stderr,
                    "Log directory %s/ doesn't exist, failed to create "
                    "one\n", logdir);
            return std::string("");
        }
    } else {
        // log dir exists, check if we have write permissions
        if (access(logdir, W_OK) < 0) {
            // don't have permissions to create this directory
            fprintf(stderr,
                    "No permissions to create log file in %s/\n",
                    logdir);
            return std::string("");
        }
    }
    return logdir + std::string(logfile);
}

//------------------------------------------------------------------------------
// initialize the logger
//------------------------------------------------------------------------------
static inline sdk_ret_t
logger_init (void)
{
    std::string logfile, err_logfile;

    logfile = log_file(std::getenv("LOG_DIR"), "./pds-agent.log");
    err_logfile = log_file(std::getenv("PERSISTENT_LOG_DIR"), "/obfl.log");

    if (logfile.empty() || err_logfile.empty()) {
        return SDK_RET_ERR;
    }

    // initialize the logger
    core::trace_init("agent", 0x1, true, err_logfile.c_str(), logfile.c_str(),
                     TRACE_FILE_SIZE, TRACE_NUM_FILES, utils::trace_debug);

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// Get device profile from device.conf
//------------------------------------------------------------------------------
static inline std::string
device_profile_read (void)
{
    boost::property_tree::ptree pt;

    PDS_TRACE_DEBUG("Read device profile...");
    try {
        std::ifstream json_cfg(DEVICE_CONF_FILE);
        read_json(json_cfg, pt);
        return pt.get<std::string>("profile", "default");
    } catch (...) {
        return std::string("default");
    }
}

//------------------------------------------------------------------------------
// spawn command server thread
//------------------------------------------------------------------------------
sdk_ret_t
spawn_cmd_server_thread (void)
{
    // spawn periodic thread that does background tasks
    g_cmd_server_thread =
        sdk::event_thread::event_thread::factory(
            "cfg", THREAD_ID_AGENT_CMD_SERVER, sdk::lib::THREAD_ROLE_CONTROL,
            0x0, core::cmd_server_thread_init, core::cmd_server_thread_exit,
            NULL, // message
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            true);

    SDK_ASSERT_TRACE_RETURN((g_cmd_server_thread != NULL), SDK_RET_ERR,
                            "Command server thread create failure");
    g_cmd_server_thread->start(g_cmd_server_thread);

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// spawn thread for metaswitch control plane stack
//------------------------------------------------------------------------------
sdk_ret_t
spawn_routing_thread (void)
{
    // spawn control plane routing thread
    g_routing_thread =
        sdk::lib::thread::factory(
            "routing", THREAD_ID_AGENT_ROUTING, sdk::lib::THREAD_ROLE_CONTROL,
            0x0, &pdsa_stub::pdsa_thread_init,
            sdk::lib::thread::priority_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            sdk::lib::thread::sched_policy_by_role(sdk::lib::THREAD_ROLE_CONTROL),
            false);

    SDK_ASSERT_TRACE_RETURN((g_routing_thread != NULL), SDK_RET_ERR,
                            "Routing thread create failure");
    g_routing_thread->start(g_routing_thread);

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// initialize the agent
//------------------------------------------------------------------------------
sdk_ret_t
agent_init (std::string cfg_file, std::string profile, std::string pipeline)
{
    sdk_ret_t    ret;

    // initialize the logger instance
    logger_init();

    // read device profile, if it exists
    if (profile.empty()) {
        profile = device_profile_read();
    }

    // initialize PDS library
    ret = init_pds(cfg_file, profile, pipeline);

    // init agent state
    ret = core::agent_state::init();
    if (ret != SDK_RET_OK) {
        return ret;
    }

    // spawn command server thread
    ret = spawn_cmd_server_thread();
    if (ret != SDK_RET_OK) {
        return ret;
    }
    
    // spawn metaswitch control plane thread
    // TODO: Triggerred from sig handler for now
#if 0
    ret = spawn_routing_thread();
    if (ret != SDK_RET_OK) {
        return ret;
    }
#endif

    if (std::getenv("PDS_MOCK_MODE")) {
        agent_state::state()->pds_mock_mode_set(true);
    }

    // install signal handlers
    pds_sig_init(pds_sig_handler);

    // install atexit() handler
    atexit(atexit_handler);
    return ret;
}

}    // namespace core
