/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>

#include "nic/sdk/platform/evutils/include/evutils.h"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"
#include "nic/sdk/platform/pciemgr/include/pciehw_dev.h"

#include "delphic.hpp"

using namespace std;

DeviceManager *devmgr;
pciemgr *pciemgr;
static string config_file;
enum ForwardingMode fwd_mode = FWD_MODE_CLASSIC_NIC;
platform_t platform = PLATFORM_NONE;
bool g_hal_up = false;
extern void nicmgr_do_client_registration(void);

static void
sigusr1_handler(int sig)
{
    fflush(stdout);
    fflush(stderr);
    if (utils::logger::logger()) {
        utils::logger::logger()->flush();
    }
}

static int
sdk_trace_cb (sdk_trace_level_e trace_level, const char *format, ...)
{
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);
    vsnprintf(logbuf, sizeof(logbuf), format, args);
    NIC_LOG_DEBUG("{}", logbuf);
    va_end(args);

    return 0;
}

static void
sdk_init (void)
{
    sdk::lib::logger::init(sdk_trace_cb);
}

static void
atexit_handler (void)
{
    if (devmgr) {
        devmgr->ThreadsWaitJoin();
    }
    fflush(stdout);
    fflush(stderr);
    if (utils::logger::logger()) {
        utils::logger::logger()->flush();
    }
}

static void
loop(void)
{
    if (platform_is_hw(platform)) {
        pciemgr = new class pciemgr("nicmgrd");
        pciemgr->initialize();
    }

    devmgr = new DeviceManager(config_file, fwd_mode, platform);
    devmgr->LoadConfig(config_file);

    if (pciemgr) {
        pciemgr->finalize();
    }

    evutil_run();
    /* NOTREACHED */
    if (pciemgr) {
        delete pciemgr;
    }
}

int main(int argc, char *argv[])
{
    int opt;
    sighandler_t osigusr1;

    while ((opt = getopt(argc, argv, "c:sp:")) != -1) {
        switch (opt) {
        case 'c':
            config_file = string(optarg);
            break;
        case 's':
            fwd_mode = FWD_MODE_SMART_NIC;
            break;
        case 'p':
            if (string(optarg) == "sim") {
                platform = PLATFORM_SIM;
            } else if (string(optarg) == "hw") {
                platform = PLATFORM_HW;
            } else if (string(optarg) == "haps") {
                platform = PLATFORM_HAPS;
            } else if (string(optarg) == "rtl") {
                platform = PLATFORM_RTL;
            } else if (string(optarg) == "mock") {
                platform = PLATFORM_MOCK;
            } else  {
                platform = PLATFORM_NONE;
            }
            break;
        default:
            exit(1);
        }
    }

    if (config_file.empty()) {
        cerr << "Please specify a config file" << endl;
        exit(1);
    }
    osigusr1 = signal(SIGUSR1, sigusr1_handler);
    // install atexit() handler
    atexit(atexit_handler);

    // instantiate the logger
    utils::logger::init();
    // initialize sdk logger
    sdk_init();
    if (platform_is_hw(platform)) {
        nicmgr::delphi_init();
    }
    loop();

    signal(SIGUSR1, osigusr1);

    return (0);
}
