/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>

#include "nic/sdk/platform/pciemgr/include/pciehw.h"
#include "nic/sdk/platform/pciemgr/include/pciehw_dev.h"
#include "cmd.h"

static void
dev(int argc, char *argv[])
{
    if (pciehdev_open(NULL) < 0) {
        printf("pciehdev_open failed\n");
        exit(1);
    }

    pciehw_dev_show(argc, argv);

    pciehdev_close();
}
CMDFUNC(dev,
        "provisioned pcie device info",
        "dev [-a]\n"
        "    -a         all devices, including virtual bridges\n");

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
