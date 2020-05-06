/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>

#include "cmd.h"
#include "pcieutilpd.h"

static void
hdrt(int argc, char *argv[])
{
    if (pciehdev_open(NULL) < 0) {
        printf("pciehdev_open failed\n");
        exit(1);
    }

    pciehw_hdrt_show(argc, argv);

    pciehdev_close();
}
CMDFUNC(hdrt,
"hdrt details",
"hdrt\n");
