//------------------------------------------------------------------------------
// Copyright (c) 2019 Pensando Systems, Inc.
//------------------------------------------------------------------------------

#include <signal.h>
#include <iostream>
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/apollo/agent/trace.hpp"
#include "nic/apollo/api/include/pds_init.hpp"

namespace core {

// number of trace files to keep
#define TRACE_NUM_FILES                        10
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
    PDS_TRACE_DEBUG("PDS received signal {}", sig);
    if (trace_logger()) {
        trace_logger()->flush();
    }

    switch (sig) {
    case SIGINT:
    case SIGTERM:
    case SIGQUIT:
        raise(SIGKILL);
        break;

    case SIGUSR1:
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
init_pds (std::string cfg_file)
{
    sdk_ret_t ret;
    pds_init_params_t init_params;

    memset(&init_params, 0, sizeof(init_params));
    init_params.init_mode = PDS_INIT_MODE_COLD_START;
    init_params.trace_cb  = sdk_logger;
    init_params.pipeline  = "apollo";
    init_params.cfg_file  = cfg_file;
    ret = pds_init(&init_params);
    return ret;
}

//------------------------------------------------------------------------------
// initialize the logger
//------------------------------------------------------------------------------
static inline sdk_ret_t
logger_init (void)
{
    std::string    logfile;
    char           *logdir;
    struct stat    st = { 0 };

    logdir = std::getenv("LOG_DIR");
    if (!logdir) {
        // log in the current dir
        logfile = std::string("./pen-agent.log");
    } else {
        // check if this log dir exists
        if (stat(logdir, &st) == -1) {
            // doesn't exist, try to create
            if (mkdir(logdir, 0755) < 0) {
                fprintf(stderr,
                        "Log directory %s/ doesn't exist, failed to create "
                        "one\n", logdir);
                return SDK_RET_ERR;
            }
        } else {
            // log dir exists, check if we have write permissions
            if (access(logdir, W_OK) < 0) {
                // don't have permissions to create this directory
                fprintf(stderr,
                        "No permissions to create log file in %s\n",
                        logdir);
                return SDK_RET_ERR;
            }
        }
        logfile = logdir + std::string("/pen-agent.log");
    }

    // initialize the logger
    core::trace_init("agent", 0x1, true, logfile.c_str(),
                     TRACE_FILE_SIZE_DEFAULT, TRACE_NUM_FILES,
                     utils::trace_debug);

    return SDK_RET_OK;
}

//------------------------------------------------------------------------------
// initialize the agent
//------------------------------------------------------------------------------
sdk_ret_t
agent_init (std::string cfg_file)
{
    sdk_ret_t    ret;

    // initialize the logger instance
    logger_init();

    // initialize PDS library
    ret = init_pds(cfg_file);

    // install signal handlers
    pds_sig_init(pds_sig_handler);

    // install atexit() handler
    atexit(atexit_handler);
    return ret;
}

}    // namespace core
