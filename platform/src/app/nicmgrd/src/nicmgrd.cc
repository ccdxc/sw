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
static enum DeviceType pciehdev_type = INVALID;

#define NUM_ETH_DEVICES     6

struct eth_devspec eth_dev[NUM_ETH_DEVICES] = {
    {
        .vrf_id = 1,
        .uplink = 1,
        .native_vlan = 1,
        .sg_id = 0,
        .mac_addr = 0x000200000101,
        .ip_addr = 0x0,
        .lif_id = 101,
        .enic_id = 101,
        //
        .rxq_base = 0,
        .rxq_count = 1,
        .txq_base = 0,
        .txq_count = 1,
        .adminq_base = 0,
        .adminq_count = 1,
        //rdma config
        .rdma_sq_size = 0,
        .rdma_sq_count = 0,
        .rdma_rq_size = 0,
        .rdma_rq_count = 0,
        .rdma_cq_size = 0,
        .rdma_cq_count = 0,
        .rdma_eq_size = 0,
        .rdma_eq_count = 0,
        //
        .intr_base = 0,
        .intr_count = 8,
        .eq_base = 0,
        .eq_count = 0,
        .rdma_pid_count = 0,
        //
        .host_dev = 0,
        .pcie_port = 255,
        //
        .enable_rdma = 0,
        .max_pt_entries = 0,
        .max_keys = 0,
    },
    {
        .vrf_id = 1,
        .uplink = 2,
        .native_vlan = 1,
        .sg_id = 0,
        .mac_addr = 0x000200000102,
        .ip_addr = 0,
        .lif_id = 102,
        .enic_id = 102,
        //
        .rxq_base = 0,
        .rxq_count = 1,
        .txq_base = 0,
        .txq_count = 1,
        .adminq_base = 0,
        .adminq_count = 1,
        //rdma config
        .rdma_sq_size = 0,
        .rdma_sq_count = 0,
        .rdma_rq_size = 0,
        .rdma_rq_count = 0,
        .rdma_cq_size = 0,
        .rdma_cq_count = 0,
        .rdma_eq_size = 0,
        .rdma_eq_count = 0,
        //
        .intr_base = 256,
        .intr_count = 8,
        .eq_base = 0,
        .eq_count = 0,
        .rdma_pid_count = 0,
        //
        .host_dev = 0,
        .pcie_port = 255,
        //
        .enable_rdma = 0,
        .max_pt_entries = 0,
        .max_keys = 0,
    },
    {
        .vrf_id = 1,
        .uplink = 1,
        .native_vlan = 1,
        .sg_id = 0,
        .mac_addr = 0x000200000103,
        .ip_addr = 0,
        .lif_id = 103,
        .enic_id = 103,
        //
        .rxq_base = 0,
        .rxq_count = 8,
        .txq_base = 0,
        .txq_count = 8,
        .adminq_base = 0,
        .adminq_count = 1,
        //rdma config
        .rdma_sq_size = 1024,
        .rdma_sq_count = 512,
        .rdma_rq_size = 1024,
        .rdma_rq_count = 512,
        .rdma_cq_size = 64,
        .rdma_cq_count = 256,
        .rdma_eq_size = 32,
        .rdma_eq_count = 512,
        //
        .intr_base = 512,
        .intr_count = 32,
        .eq_base = 0,
        .eq_count = 0,
        .rdma_pid_count = 2,
        //
        .host_dev = 1,
        .pcie_port = 0,
        //
        .enable_rdma = 1,
        .max_pt_entries = 32768,
        .max_keys = 32768,
    },
    {
        .vrf_id = 1,
        .uplink = 2,
        .native_vlan = 1,
        .sg_id = 0,
        .mac_addr = 0x000200000104,
        .ip_addr = 0,
        .lif_id = 104,
        .enic_id = 104,
        //
        .rxq_base = 0,
        .rxq_count = 8,
        .txq_base = 0,
        .txq_count = 8,
        .adminq_base = 0,
        .adminq_count = 1,
        //rdma config
        .rdma_sq_size = 1024,
        .rdma_sq_count = 512,
        .rdma_rq_size = 1024,
        .rdma_rq_count = 512,
        .rdma_cq_size = 64,
        .rdma_cq_count = 256,
        .rdma_eq_size = 32,
        .rdma_eq_count = 512,
        //
        .intr_base = 768,
        .intr_count = 32,
        .eq_base = 0,
        .eq_count = 0,
        .rdma_pid_count = 2,
        //
        .host_dev = 1,
        .pcie_port = 0,
        //
        .enable_rdma = 1,
        .max_pt_entries = 32768,
        .max_keys = 32768,
    },
    {
        .vrf_id = 1,
        .uplink = 1,
        .native_vlan = 1,
        .sg_id = 0,
        .mac_addr = 0x000200000105,
        .ip_addr = 0,
        .lif_id = 105,
        .enic_id = 105,
        //
        .rxq_base = 0,
        .rxq_count = 8,
        .txq_base = 0,
        .txq_count = 8,
        .adminq_base = 0,
        .adminq_count = 1,
        //rdma config
        .rdma_sq_size = 1024,
        .rdma_sq_count = 512,
        .rdma_rq_size = 1024,
        .rdma_rq_count = 512,
        .rdma_cq_size = 64,
        .rdma_cq_count = 256,
        .rdma_eq_size = 32,
        .rdma_eq_count = 512,
        //
        .intr_base = 1024,
        .intr_count = 32,
        .eq_base = 0,
        .eq_count = 0,
        .rdma_pid_count = 2,
        //
        .host_dev = 1,
        .pcie_port = 2,
        //
        .enable_rdma = 1,
        .max_pt_entries = 32768,
        .max_keys = 32768,
    },
    {
        .vrf_id = 1,
        .uplink = 2,
        .native_vlan = 1,
        .sg_id = 0,
        .mac_addr = 0x000200000106,
        .ip_addr = 0,
        .lif_id = 106,
        .enic_id = 106,
        //
        .rxq_base = 0,
        .rxq_count = 8,
        .txq_base = 0,
        .txq_count = 8,
        .adminq_base = 0,
        .adminq_count = 1,
        //rdma config
        .rdma_sq_size = 1024,
        .rdma_sq_count = 512,
        .rdma_rq_size = 1024,
        .rdma_rq_count = 512,
        .rdma_cq_size = 64,
        .rdma_cq_count = 256,
        .rdma_eq_size = 32,
        .rdma_eq_count = 512,
        //
        .intr_base = 1280,
        .intr_count = 32,
        .eq_base = 0,
        .eq_count = 0,
        .rdma_pid_count = 2,
        //
        .host_dev = 1,
        .pcie_port = 2,
        //
        .enable_rdma = 1,
        .max_pt_entries = 32768,
        .max_keys = 32768,
    }
};

