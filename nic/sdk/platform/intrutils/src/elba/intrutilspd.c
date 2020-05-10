/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <inttypes.h>

#include "platform/intrutils/include/intrutils.h"
#include "intrutilspd.h"

void
intrpd_hwinit(const u_int32_t clock_freq)
{
    intrpd_coal_init(clock_freq);
}
