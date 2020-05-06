/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/time.h>

#include "platform/pcieport/include/pcieport.h"
#include "pcieport_impl.h"

void
pcieport_intr_inherit(pcieport_t *p)
{
    pcieportpd_intr_inherit(p);
}

int
pcieport_intr(const int port)
{
    return pcieportpd_check_for_intr(port);
}

int
pcieport_poll(const int port)
{
    int r = pcieportpd_check_for_intr(port);
    if (r >= 0) {
        pcieport_t *p = pcieport_get(port);
        if (p) p->stats.intr_polled++;
    }
    return r;
}

void
pcieport_intr_enable(const int port)
{
    pcieportpd_intr_enable(port);
}

void
pcieport_intr_disable(const int port)
{
    pcieportpd_intr_disable(port);
}

int
pcieport_intr_init(const int port)
{
    return pcieportpd_intr_init(port);
}

int
pcieport_poll_init(const int port)
{
    return pcieportpd_poll_init(port);
}