#define ACCEL_NUM_DEVICES     1

accel_devspec_t accel_devices[ACCEL_NUM_DEVICES] = {
    {
        .lif_id = STORAGE_SEQ_SW_LIF_ID,
        .seq_queue_base = 0,
        .seq_queue_count = 8192,
        .seq_created_count = 0,
        .adminq_base = 0,
        .adminq_count = 1,
        //
        .intr_base = 1536,
        .intr_count = 64,
        //
        .pcie_port = 0,
        .enable_pciehdev_create = 0,
    }
};

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
    int off = 0;
    pciemgrenv_t *pme = pciemgrenv_get();
    pciehdev_openparams_t p;
    useconds_t polltm_us = 10000;

    memset(&p, 0, sizeof(p));

    /*
     * For simulation we want the virtual upstream port bridge
     * at 00:00.0, but on "real" systems the upstream port bridge
     * is in hw and our first virtual device is 00:00.0.
     */
    p.first_bus = 0;
    p.inithw = 1;
    p.subdeviceid = PCI_SUBDEVICE_ID_PENSANDO_NAPLES100;
    p.enabled_ports = 0x5;
    pme->enabled_ports = 0x5;

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

    for (int i = 0; i < NUM_ETH_DEVICES; i++) {
        eth_dev[i].mac_addr |= (base_mac << 16);
        devmgr->AddDevice(ETH_PF, (void *)&eth_dev[i]);
    }

    for (int i = 0; i < ACCEL_NUM_DEVICES; i++) {
        if (pciehdev_type == ACCEL) {
            accel_devices[i].enable_pciehdev_create = 1;
        }
        devmgr->AddDevice(ACCEL, (void *)&accel_devices[i]);
    }

    // Register for PCI events
#ifdef __aarch64__
    if (pciehdev_register_event_handler(&devmgr->PcieEventHandler) < 0) {
        printf("[ERROR] Failed to register PCIe Event Handler\n");
        exit(1);
    }
#endif

    pciehdev_finalize();

    for (int port = 0; port < PCIEPORT_NPORTS; port++) {
        if (pme->pport[port]) {
            pcieport_ctrl(pme->pport[port], PCIEPORT_CMD_CRS, &off);
        }
    }
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

static enum DeviceType
dev_str_to_type(const char *dev_str)
{
    enum DeviceType type;

    if (strcmp(dev_str, "eth") == 0) {
        type = ETH_PF;
    } else if (strcmp(dev_str, "eth_vf") == 0) {
        type =  ETH_VF;
    } else if (strcmp(dev_str, "accel") == 0) {
        type =  ACCEL;
    } else if (strcmp(dev_str, "nvme") == 0) {
        type =  NVME;
    } else if (strcmp(dev_str, "virtio") == 0) {
        type =  VIRTIO;
    } else if (strcmp(dev_str, "debug") == 0) {
        type =  DEBUG;
    } else {
        printf("[ERROR] unknown pciehdev type %s\n", dev_str);
        type = INVALID;
    }

    printf("[INFO] pciehdev type %s: %d\n", dev_str, type);
    return type;
}

int main(int argc, char *argv[])
{
    int opt;
    sighandler_t osigint, osigterm, osigquit, osigusr1;
    enum ForwardingMode fwd_mode = FWD_MODE_CLASSIC_NIC;

    while ((opt = getopt(argc, argv, "m:sp:")) != -1) {
        switch (opt) {
        case 'm':
            base_mac = atoi(optarg);
            break;
        case 's':
            fwd_mode = FWD_MODE_SMART_NIC;
            break;
        case 'p':

            /*
             * Unconditionally create this PCIe device type (if the
             * corresponding PF device supports it)
             */
            pciehdev_type = dev_str_to_type(optarg);
            break;
        default:
            exit(1);
        }
    }

    devmgr = new DeviceManager(fwd_mode);
    assert(devmgr->LoadConfig("") == 0);

    poll_enabled = 1;
    osigint  = signal(SIGINT,  polling_sighand);
    osigterm = signal(SIGTERM, polling_sighand);
    osigquit = signal(SIGQUIT, polling_sighand);
    osigusr1 = signal(SIGUSR1, sigusr1_handler);

    loop();

    signal(SIGINT,  osigint);
    signal(SIGTERM, osigterm);
    signal(SIGQUIT, osigquit);
    signal(SIGQUIT, osigusr1);

    return (0);
}
