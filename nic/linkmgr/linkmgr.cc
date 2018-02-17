// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "linkmgr.hpp"
#include "sdk/linkmgr.hpp"
#include "linkmgr_src.hpp"

extern "C" void __gcov_flush(void);

#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif

namespace linkmgr {

//------------------------------------------------------------------------------
// initialize all the signal handlers
//------------------------------------------------------------------------------
static void
linkmgr_sig_handler (int sig, siginfo_t *info, void *ptr)
{
    HAL_TRACE_DEBUG("HAL received signal {}", sig);

    switch (sig) {
    case SIGINT:
        HAL_GCOV_FLUSH();
        if (hal::utils::hal_logger()) {
            hal::utils::hal_logger()->flush();
        }
        exit(0);
        break;

    case SIGUSR1:
    case SIGUSR2:
        HAL_GCOV_FLUSH();
        if(hal::utils::hal_logger()) {
            hal::utils::hal_logger()->flush();
        }
        break;

    case SIGHUP:
    case SIGQUIT:
    case SIGCHLD:
    case SIGURG:
    case SIGTERM:
    default:
        if (hal::utils::hal_logger()) {
            hal::utils::hal_logger()->flush();
        }
        break;
    }
}

//------------------------------------------------------------------------------
// initialize all the signal handlers
// TODO: save old handlers and restore when signal happened
//------------------------------------------------------------------------------
static hal_ret_t
linkmgr_sig_init (void)
{
    struct sigaction    act;

    memset(&act, 0, sizeof(act));
    act.sa_sigaction = linkmgr_sig_handler;
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGHUP, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGCHLD, &act, NULL);
    sigaction(SIGURG, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    sigaction(SIGTERM, &act, NULL);

    return HAL_RET_OK;
}

}

int
sdk_error_logger (const char *format, ...)
{
    char       logbuf[128];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    HAL_TRACE_ERR_NO_META("{}", logbuf);
    va_end(args);

    return 0;
}

int
sdk_debug_logger (const char *format, ...)
{
    char       logbuf[128];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    HAL_TRACE_DEBUG_NO_META("{}", logbuf);
    va_end(args);

    return 0;
}

//------------------------------------------------------------------------------
// linkmgr's entry point
//------------------------------------------------------------------------------
int
main (int argc, char **argv)
{
    linkmgr::linkmgr_sig_init();

    sdk::lib::thread::control_cores_mask_set(0x1);

    // Initialize the logger
    hal::utils::logger_init(ffsl(sdk::lib::thread::control_cores_mask()) - 1,
                            0);
    sdk::lib::logger::init(sdk_error_logger, sdk_debug_logger);

    linkmgr::linkmgr_init();

    HAL_TRACE_DEBUG("linkmgr done");

    return 0;
}
