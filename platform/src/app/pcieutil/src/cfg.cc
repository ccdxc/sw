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
cfg(int argc, char *argv[])
{
    if (pciehdev_open(NULL) < 0) {
        printf("pciehdev_open failed\n");
        exit(1);
    }

    pciehw_cfg_show(argc, argv);

    pciehdev_close();
}
CMDFUNC(cfg,
"device cfg details",
"cfg [-d <dev>][-cmrx]\n"
"    -d <dev>   display cfgspace for <dev> (default all devices)\n"
"    -c         display current cfgspace (default)\n"
"    -m         display cfgspace write-mask\n"
"    -r         display cfgspace reset values\n"
"    -x         like lspci -x show more cfgspace (default -x)\n");
