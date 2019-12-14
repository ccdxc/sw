/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <endian.h>
#include <fcntl.h>
#include <assert.h>
#include <inttypes.h>
#include <sys/param.h>

#include "cap_top_csr_defines.h"
#include "cap_pp_c_hdr.h"

#include "platform/pal/include/pal.h"
#include "platform/pcietlp/include/pcietlp.h"
#include "platform/pciemgr/include/pciemgr.h"
#include "pcieport.h"
#include "portmap.h"

typedef struct portmapenv_s {
    int portmap_is_init;
    uint32_t portmask;
    uint32_t hostmask;
    pcieport_spec_t portspec[PCIEPORT_NPORTS];
    int pcieport_rmap[PCIEPORT_NPORTS];
} portmapenv_t;

static portmapenv_t portmapenv;

static int
valid_spec(const pcieport_spec_t *ps)
{
    if (ps->port < 0 || ps->port >= PCIEPORT_NPORTS) return 0;
    if (ps->host < 0 || ps->host >= PCIEPORT_NPORTS) return 0;
    if (ps->gen <= 0 || ps->gen > 4) return 0;
    if (ps->width <= 0 || ps->width > 16) return 0;
    return 1;
}

int
portmap_init(void)
{
    memset(&portmapenv, 0, sizeof(portmapenv));
    return 0;
}

int
portmap_addhost(const pcieport_spec_t *ps)
{
    portmapenv_t *pme = &portmapenv;

    if (!valid_spec(ps)) return -1;
    if (pme->portmask & (1 << ps->port)) return -1;
    if (pme->hostmask & (1 << ps->host)) return -1;

    pme->portspec[ps->port] = *ps;
    pme->pcieport_rmap[ps->host] = ps->port;
    pme->portmask |= (1 << ps->port);
    pme->hostmask |= (1 << ps->host);
    pme->portmap_is_init = 1;
    return 0;
}

uint32_t
portmap_portmask(void)
{
    portmapenv_t *pme = &portmapenv;

    if (!pme->portmap_is_init) return 0;
    return pme->portmask;
}

uint32_t
portmap_hostmask(void)
{
    portmapenv_t *pme = &portmapenv;

    if (!pme->portmap_is_init) return 0;
    return pme->hostmask;
}

int
portmap_getspec(const int port, pcieport_spec_t *ps)
{
    portmapenv_t *pme = &portmapenv;

    if (!pme->portmap_is_init) return -1;
    if (port < 0 || port >= PCIEPORT_NPORTS) return -1;
    if (pme->portspec[port].gen == 0) return -1;

    *ps = pme->portspec[port];
    return 0;
}

int
portmap_hostport(const int pcieport)
{
    portmapenv_t *pme = &portmapenv;

    if (!pme->portmap_is_init) return -1;
    if (pcieport < 0 || pcieport >= PCIEPORT_NPORTS) return -1;
    if (pme->portspec[pcieport].gen == 0) return -1;

    return pme->portspec[pcieport].host;
}

int
portmap_pcieport(const int hostport)
{
    portmapenv_t *pme = &portmapenv;

    if (!pme->portmap_is_init) return -1;
    if (hostport < 0 || hostport >= PCIEPORT_NPORTS) return -1;
    if ((pme->hostmask & (1 << hostport)) == 0) return -1;

    return pme->pcieport_rmap[hostport];
}

void
portmap_foreach_port(portmap_cb_t callbackf, void *arg)
{
    uint32_t portmask = portmap_portmask();
    int port;

    for (port = 0; portmask && (portmask & (1 << port)); port++) {
        callbackf(port, arg);
        portmask &= ~(1 << port);
    }
}

void
portmap_foreach_host(portmap_cb_t callbackf, void *arg)
{
    uint32_t hostmask = portmap_hostmask();
    int host;

    for (host = 0; hostmask && (hostmask & (1 << host)); host++) {
        callbackf(host, arg);
        hostmask &= ~(1 << host);
    }
}
