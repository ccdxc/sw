/*
 * Copyright (c) 2018, Pensando Systems Inc.
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
bar(int argc, char *argv[])
{
    if (pciehdev_open(NULL) < 0) {
        printf("pciehdev_open failed\n");
        exit(1);
    }

    pciehw_bar_show(argc, argv);

    pciehdev_close();
}
CMDFUNC(bar,
"device bar details",
"bar [-v][-d <dev>][-b <idx>]\n"
"    -d <dev>   display bars for <dev> (default all devices)\n"
"    -b <idx>   display bar <idx> (default all bars)\n"
"    -v         verbose output\n");
