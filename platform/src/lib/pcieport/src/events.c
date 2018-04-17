/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "pciehsys.h"
#include "pcieport.h"
#include "pcieport_impl.h"

typedef struct pcieport_handler_info_s {
    pcieport_event_handler_t handler;
    void *arg;
} pcieport_handler_info_t;

typedef struct pcieport_event_info_s {
    int nhandlers;
    pcieport_handler_info_t *handlers;
} pcieport_event_info_t;

static pcieport_event_info_t pcieport_event_info;

static void
send_event(pcieport_event_t *ev)
{
    pcieport_event_info_t *evinfo = &pcieport_event_info;
    int i;

    for (i = 0; i < evinfo->nhandlers; i++) {
        pcieport_handler_info_t *hi = &evinfo->handlers[i];

        if (hi->handler) {
            hi->handler(ev, hi->arg);
        }
    }
}

void
pcieport_event_hostup(pcieport_t *p, const int genid)
{
    pcieport_event_t ev;
    pcieport_event_hostup_t *hostup;

    memset(&ev, 0, sizeof(ev));
    ev.type = PCIEPORT_EVENT_HOSTUP;
    hostup = &ev.hostup;
    hostup->port = p->port;
    hostup->gen = p->cur_gen;
    hostup->width = p->cur_width;
    hostup->genid = genid;
    send_event(&ev);
}

void
pcieport_event_hostdn(pcieport_t *p, const int genid)
{
    pcieport_event_t ev;
    pcieport_event_hostdn_t *hostdn;

    memset(&ev, 0, sizeof(ev));
    ev.type = PCIEPORT_EVENT_HOSTDN;
    hostdn = &ev.hostdn;
    hostdn->port = p->port;
    hostdn->genid = genid;
    send_event(&ev);
}

void
pcieport_event_buschg(pcieport_t *p, const u_int8_t secbus)
{
    pcieport_event_t ev;
    pcieport_event_buschg_t *buschg;

    memset(&ev, 0, sizeof(ev));
    ev.type = PCIEPORT_EVENT_BUSCHG;
    buschg = &ev.buschg;
    buschg->port = p->port;
    buschg->secbus = secbus;
    send_event(&ev);
}

int
pcieport_register_event_handler(pcieport_event_handler_t h, void *arg)
{
    pcieport_event_info_t *evinfo = &pcieport_event_info;
    pcieport_handler_info_t *newhandlers, *hi;
    int n;

    n = evinfo->nhandlers + 1;
    newhandlers = pciehsys_realloc(evinfo->handlers, n * sizeof(*newhandlers));
    if (newhandlers == NULL) return -1;

    evinfo->handlers = newhandlers;
    hi = &evinfo->handlers[evinfo->nhandlers];
    hi->handler = h;
    hi->arg = arg;
    evinfo->nhandlers++;

    return 0;
}
