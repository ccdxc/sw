/*
* Copyright (c) 2018, Pensando Systems Inc.
*/

#include <cstdio>
#include <algorithm>
#include <cstdlib>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>

#include "nic/sdk/lib/device/device.hpp"
#include "nic/sdk/platform/evutils/include/evutils.h"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/nicmgr/include/logger.hpp"

#include "delphic.hpp"

using namespace std;

DeviceManager *devmgr;
sdk::platform::platform_type_t platform;
static string config_file;

UpgradeMode upg_mode;
const char* nicmgr_upgrade_state_file = "/update/nicmgr_upgstate";
const char* nicmgr_rollback_state_file = "/update/nicmgr_rollback_state";

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
    string profile;
    bool micro_seg_en = false;
    sdk::lib::device *device = NULL;
    sdk::lib::dev_forwarding_mode_t fwd_mode;
    sdk::lib::dev_feature_profile_t feature_profile;

    // Load device configuration
    device = sdk::lib::device::factory(config_file);
    fwd_mode = device->get_forwarding_mode();
    feature_profile = device->get_feature_profile();
    micro_seg_en = (device->get_micro_seg_en() == device::MICRO_SEG_ENABLE);

    // TODO: Profile should be independent of forwarding mode.
    // TODO: No need to figure out the profile while upgrading.
    if (fwd_mode == sdk::lib::FORWARDING_MODE_HOSTPIN ||
        fwd_mode == sdk::lib::FORWARDING_MODE_SWITCH) {
        profile = "/platform/etc/nicmgrd/eth_smart.json";
    } else {
        if (feature_profile == sdk::lib::FEATURE_PROFILE_CLASSIC_ETH_DEV_SCALE) {
            profile = "/platform/etc/nicmgrd/eth_scale.json";
        } else {
            profile = "/platform/etc/nicmgrd/device.json";
        }
    }

    NIC_LOG_INFO("Forwarding Mode {}", fwd_mode);
    NIC_LOG_INFO("Micro-segmentation {}", micro_seg_en);
    NIC_LOG_INFO("Feature Profile {} {}", feature_profile, profile);

    // Are we in the middle of an upgrade?
    if (rollback_in_progress()) {
        upg_mode = FW_MODE_ROLLBACK;
    } else if (upgrade_in_progress()) {
        upg_mode = FW_MODE_UPGRADE;
    } else {
        upg_mode = FW_MODE_NORMAL_BOOT;
    }

    NIC_LOG_INFO("Upgrade mode: {}", upg_mode);

    // Start device manager
    devmgr = new DeviceManager(platform, fwd_mode, micro_seg_en, EV_DEFAULT);
    if (upg_mode == FW_MODE_NORMAL_BOOT) {
        devmgr->LoadProfile(profile, true);
    } else if (upg_mode == FW_MODE_ROLLBACK) {
        devmgr->LoadProfile(profile, false);
        unlink(nicmgr_rollback_state_file);
    } else {
        // Delphi will Restore State
        unlink(nicmgr_upgrade_state_file);
    }

    // Connect to delphi
    if (platform_is_hw(platform)) {
        NIC_LOG_INFO("Initializing Delphi Client");
        nicmgr::delphi_init();
    }

    evutil_add_check(EV_DEFAULT_ &log_check, &log_flush, NULL);

    NIC_LOG_INFO("Entering event loop");
    evutil_run(EV_DEFAULT);
}

int main(int argc, char *argv[])
{
    int opt;

    while ((opt = getopt(argc, argv, "c:p:")) != -1) {
        switch (opt) {
        case 'c':
            config_file = string(optarg);
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

    loop();

    return (0);
}
