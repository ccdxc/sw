// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "linkmgr_src.hpp"
#include <getopt.h>

extern "C" void __gcov_flush(void);

#ifdef COVERAGE
#define HAL_GCOV_FLUSH()     { ::__gcov_flush(); }
#else
#define HAL_GCOV_FLUSH()     { }
#endif

using hal::utils::hal_logger;

namespace linkmgr {

linkmgr_cfg_t linkmgr_cfg;

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
        if (hal_logger()) {
            hal_logger()->flush();
        }
        exit(0);
        break;

    case SIGUSR1:
    case SIGUSR2:
        HAL_GCOV_FLUSH();
        if(hal_logger()) {
            hal_logger()->flush();
        }
        break;

    case SIGHUP:
    case SIGQUIT:
    case SIGCHLD:
    case SIGURG:
    case SIGTERM:
    default:
        if (hal_logger()) {
            hal_logger()->flush();
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

static void inline
print_usage (char **argv)
{
    fprintf(stdout,
            "Usage : %s [-c|--config <cfg.json>] [-p|--platform <catalog.json>]\n",
            argv[0]);
}

int
parse_options(int argc, char **argv)
{
    int oc = 0;

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
            if (optarg) {
                linkmgr_cfg.cfg_file = std::string(optarg);
            } else {
                fprintf(stderr, "config file is not specified\n");
                print_usage(argv);
                exit(1);
            }
            break;

        case 'p':
            if (optarg) {
                linkmgr_cfg.catalog_file = std::string(optarg);
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

    return 0;
}

static void
linkmgr_cfg_init(void)
{
    linkmgr_cfg.cfg_file     = "linkmgr.json";
    linkmgr_cfg.catalog_file = "catalog.json";
}

} // namespace linkmgr

static int
sdk_logger (sdk_trace_level_e tracel_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    switch (tracel_level) {
    case sdk::lib::SDK_TRACE_LEVEL_ERR:
        HAL_TRACE_ERR_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_WARN:
        HAL_TRACE_WARN_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_INFO:
        HAL_TRACE_INFO_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_DEBUG:
        HAL_TRACE_DEBUG_NO_META("{}", logbuf);
        break;
    case sdk::lib::SDK_TRACE_LEVEL_VERBOSE:
        HAL_TRACE_VERBOSE_NO_META("{}", logbuf);
        break;
    default:
        break;
    }
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
    hal::utils::trace_init("linkmgr", sdk::lib::thread::control_cores_mask(),
                           false, "linkmgr.log",
                           TRACE_FILE_SIZE_DEFAULT, TRACE_NUM_FILES_DEFAULT,
                           ::utils::trace_debug);
    sdk::lib::logger::init(sdk_logger);

    linkmgr::linkmgr_cfg_init();

    linkmgr::parse_options(argc, argv);

    linkmgr::linkmgr_global_init(&linkmgr::linkmgr_cfg);

    HAL_TRACE_DEBUG("linkmgr done");

    return 0;
}
