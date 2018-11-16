/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>

#include "platform/src/lib/pciemgr/include/pciehw.h"
#include "platform/src/lib/pciemgr/include/pciehw_dev.h"
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
CMDFUNC(dev, "dev");
