/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "pciesys.h"
#include "pciehw_impl.h"

void
pciehw_event(pciehwdev_t *phwdev, const pciehdev_eventdata_t *evd)
{
    pciehdev_event(evd);
}
