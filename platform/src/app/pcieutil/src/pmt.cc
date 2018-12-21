/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>
#include <sys/time.h>

#include "platform/src/lib/misc/include/bdf.h"
#include "platform/src/lib/pciemgr/include/pciehw.h"
#include "platform/src/lib/pciemgr/include/pciehw_dev.h"
#include "platform/src/lib/pciemgrutils/include/pciemgrutils.h"
#include "nic/sdk/platform/pal/include/pal.h"

#include "cap_top_csr_defines.h"
#include "cap_pxb_c_hdr.h"
#include "cap_pp_c_hdr.h"
#include "cmd.h"

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
CMDFUNC(pmt, "pmt [-bcr]");
