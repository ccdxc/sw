/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>

#include "platform/misc/include/bdf.h"
#include "nic/sdk/platform/pciemgr/include/pciemgr.h"
#include "nic/sdk/platform/pcieport/include/pcieport.h"
#include "nic/sdk/platform/pciemgr/include/pciehw.h"
#include "nic/sdk/platform/pciemgr/include/pciehw_dev.h"
#include "cmd.h"
#include "utils.hpp"

static void
dev(int argc, char *argv[])
{
    int opt, port, venid, devid;

    port = default_pcieport();
    venid = 0;
    devid = 0;
    optind = 0;
    while ((opt = getopt(argc, argv, "ad:D:p:")) != -1) {
        switch (opt) {
        case 'D': {
            int n = sscanf(optarg, "%x:%x", &venid, &devid);
            if (n != 2) {
                fprintf(stderr, "%s: invalid dev spec\n", optarg);
                exit(1);
            }
            break;
        }
        case 'p':
            port = strtoul(optarg, NULL, 0);
            break;
        default:
            break;
        }
    }

    if (pciehdev_open(NULL) < 0) {
        printf("pciehdev_open failed\n");
        exit(1);
    }

    /*
     * -D make it easy for scripts to get the mgmt device bus in decimal.
     * If secbus==0 then we haven't been initialized yet.
     */
    if (venid && devid) {
        pcieport_t *p = pcieport_get(port);
        if (p && p->secbus) {
            pciehwdev_t *phwdev = pciehwdev_get_by_id(port, venid, devid);
            if (phwdev) {
                const uint16_t bdf = pciehwdev_get_hostbdf(phwdev);
                const uint8_t bus = bdf_to_bus(bdf);
                printf("%d\n", bus);
            }
        }
        exit(0);
    }

    pciehw_dev_show(argc, argv);

    pciehdev_close();
}
CMDFUNC(dev,
        "provisioned pcie device info",
        "dev [-a]\n"
        "    -a         all devices, including virtual bridges"
        "    -d <dev>   detailed device info for <dev>\n");

static void
devintr(int argc, char *argv[])
{
    if (pciehdev_open(NULL) < 0) {
        printf("pciehdev_open failed\n");
        exit(1);
    }

    pciehw_devintr_show(argc, argv);

    pciehdev_close();
}
CMDFUNC(devintr,
"device interrupt state",
"devintr [-d <dev>]\n"
"    -d <dev>   display intrs for <dev> (default all devices)\n");
