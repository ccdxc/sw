/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <cinttypes>
#include <sys/time.h>

#include "cmd.h"
#include "pcieutilpd.h"

static void
pmt(int argc, char *argv[])
{
    if (pciehdev_open(NULL) < 0) {
        printf("pciehdev_open failed\n");
        exit(1);
    }

    pciehw_pmt_show(argc, argv);

    pciehdev_close();
}
CMDFUNC(pmt,
"all valid pcie match table tcam entries",
"pmt [-bcr]\n"
"    -b         include bar PMT entries\n"
"    -c         include cfg PMT entries\n"
"    -r         raw format\n");
