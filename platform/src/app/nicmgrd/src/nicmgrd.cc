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

#include "nic/sdk/include/sdk/thread.hpp"
#include "platform/src/lib/evutils/include/evutils.h"
#include "platform/src/lib/nicmgr/include/dev.hpp"
#include "platform/src/lib/pciemgr_if/include/pciemgr_if.hpp"
#include "platform/src/lib/pciemgr/include/pciehw_dev.h"
#include "platform/src/lib/pcieport/include/pcieport.h"

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

void *nicmgrd_create_mnets(void *ctxt)
{
    // Walk through mnets and do create_mnet
    cout << "Creating mnets ..." << endl;
    devmgr->CreateMnets();
    cout << "Successfully created mnets!!" << endl;

    return NULL;
}


#define NICMGRD_THREAD_ID_MNET 0
static void
nicmgrd_mnet_thread_init()
{
#ifdef __aarch64__
     sdk::lib::thread    *mnet_thread = NULL;

     sdk::lib::thread::control_cores_mask_set(0x8);

     mnet_thread = sdk::lib::thread::factory(std::string("mnet-thread").c_str(),
                                             NICMGRD_THREAD_ID_MNET,
                                             sdk::lib::THREAD_ROLE_CONTROL,
                                             0x8,
                                             nicmgrd_create_mnets,
                                             sched_get_priority_max(SCHED_OTHER),
                                             SCHED_OTHER,
                                             false); // yield
     if (mnet_thread == NULL) {
         cerr << "Unable to start mnet thread. Exiting!!" << endl;
         exit(1);
     }

     cout << "Starting mnet thread ... " << endl;
     // Starting mnet thread
     mnet_thread->start(mnet_thread);
#endif
}


static void
loop()
{
    if (platform_is_hw(platform)) {
        pciemgr = new class pciemgr("nicmgrd");
        pciemgr->initialize();
    }

    devmgr = new DeviceManager(fwd_mode, platform, dol_integ);
    devmgr->LoadConfig(config_file);

    if (pciemgr) {
        pciemgr->finalize();
    }

    // Initialize Mnet thread to create mnets
    nicmgrd_mnet_thread_init();

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

    // instantiate the logger
    utils::logger::init(dol_integ);
    if (platform_is_hw(platform)) {
        nicmgr_delphi_client_entry(NULL);
    }
    loop();

    signal(SIGUSR1, osigusr1);

    return (0);
}
