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

#include "nic/sdk/platform/capri/capri_state.hpp"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "nic/sdk/platform/pciemgr_if/include/pciemgr_if.hpp"
#include "platform/src/lib/nicmgr/include/logger.hpp"

#include "delphic.hpp"
#include "devapi_types.hpp"

#define MAX_STRING_BUFF_SIZE   100

using namespace std;

DeviceManager *devmgr;
pciemgr *pciemgr;
static string config_file;
fwd_mode_t fwd_mode = sdk::platform::FWD_MODE_CLASSIC;
platform_t platform = PLATFORM_NONE;
bool g_hal_up = false;
extern void nicmgr_do_client_registration(void);

static void
log_flush(void *arg)
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
    fflush(stdout);
    fflush(stderr);
    if (utils::logger::logger()) {
        utils::logger::logger()->flush();
    }
}

const char* nicmgr_upgrade_state_file = "/update/nicmgr_upgstate";

static bool
upgrade_in_progress()
{
    return (access(nicmgr_upgrade_state_file, R_OK) == 0);
}

static void
loop(void)
{
    evutil_check log_check;
    bool upg_mode;

    upg_mode = upgrade_in_progress();

    NIC_LOG_INFO("upg_mode: {}", upg_mode);

    if (platform_is_hw(platform)) {
        pciemgr = new class pciemgr("nicmgrd", EV_DEFAULT);

        if (!upg_mode)
            pciemgr->initialize();
    }

    devmgr = new DeviceManager(config_file, fwd_mode, platform);

    devmgr->SetUpgradeMode(upg_mode);
    devmgr->LoadConfig(config_file);

    if (pciemgr) {
        pciemgr->finalize();
    }

    //All nicmgr objects are restored now so we can delete the file
    unlink(nicmgr_upgrade_state_file);

    evutil_add_check(EV_DEFAULT_ &log_check, &log_flush, NULL);

    evutil_run(EV_DEFAULT);

    /* NOTREACHED */
    if (pciemgr) {
        delete pciemgr;
    }
}

int main(int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "c:sp:")) != -1) {
        switch (opt) {
        case 'c':
            config_file = DeviceManager::ParseDeviceConf(string(optarg));
            break;
        case 's':
            fwd_mode = sdk::platform::FWD_MODE_SMART;
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

    // instantiate the logger
    utils::logger::init();

    // initialize sdk logger
    sdk_init();

    // install atexit() handler
    atexit(atexit_handler);

    if (config_file.empty()) {
        NIC_LOG_ERR("No config file");
        exit(1);
    }
    NIC_LOG_INFO("Using config file {}", config_file);

    // initialize capri_state_pd
    sdk::platform::capri::capri_state_pd_init(NULL);

    if (platform_is_hw(platform)) {
        nicmgr::delphi_init();
    }

    loop();

    return (0);
}
