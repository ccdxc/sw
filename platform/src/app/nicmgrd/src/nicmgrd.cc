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

#include "evutils.h"
#include "dev.hpp"
#include "eth_dev.hpp"
#include "accel_dev.hpp"
#include "hal_client.hpp"
#include "pciehw_dev.h"
#include "pcieport.h"
#include "pciemgr_if.hpp"
#include "delphic.hpp"

using namespace std;

uint16_t base_mac = 0x0a0a;

DeviceManager *devmgr;
pciemgr *pciemgr;
static string config_file;
enum ForwardingMode fwd_mode = FWD_MODE_CLASSIC_NIC;
platform_t platform = PLATFORM_NONE;
static bool dol_integ;
extern void nicmgr_do_client_registration(void);

static void
sigusr1_handler(int sig)
{
    fflush(stdout);
    fflush(stderr);
}

static void
nicmgrd_poll(void *arg)
{
    devmgr->DevcmdPoll();
    devmgr->AdminQPoll();

    fflush(stdout);
    fflush(stderr);
}

static void
loop()
{
    if (!dol_integ) {
        pciemgr = new class pciemgr("nicmgrd");
        pciemgr->initialize();
    }

    devmgr = new DeviceManager(fwd_mode, platform, dol_integ);
    devmgr->LoadConfig(config_file);

    if (pciemgr) {
        pciemgr->finalize();
    }

#if 0
    // Register for PCI events
#ifdef __aarch64__
    if (pciehdev_register_event_handler(&devmgr->PcieEventHandler) < 0) {
        printf("[ERROR] Failed to register PCIe Event Handler\n");
        exit(1);
    }
#endif
#endif

    evutil_timer timer;
    evutil_timer_start(&timer, nicmgrd_poll, NULL, 0.1, 0.1);

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
    const char  *dol_integ_str;

    dol_integ_str = std::getenv("DOL");
    dol_integ = dol_integ_str ? !!atoi(dol_integ_str) : false;

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

    //nicmgr_do_client_registration();
    if (!dol_integ) {
        nicmgr_delphi_client_entry(NULL);
    }
    loop();

    signal(SIGUSR1, osigusr1);

    return (0);
}
