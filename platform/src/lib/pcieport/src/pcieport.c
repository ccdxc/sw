/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "pci_ids.h"
#include "pal.h"
#include "pciesys.h"
#include "pciehw_dev.h"
#include "pcieport.h"
#include "pcieport_impl.h"

pcieport_info_t pcieport_info;

static pcieport_t *
pcieport_get(const int port)
{
    pcieport_info_t *pi = &pcieport_info;
    pcieport_t *p = NULL;

    if (port >= 0 && port < PCIEPORT_NPORTS) {
        p = &pi->pcieport[port];
    }
    return p;
}

int
pcieport_open(const int port)
{
    pcieport_info_t *pi = &pcieport_info;
    pcieport_t *p;
    int otrace;

    otrace = pal_reg_trace_control(getenv("PCIEPORT_INIT_TRACE") != NULL);
    pal_reg_trace("================ pcieport_open %d start\n", port);

    if (port >= PCIEPORT_NPORTS) {
        pciesys_logerror("pcieport_open port %d out of range\n", port);
        return -EBADF;
    }
    if (pcieport_onetime_init() < 0) {
        return -EIO;
    }
    p = &pi->pcieport[port];
    if (p->open) {
        return -EBUSY;
    }
    p->port = port;
    p->open = 1;
    p->host = 0;
    p->config = 0;
    pal_reg_trace("================ pcieport_open %d end\n", port);
    pal_reg_trace_control(otrace);
    return 0;
}

void
pcieport_close(const int port)
{
    pcieport_t *p = pcieport_get(port);

    if (p && p->open) {
        p->open = 0;
    }
}

static int
pcieport_validate_hostconfig(pcieport_t *p)
{
    switch (p->cap_gen) {
    case 1:
    case 2:
    case 3:
    case 4:
        /* all good */
        break;
    default:
        pciesys_logerror("port %d unsupported gen%d\n", p->port, p->cap_gen);
        return -EFAULT;
    }

    switch (p->cap_width) {
    case 1: /* x1 uses 2 lanes */
    case 2:
        /* XXX verify peer isn't also configured to use our lanes */
        break;
    case 4:
        /* odd ports don't support x4 */
        if (p->port & 0x1) {
            goto bad_width;
        }
        /* XXX verify peer isn't also configured to use our lanes */
        break;
    case 8:
        /* only ports 0,4 can support x8 */
        if (p->port != 0 && p->port != 4) {
            goto bad_width;
        }
        break;
    case 16:
        /* only port 0 can use all 16 lanes */
        if (p->port != 0) {
            goto bad_width;
        }
        break;
    default:
        pciesys_logerror("port %d unsupported x%d\n", p->port, p->cap_width);
        return -ERANGE;
    }
    return 0;

 bad_width:
    pciesys_logerror("port %d doesn't support x%d\n", p->port, p->cap_width);
    return -EINVAL;
}

static int
pcieport_parse_cap(char *cap, int *gen, int *width)
{
    if (sscanf(cap, "gen%dx%d", gen, width) == 2) {
        if (*gen >= 1 && *gen <= 4 &&
            *width >= 1 && *width <= 16) {
            return 1;
        }
    }
    return 0;
}

static int
pcieport_getenv_cap(int port, int *gen, int *width)
{
    char envar[40];
    char *env;

    snprintf(envar, sizeof(envar), "PCIEPORT%d_CAP", port);
    env = getenv(envar);
    if (env && pcieport_parse_cap(env, gen, width)) {
        pciesys_loginfo("pcieport: $%s selects gen%dx%d\n",
                        envar, *gen, *width);
        return 1;
    }

    snprintf(envar, sizeof(envar), "PCIEPORT_CAP");
    env = getenv(envar);
    if (env && pcieport_parse_cap(env, gen, width)) {
        pciesys_loginfo("pcieport: $%s selects gen%dx%d\n",
                        envar, *gen, *width);
        return 1;
    }

    snprintf(envar, sizeof(envar), "PCIE_CAP");
    env = getenv(envar);
    if (env && pcieport_parse_cap(env, gen, width)) {
        pciesys_loginfo("pcieport: $%s selects gen%dx%d\n",
                        envar, *gen, *width);
        return 1;
    }

    return 0;
}

static void
pcieport_default_cap(pcieport_t *p, int *gen, int *width)
{
    /* check envar for override */
    if (!pcieport_getenv_cap(p->port, gen, width)) {
        /* no envar, provide defaults */
        *gen = pal_is_asic() ? 4 : 1;
        *width = pal_is_asic() ? 16 : 4;
    }
}

static int
pcieport_getenv_compliance(int *compliance)
{
    char *env = getenv("PCIE_COMPLIANCE");
    if (env) {
        *compliance = strtoul(env, NULL, 0);
        pciesys_loginfo("pcieport: $PCIE_COMPLIANCE override %d\n",
                        *compliance);
        return 1;
    }
    return 0;
}

