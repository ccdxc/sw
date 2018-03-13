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
#include "pciehsys.h"
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
    pal_reg_wr32(PP_(CFG_PP_LINKWIDTH), 0x2222); /* 4 port x4 linkwidth mode */
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


static void
cmd_fsm(int argc, char *argv[])
{
    pcieport_fsm_dbg(argc, argv);
}

int vga_support;

static void
cmd_vga_support(int argc, char *argv[])
{
    if (argc < 2) {
        pciehsys_log("vga_support %d\n", vga_support);
        return;
    }
    vga_support = strtoul(argv[1], NULL, 0);
}

static void
cmd_port(int argc, char *argv[])
{
    pcieport_info_t *pi = &pcieport_info;
    pcieport_t *p;
    const int w = 16;
    int port;

    if (argc <= 1) {
        pciehsys_log("Usage: port <n>\n");
        return;
    }
    port = strtoul(argv[1], NULL, 0);
    if (port < 0 || port > PCIEPORT_NPORTS) {
        pciehsys_log("port %d out of range\n", port);
        return;
    }

    p = &pi->pcieport[port];
    pciehsys_log("%-*s: gen%dx%d\n", w, "config", p->gen, p->width);
    pciehsys_log("%-*s: 0x%04x\n", w, "lanemask", p->lanemask);
    pciehsys_log("%-*s: 0x%04x\n", w, "subvendorid", p->subvendorid);
    pciehsys_log("%-*s: 0x%04x\n", w, "subdeviceid", p->subdeviceid);
    pciehsys_log("%-*s: %d\n", w, "open", p->open);
    pciehsys_log("%-*s: %d\n", w, "config", p->open);
    pciehsys_log("%-*s: %d\n", w, "crs", p->crs);
    pciehsys_log("%-*s: %d\n", w, "state", p->state);
    pciehsys_log("%-*s: %d\n", w, "event", p->event);
    pciehsys_log("%-*s: %"PRIu64"\n", w, "linkup", p->linkup);
}

typedef struct cmd_s {
    const char *name;
    void (*f)(int argc, char *argv[]);
    const char *desc;
    const char *helpstr;
} cmd_t;

static cmd_t cmdtab[] = {
#define CMDENT(name, desc, helpstr) \
    { #name, cmd_##name, desc, helpstr }
    CMDENT(fsm, "fsm", ""),
    CMDENT(port, "port", ""),
    CMDENT(vga_support, "vga_support", ""),
    { NULL, NULL }
};

static cmd_t *
cmd_lookup(cmd_t *cmdtab, const char *name)
{
    cmd_t *c;

    for (c = cmdtab; c->name; c++) {
        if (strcmp(c->name, name) == 0) {
            return c;
        }
    }
    return NULL;
}

void
pcieport_dbg(int argc, char *argv[])
{
    cmd_t *c;

    if (argc < 2) {
        pciehsys_log("Usage: pcieport <subcmd>\n");
        return;
    }

    c = cmd_lookup(cmdtab, argv[1]);
    if (c == NULL) {
        pciehsys_log("%s: %s not found\n", argv[0], argv[1]);
        return;
    }
    c->f(argc - 1, argv + 1);
}
