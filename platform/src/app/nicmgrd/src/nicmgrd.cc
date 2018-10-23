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

#include "dev.hpp"
#include "eth_dev.hpp"
#include "accel_dev.hpp"
#include "hal_client.hpp"

#ifndef __x86_64__
#include "pci_ids.h"
#include "misc.h"
#include "bdf.h"
#include "cfgspace.h"
#include "pciehost.h"
#include "pciehdevices.h"
#include "pciehw.h"
#include "pcieport.h"
#endif

using namespace std;

uint16_t base_mac = 0x0a0a;

static int poll_enabled;
static DeviceManager *devmgr;
static string config_file;
enum ForwardingMode fwd_mode = FWD_MODE_CLASSIC_NIC;
platform_mode_t platform = PLATFORM_MODE_NONE;
extern void nicmgr_do_client_registration(void);

static void
polling_sighand(int s)
{
    printf("Stopping poll ...\n");
    poll_enabled = 0;
}

static void
sigusr1_handler(int sig)
{
    fflush(stdout);
    fflush(stderr);
}

typedef struct pciemgrenv_s {
    pciehdev_t *current_dev;
    u_int8_t enabled_ports;
    pcieport_t *pport[PCIEPORT_NPORTS];
} pciemgrenv_t;

static pciemgrenv_t pciemgrenv;

static u_int64_t __attribute__((used))
timestamp(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec);
}

static pciemgrenv_t *
pciemgrenv_get(void)
{
    return &pciemgrenv;
}

static void
loop()
{
    pciemgrenv_t *pme = pciemgrenv_get();
    pciehdev_openparams_t p;
    useconds_t polltm_us = 10000;

    memset(&p, 0, sizeof(p));

    /*
     * For simulation we want the virtual upstream port bridge
     * at 00:00.0, but on "real" systems the upstream port bridge
     * is in hw and our first virtual device is 00:00.0.
     */
#ifdef __aarch64__
    p.first_bus = 0;
#else
    p.first_bus = 1;
    p.fake_bios_scan = 1;
#endif
    p.inithw = 1;
    p.subdeviceid = PCI_SUBDEVICE_ID_PENSANDO_NAPLES100;
    p.enabled_ports = 0x1;
    pme->enabled_ports = 0x1;

    for (int port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->enabled_ports & (1 << port)) {
            pcieport_t *pport;

            if ((pport = pcieport_open(port)) == NULL) {
                printf("pcieport_open %d failed\n", port);
                exit(1);
            }
            if (pcieport_ctrl(pport, PCIEPORT_CMD_HOSTCONFIG, NULL) < 0) {
                printf("pcieport_ctrl(HOSTCONFIG) %d failed\n", port);
                exit(1);
            }

            pme->pport[port] = pport;
        }
    }

    if (pciehdev_open(&p) < 0) {
        printf("pciehdev_open failed\n");
        exit(1);
    }

    if (pciehdev_initialize() < 0) {
        printf("pciehdev_initialize failed\n");
        exit(1);
    }

    devmgr = new DeviceManager(fwd_mode, platform);
    devmgr->LoadConfig(config_file);

    // Register for PCI events
#ifdef __aarch64__
    if (pciehdev_register_event_handler(&devmgr->PcieEventHandler) < 0) {
        printf("[ERROR] Failed to register PCIe Event Handler\n");
        exit(1);
    }
#endif

    pciehdev_finalize();

#ifdef __aarch64__
    int off = 0;
    for (int port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->pport[port]) {
            pcieport_ctrl(pme->pport[port], PCIEPORT_CMD_CRS, &off);
        }
    }
#endif
    printf("Polling enabled every %dus, ^C to exit...\n", polltm_us);

    while (poll_enabled) {
#ifdef __aarch64__
        u_int64_t tm_start, tm_stop, tm_port;

        tm_start = timestamp();
        for (int port = 0; port < PCIEPORT_NPORTS; port++) {
            if (pme->pport[port]) {
                pcieport_poll(pme->pport[port]);
            }
        }
        tm_port = timestamp();
        pciehw_poll();
        tm_stop = timestamp();

        if (tm_port - tm_start > 1000000) {
            printf("pcieport_poll: %ldus\n", tm_port - tm_start);
        }
        if (tm_stop - tm_port > 1000000) {
            printf("pciehw_poll: %ldus\n", tm_stop - tm_port);
        }
#endif
        devmgr->DevcmdPoll();
        devmgr->AdminQPoll();

        fflush(stdout);
        fflush(stderr);

        if (polltm_us) usleep(polltm_us);
    }

    printf("Polling stopped\n");

    pciehdev_close();

    for (int port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->pport[port]) {
            pcieport_close(pme->pport[port]);
        }
    }
}

int main(int argc, char *argv[])
{
    int opt;
    sighandler_t osigint, osigterm, osigquit, osigusr1;

    poll_enabled = 1;

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
                platform = PLATFORM_MODE_SIM;
            } else if (string(optarg) == "hw") {
                platform = PLATFORM_MODE_HW;
            } else if (string(optarg) == "haps") {
                platform = PLATFORM_MODE_HAPS;
            } else if (string(optarg) == "rtl") {
                platform = PLATFORM_MODE_RTL;
            } else if (string(optarg) == "mock") {
                platform = PLATFORM_MODE_MOCK;
            } else  {
                platform = PLATFORM_MODE_NONE;
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

    osigint  = signal(SIGINT,  polling_sighand);
    osigterm = signal(SIGTERM, polling_sighand);
    osigquit = signal(SIGQUIT, polling_sighand);
    osigusr1 = signal(SIGUSR1, sigusr1_handler);

    nicmgr_do_client_registration();
    loop();

    signal(SIGINT,  osigint);
    signal(SIGTERM, osigterm);
    signal(SIGQUIT, osigquit);
    signal(SIGQUIT, osigusr1);

    return (0);
}

