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

#include "pal.h"
#include "pcieport.h"
#include "pcieport_impl.h"

pcieport_info_t pcieport_info;

static int
pcieport_info_init(void)
{
    pcieport_info_t *pi = &pcieport_info;

    if (pi->init) {
        /* already initialized */
        return 0;
    }
    pi->init = 1;
    return 0;
}

pcieport_t *
pcieport_open(const int port)
{
    pcieport_info_t *pi = &pcieport_info;
    pcieport_t *p;

    assert(port < PCIEPORT_NPORTS);
    if (pcieport_info_init() < 0) {
        return NULL;
    }
    p = &pi->pcieport[port];
    if (p->open) {
        return NULL;
    }
    p->port = port;
    p->open = 1;
    p->host = 0;
    p->config = 0;
    return p;
}

void
pcieport_close(pcieport_t *p)
{
    if (p->open) {
        p->open = 0;
    }
}
