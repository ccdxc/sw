/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "platform/pal/include/pal.h"
#include "platform/intrutils/include/intrutils.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "pciehw_impl.h"

void
pciehw_reset_hostdn(pciehwdev_t *phwdev)
{
    const u_int32_t intrb = phwdev->intrb;
    const u_int32_t intrc = phwdev->intrc;
    const int dmask = phwdev->intrdmask;

    intr_reset_bus(intrb, intrc, dmask);
    /* XXX reset cfg/bars */
}
