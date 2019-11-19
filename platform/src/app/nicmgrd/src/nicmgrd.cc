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
sdk::platform::platform_type_t platform = platform_type_t::PLATFORM_TYPE_NONE;
bool g_hal_up = false;
extern void nicmgr_do_client_registration(void);
extern bool devices_restored;

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
const char* nicmgr_rollback_state_file = "/update/nicmgr_rollback_state";

static bool
upgrade_in_progress()
{
    return (access(nicmgr_upgrade_state_file, R_OK) == 0);
}

static bool
rollback_in_progress()
{
    return (access(nicmgr_rollback_state_file, R_OK) == 0);
}

static void
loop(void)
{
    evutil_check log_check;
    std::vector <struct EthDevInfo *> eth_info;
    UpgradeMode fw_mode;

    if (rollback_in_progress()) {
        fw_mode = FW_MODE_ROLLBACK;
        unlink(nicmgr_rollback_state_file);
    }
    else if (upgrade_in_progress())
        fw_mode = FW_MODE_UPGRADE;
    else
        fw_mode = FW_MODE_NORMAL_BOOT;

    NIC_LOG_INFO("Upgrade mode: {}", fw_mode);

    devmgr = new DeviceManager(config_file, fwd_mode, platform);
    devmgr->SetUpgradeMode(fw_mode);

    if (platform_is_hw(platform))
        pciemgr = new class pciemgr("nicmgrd", devmgr->pcie_evhandler,
                                    EV_DEFAULT);

    if (pciemgr) {
        if (fw_mode == FW_MODE_NORMAL_BOOT)
            pciemgr->initialize();
        if (fw_mode != FW_MODE_UPGRADE)
            devmgr->LoadConfig(config_file);
        if (fw_mode == FW_MODE_NORMAL_BOOT)
            pciemgr->finalize();
    }

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
            config_file = DeviceManager::ParseDeviceConf(string(optarg), &fwd_mode);
            break;
        case 's':
            // Ignore cmd line mode specification. Determines mode always from device.conf
            // fwd_mode = sdk::platform::FWD_MODE_SMART;
            break;
        case 'p':
            if (string(optarg) == "sim") {
                platform = platform_type_t::PLATFORM_TYPE_SIM;
            } else if (string(optarg) == "hw") {
                platform = platform_type_t::PLATFORM_TYPE_HW;
            } else if (string(optarg) == "haps") {
                platform = platform_type_t::PLATFORM_TYPE_HAPS;
            } else if (string(optarg) == "rtl") {
                platform = platform_type_t::PLATFORM_TYPE_RTL;
            } else if (string(optarg) == "mock") {
                platform = platform_type_t::PLATFORM_TYPE_MOCK;
            } else  {
                platform = platform_type_t::PLATFORM_TYPE_NONE;
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