int
pcieport_hostconfig(const int port, const pciehdev_params_t *params)
{
    pcieport_t *p = pcieport_get(port);
    int r, default_gen, default_width, compliance;

    if (p == NULL) {
        return -EBADF;
    }

    pcieport_default_cap(p, &default_gen, &default_width);

    if (params) {
        p->cap_gen = params->cap_gen;
        p->cap_width = params->cap_width;
        p->subvendorid = params->subvendorid;
        p->subdeviceid = params->subdeviceid;
        p->compliance = params->compliance;
        p->sris = params->sris;
    }

    /*
     * Provide default params for any unspecified.
     */
    if (p->cap_gen == 0) {
        p->cap_gen = default_gen;
    }
    if (p->cap_width == 0) {
        p->cap_width = default_width;
    }
    if (p->subvendorid == 0) {
        p->subvendorid = PCI_VENDOR_ID_PENSANDO;
    }
    if (p->subdeviceid == 0) {
        p->subdeviceid = PCI_SUBDEVICE_ID_PENSANDO_NAPLES100;
    }
    if (pcieport_getenv_compliance(&compliance)) {
        p->compliance = compliance;
    }

    /*
     * Verify the requested config is valid.
     */
    if ((r = pcieport_validate_hostconfig(p)) < 0) {
        return r;
    }

    switch (p->cap_width) {
    case  1: /* x1 uses 2 lanes */
    case  2: p->lanemask = 0x0003 << (p->port << 0); break;
    case  4: p->lanemask = 0x000f << (p->port << 1); break;
    case  8: p->lanemask = 0x00ff << (p->port << 2); break;
    case 16: p->lanemask = 0xffff << (p->port << 4); break;
    }

    p->host = 1;
    p->config = 1;

    return 0;
}

int
pcieport_crs_off(const int port)
{
    pcieport_t *p = pcieport_get(port);

    if (p == NULL)  return -EBADF;
    if (!p->config) return -EIO;
    if (!p->host)   return -EINVAL;
    p->crs = 0;
    pcieport_set_crs(p, p->crs);
    return 0;
}

/******************************************************************
 * debug
 */

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
        pciesys_loginfo("vga_support %d\n", vga_support);
        return;
    }
    vga_support = strtoul(argv[1], NULL, 0);
}

static void
cmd_port(int argc, char *argv[])
{
    pcieport_info_t *pi = &pcieport_info;
    pcieport_t *p;
    const int w = 20;
    int port;

    if (argc <= 1) {
        pciesys_loginfo("Usage: port <n>\n");
        return;
    }
    port = strtoul(argv[1], NULL, 0);
    if (port < 0 || port >= PCIEPORT_NPORTS) {
        pciesys_logerror("port %d out of range\n", port);
        return;
    }

    p = &pi->pcieport[port];

#define LOG(args, ...) \
    pciesys_loginfo(args, ##__VA_ARGS__)
    LOG("%-*s: cap gen%dx%d\n", w, "config", p->cap_gen,p->cap_width);
    LOG("%-*s: cur gen%dx%d\n", w, "current",p->cur_gen,p->cur_width);
    LOG("%-*s: 0x%04x\n", w, "lanemask", p->lanemask);
    LOG("%-*s: 0x%04x\n", w, "subvendorid", p->subvendorid);
    LOG("%-*s: 0x%04x\n", w, "subdeviceid", p->subdeviceid);
    LOG("%-*s: %d\n", w, "open", p->open);
    LOG("%-*s: %d\n", w, "config", p->open);
    LOG("%-*s: %d\n", w, "crs", p->crs);
    LOG("%-*s: %d\n", w, "state", p->state);
    LOG("%-*s: %d\n", w, "event", p->event);
    LOG("%-*s: %s\n", w, "fault_reason", p->fault_reason);
    LOG("%-*s: %s\n", w, "last_fault_reason", p->last_fault_reason);
    LOG("%-*s: %"PRIu64"\n", w, "hostup", p->hostup);
    LOG("%-*s: %"PRIu64"\n", w, "phypolllast", p->phypolllast);
    LOG("%-*s: %"PRIu64"\n", w, "phypollmax", p->phypollmax);
    LOG("%-*s: %"PRIu64"\n", w, "phypollperstn", p->phypollperstn);
    LOG("%-*s: %"PRIu64"\n", w, "phypollfail", p->phypollfail);
    LOG("%-*s: %"PRIu64"\n", w, "gatepolllast", p->gatepolllast);
    LOG("%-*s: %"PRIu64"\n", w, "gatepollmax", p->gatepollmax);
    LOG("%-*s: %"PRIu64"\n", w, "markerpolllast", p->markerpolllast);
    LOG("%-*s: %"PRIu64"\n", w, "markerpollmax", p->markerpollmax);
    LOG("%-*s: %"PRIu64"\n", w, "axipendpolllast",p->axipendpolllast);
    LOG("%-*s: %"PRIu64"\n", w, "axipendpollmax", p->axipendpollmax);
#undef LOG
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
        pciesys_loginfo("Usage: pcieport <subcmd>\n");
        return;
    }

    c = cmd_lookup(cmdtab, argv[1]);
    if (c == NULL) {
        pciesys_logerror("%s: %s not found\n", argv[0], argv[1]);
        return;
    }
    c->f(argc - 1, argv + 1);
}
