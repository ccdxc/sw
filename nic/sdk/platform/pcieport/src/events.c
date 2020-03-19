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

#include "platform/misc/include/misc.h"
#include "platform/pciemgrutils/include/pciesys.h"
#include "platform/pciemgr/include/pciemgr.h"
#include "pcieport.h"
#include "portcfg.h"
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
pcieport_event_linkup(pcieport_t *p, const int genid)
{
    pcieport_event_t ev;
    pcieport_event_linkup_t *linkup;

    memset(&ev, 0, sizeof(ev));
    ev.type = PCIEPORT_EVENT_LINKUP;
    ev.port = p->port;
    linkup = &ev.linkup;
    linkup->gen = p->cur_gen;
    linkup->width = p->cur_width;
    linkup->reversed = p->cur_reversed;
    linkup->genid = genid;
    send_event(&ev);
}

void
pcieport_event_linkdn(pcieport_t *p, const int genid)
{
    pcieport_event_t ev;
    pcieport_event_linkdn_t *linkdn;

    memset(&ev, 0, sizeof(ev));
    ev.type = PCIEPORT_EVENT_LINKDN;
    ev.port = p->port;
    linkdn = &ev.linkdn;
    linkdn->genid = genid;
    send_event(&ev);
}

void
pcieport_event_hostup(pcieport_t *p, const int genid)
{
    pcieport_event_t ev;
    pcieport_event_hostup_t *hostup;

    memset(&ev, 0, sizeof(ev));
    ev.type = PCIEPORT_EVENT_HOSTUP;
    ev.port = p->port;
    hostup = &ev.hostup;
    hostup->gen = p->cur_gen;
    hostup->width = p->cur_width;
    hostup->reversed = p->cur_reversed;
    hostup->genid = genid;
    hostup->lnksta2 = portcfg_readw(p->port, PORTCFG_CAP_PCIE + 0x32);
    send_event(&ev);
}

void
pcieport_event_hostdn(pcieport_t *p, const int genid)
{
    pcieport_event_t ev;
    pcieport_event_hostdn_t *hostdn;

    memset(&ev, 0, sizeof(ev));
    ev.type = PCIEPORT_EVENT_HOSTDN;
    ev.port = p->port;
    hostdn = &ev.hostdn;
    hostdn->genid = genid;
    send_event(&ev);
}

void
pcieport_event_macup(pcieport_t *p, const int genid)
{
    pcieport_event_t ev;
    pcieport_event_macup_t *macup;

    memset(&ev, 0, sizeof(ev));
    ev.type = PCIEPORT_EVENT_MACUP;
    ev.port = p->port;
    macup = &ev.macup;
    macup->genid = genid;
    send_event(&ev);
}

void
pcieport_event_buschg(pcieport_t *p)
{
    pcieport_event_t ev;
    pcieport_event_buschg_t *buschg;

    memset(&ev, 0, sizeof(ev));
    ev.type = PCIEPORT_EVENT_BUSCHG;
    ev.port = p->port;
    buschg = &ev.buschg;
    buschg->pribus = p->pribus;
    buschg->secbus = p->secbus;
    buschg->subbus = p->subbus;
    send_event(&ev);
}

void
pcieport_event_fault(pcieport_t *p)
{
    pcieport_event_t ev;
    pcieport_event_fault_t *fault;

    memset(&ev, 0, sizeof(ev));
    ev.type = PCIEPORT_EVENT_FAULT;
    ev.port = p->port;
    fault = &ev.fault;
    strncpy0(fault->reason, p->fault_reason, sizeof(fault->reason));
    send_event(&ev);
}

int
pcieport_register_event_handler(pcieport_event_handler_t h, void *arg)
{
    pcieport_event_info_t *evinfo = &pcieport_event_info;
    pcieport_handler_info_t *newhandlers, *hi;
    int n;

    n = evinfo->nhandlers + 1;
    newhandlers = pciesys_realloc(evinfo->handlers, n * sizeof(*newhandlers));
    if (newhandlers == NULL) return -1;

    evinfo->handlers = newhandlers;
    hi = &evinfo->handlers[evinfo->nhandlers];
    hi->handler = h;
    hi->arg = arg;
    evinfo->nhandlers++;

    return 0;
}
