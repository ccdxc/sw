/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <inttypes.h>

#include "misc.h"
#include "pal.h"
#include "pciehost.h"
#include "cfgspace.h"
#include "pciehw.h"
#include "pciehw_impl.h"
#include "pciehsys.h"
#include "bdf.h"

static pciehw_t pciehw;

pciehw_t *
pciehw_get(void)
{
    return &pciehw;
}

pciehw_mem_t *
pciehw_get_hwmem(pciehw_t *phw)
{
    return phw->pciehwmem;
}

pciehwdev_t *
pciehwdev_get(pciehwdevh_t hwdevh)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    return hwdevh > 0 && hwdevh < PCIEHW_NDEVS ? &phwmem->dev[hwdevh] : NULL;
}

pciehwdevh_t
pciehwdev_geth(const pciehwdev_t *phwdev)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    return phwdev ? phwdev - phwmem->dev : 0;
}

/*
 * Avoid hw alignment fault when accessing name by carefully
 * copying into a static buf and returning a pointer to that.
 */
char *
pciehwdev_get_name(const pciehwdev_t *phwdev)
{
    static char name[32];
    pciehw_memcpy(name, phwdev->name, sizeof(name));
    return name;
}

pciehwdev_t *
pciehwdev_find_by_name(const char *name)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdev_t *phwdev;
    int i;

    phwdev = &phwmem->dev[1];
    for (i = 1; i <= phwmem->allocdev; i++, phwdev++) {
        if (strcmp(name, pciehwdev_get_name(phwdev)) == 0) {
            return phwdev;
        }
    }
    return NULL;
}

void
pciehwdev_get_cfgspace(const pciehwdev_t *phwdev, cfgspace_t *cs)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);

    cs->cur = phwmem->cfgcur[hwdevh];
    cs->msk = phwmem->cfgmsk[hwdevh];
    cs->size = PCIEHW_CFGSZ;
}

int
pciehwdev_cfgrd(pciehwdev_t *phwdev,
                const u_int16_t offset, const u_int8_t size, u_int32_t *valp)
{
    cfgspace_t cs;

    pciehwdev_get_cfgspace(phwdev, &cs);
    if (cfgspace_read(&cs, offset, size, valp) < 0) {
        return -EINVAL;
    }
    return 0;
}

int
pciehwdev_cfgwr(pciehwdev_t *phwdev,
                const u_int16_t offset,
                const u_int8_t size,
                const u_int32_t val)
{
    cfgspace_t cs;

    pciehwdev_get_cfgspace(phwdev, &cs);
    if (cfgspace_write(&cs, offset, size, val) < 0) {
        return -EINVAL;
    }
    return 0;
}

static int
pciehw_openregs(pciehw_t *phw)
{
#define PCIEHW_BASE     CAP_ADDR_BASE_PXB_PXB_OFFSET
#define PCIEHW_SIZE     CAP_PXB_CSR_BYTE_SIZE

    return 0;
}

int
pciehw_port_is_enabled(const int port)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);

    return (phwmem->enabled_ports & (1 << port)) != 0;
}

static void
pciehw_closeregs(pciehw_t *phw)
{
}

static int
pciehw_initmem(pciehw_mem_t *phwmem)
{
    pciehw_memset(phwmem, 0, sizeof(*phwmem));
    phwmem->version = PCIEHW_VERSION;
    phwmem->magic = PCIEHW_MAGIC;
    return 0;
}

static int
pciehw_openmem(pciehw_t *phw)
{
    pciehw_mem_t *pciehwmem;
    const char *pciehw_addr_env = getenv("PCIEHW_ADDR");
    u_int64_t pciehw_pa = 0x13c000000;

    if (pciehw_addr_env) {
        pciehw_pa = strtoull(pciehw_addr_env, NULL, 0);
        pciehsys_log("$PCIEHW_ADDR override 0x%"PRIx64"\n", pciehw_pa);
    }

    pciehwmem = pal_mem_map(pciehw_pa, sizeof(pciehw_mem_t));
    if (pciehwmem == NULL) {
        return -1;
    }
    phw->pciehwmem = pciehwmem;
    return 0;
}

static void
pciehw_closemem(pciehw_t *phw)
{
    pal_mem_unmap(phw->pciehwmem);
    phw->pciehwmem = NULL;
}

static void
pciehw_db_init(pciehw_t *phw)
{
#define DOORBELL_BASE \
    (CAP_ADDR_BASE_PXB_PXB_OFFSET + \
     CAP_PXB_CSR_CFG_TGT_DOORBELL_BASE_BYTE_OFFSET)
    union {
        struct {
            u_int32_t addr_33_24:10;
            u_int32_t db_host_sel:2;
            u_int32_t db_32b_sel:2;
        } __attribute__((packed));
        u_int32_t w;
    } e;

    e.addr_33_24 = 0x8;
    e.db_host_sel = 0x1;
    e.db_32b_sel = 0x3;

    pal_reg_wr32(DOORBELL_BASE, e.w);
}

static void
pciehw_init(pciehw_t *phw)
{
    pciehw_intrhw_init(phw);
    pciehw_db_init(phw);
    pciehw_cfg_init(phw);
    pciehw_bar_init(phw);
    pciehw_vfstride_init(phw);
    pciehw_tgt_port_init(phw);
    pciehw_itr_port_init(phw);
    pciehw_hdrt_init(phw);
    pciehw_portmap_init(phw);
    pciehw_notify_init(phw);
    pciehw_indirect_init(phw);
}

int
pciehw_open(pciehw_params_t *hwparams)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem;
    int r;

    /* already open */
    if (phw->open) {
        phw->clients++;
        return 0;
    }
    phw->nports = PCIEHW_NPORTS;
    if (hwparams) {
        phw->hwparams = *hwparams;
    }

    if ((r = pciehw_openregs(phw)) < 0) {
        return r;
    }
    if ((r = pciehw_openmem(phw)) < 0) {
        pciehw_closeregs(phw);
        return r;
    }

    /* Is mem initialized? */
    phwmem = pciehw_get_hwmem(phw);
    if (phw->hwparams.inithw) {
        pciehw_initmem(phwmem);
    } else if (phwmem->magic != PCIEHW_MAGIC) {
        return -EINVAL;
    }

    /* Do we understand this version? */
    if (phwmem->version != PCIEHW_VERSION) {
        pciehw_closeregs(phw);
        pciehw_closemem(phw);
        return -ENOEXEC;
    }

    if (phw->hwparams.inithw) {
        phwmem->enabled_ports = hwparams ? hwparams->enabled_ports : 0x5;
        pciehw_init(phw);
    }

    phw->open = 1;
    phw->clients++;
    return 0;
}

void
pciehw_close(void)
{
    pciehw_t *phw = pciehw_get();

    if (phw->open) {
        if (--phw->clients) {
            pciehw_closemem(phw);
            phw->open = 0;
        }
    }
}

static pciehwdev_t *
pciehwdev_alloc(pciehdev_t *pdev)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    const char *name = pciehdev_get_name(pdev);
    pciehwdev_t *phwdev;

    assert(phwmem->allocdev < PCIEHW_NDEVS - 1);
    phwdev = &phwmem->dev[++phwmem->allocdev];
    pciehw_memset(phwdev, 0, sizeof(*phwdev));
    pciehw_memcpy(phwdev->name, name, strlen(name) + 1);
    pciehdev_set_hwdev(pdev, phwdev);
    return phwdev;
}

void
pciehw_initialize_topology(const u_int8_t port)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);

    if (phwmem) {
        /* XXX these are global, not per-port */
        phwmem->allocdev = 0;
        phwmem->allocprt = 0;

        phwmem->rooth[port] = 0;
    }
}

static pciehwdev_t *
pciehwdev_search_bdf(pciehwdevh_t phwdevh, const u_int16_t bdf)
{
    pciehwdev_t *phwdev, *phwdev_found;
    int bus;

    if (phwdevh == 0) {
        return NULL;
    }

    phwdev = pciehwdev_get(phwdevh);

    /*
     * As the BIOS (or QEMU) scans the bus and assigns bus number
     * to bridges to fit its bus topology, the BDF that we assigned
     * to phwdev->bdf at "finalize" time might no longer be correct
     * because the parent bridge secondary bus number can get reassigned.
     * We get the parent bridge secondary bus number here and use
     * that to compare for a match so we can "follow" the new bus
     * topology for our BDFs.
     */

    bus = -1;
    if (phwdev->parenth) {
        pciehwdev_t *parent_hwdev = pciehwdev_get(phwdev->parenth);
        u_int32_t hdrt, sec_bus;

        if (pciehwdev_cfgrd(parent_hwdev, 0xe, 1, &hdrt) == 0 &&
            pciehwdev_cfgrd(parent_hwdev, 0x19, 1, &sec_bus) == 0) {
            if ((hdrt & 0x7f) == 1) {
                bus = sec_bus;
            }
        }
    } else {
        bus = 0;
    }
    assert(bus != -1);

    /* looking for this bdf? */
    if (bdf_to_bus(bdf) == bus &&
        bdf_to_dev(bdf) == bdf_to_dev(phwdev->bdf) &&
        bdf_to_fnc(bdf) == bdf_to_fnc(phwdev->bdf)) {
        return phwdev;
    }

    /* check peer */
    phwdev_found = pciehwdev_search_bdf(phwdev->peerh, bdf);
    if (phwdev_found) {
        return phwdev_found;
    }
    /* check child */
    phwdev_found = pciehwdev_search_bdf(phwdev->childh, bdf);
    if (phwdev_found) {
        return phwdev_found;
    }
    return NULL;
}

static pciehwdev_t *
pciehwdev_by_bdf(const u_int8_t port, const u_int16_t bdf)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdev_t *phwdev, *phwdev_found;
    pciehwdevh_t phwdevh;

    if (phwmem == NULL) {
        return NULL;
    }
    phwdev_found = NULL;
    for (phwdevh = phwmem->rooth[port]; phwdevh; phwdevh = phwdev->peerh) {
        phwdev_found = pciehwdev_search_bdf(phwdevh, bdf);
        if (phwdev_found) {
            break;
        }
        phwdev = pciehwdev_get(phwdevh);
    }
    return phwdev_found;
}

static pciehwdev_t *
pciehw_finalize_dev(pciehdev_t *pdev)
{
    pciehw_t *phw = pciehw_get();
    pciehwdev_t *phwdev = pciehwdev_alloc(pdev);
    pciehdev_t *parent, *peer, *child;
    int lif;

    phwdev->pdev = pdev;
    phwdev->bdf = pciehdev_get_bdf(pdev);
    phwdev->port = pciehdev_get_port(pdev);
    phwdev->intrb = pciehdev_get_intrb(pdev);
    phwdev->intrc = pciehdev_get_intrc(pdev);

    lif = pciehdev_get_lif(pdev);
    if (lif >= 0) {
        phwdev->lif_valid = 1;
        phwdev->lif = lif;
    }
    parent = pciehdev_get_parent(pdev);
    phwdev->parenth = parent ? pciehwdev_geth(pciehdev_get_hwdev(parent)) : 0;

    /*
     * Don't load the root into hardware.  root represents the
     * upstream port bridge which is provided by hardware so no
     * need to add to hw tables to virtualize.
     */
    if (parent) {
        /* bar first, the cfg below */
        pciehw_bar_finalize(pdev);
        if (phwdev->lif_valid) {
            pciehw_hdrt_load(phw, phwdev->lif, phwdev->bdf);
            pciehw_portmap_load(phw, phwdev->lif, phwdev->port);
        }
    }

    /* cfg after bar above */
    pciehw_cfg_finalize(pdev);

    /* init_init() now that we have a parent link and cfg */
    pciehw_intr_init(phwdev);

    child = pciehdev_get_child(pdev);
    if (child) {
        pciehwdev_t *phwchild = pciehw_finalize_dev(child);
        phwdev->childh = pciehwdev_geth(phwchild);
    }

    peer = pciehdev_get_peer(pdev);
    if (peer) {
        pciehwdev_t *phwpeer = pciehw_finalize_dev(peer);
        phwdev->peerh = pciehwdev_geth(phwpeer);
    }

    return phwdev;
}

static void
fake_bios_scan(const u_int8_t port, int bus, int *nextbus)
{
    int dev, bdf;
    u_int32_t val;

    for (dev = 0; dev < 32; dev++) {
        bdf = bdf_make(bus, dev, 0);

        /* read vendor/device id to be sure a device exists at bdf */
        if (pciehw_cfgrd(port, bdf, 0, 4, &val) < 0) {
            continue;
        }
        /* read header type register */
        if (pciehw_cfgrd(port, bdf, 0xe, 1, &val) < 0) {
            continue;
        }
        /* bridge header type? */
        if ((val & 0x7f) == 1) {
            /* set secondary bus number */
            int secbus = (*nextbus)++;
            pciehw_cfgwr(port, bdf, 0x19, 1, secbus);
            fake_bios_scan(port, secbus, nextbus); /* scan secondary bus */
        }
    }
}

void
pciehw_finalize_topology(pciehdev_t *proot)
{
    pciehw_t *phw = pciehw_get();
    pciehw_params_t *phwparams = &phw->hwparams;
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdev_t *phwroot = proot ? pciehw_finalize_dev(proot) : NULL;
    const u_int8_t port = pciehdev_get_port(proot);

    phwmem->rooth[port] = pciehwdev_geth(phwroot);
    if (phwparams->fake_bios_scan) {
        int nextbus = 1;
        fake_bios_scan(port, 0, &nextbus);
    }
}

/******************************************************************
 * debug
 */

static void
cmd_dev(int argc, char *argv[])
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdev_t *phwdev;
    char lifstr[8];
    int i;

    pciehsys_log("%-3s %-16s %-9s %-4s %-4s\n",
                 "hdl", "name", "p:bdf", "intx", "lif");
    phwdev = &phwmem->dev[1];
    for (i = 1; i <= phwmem->allocdev; i++, phwdev++) {
        lifstr[0] = '\0';
        if (phwdev->lif_valid) {
            snprintf(lifstr, sizeof(lifstr), "%d", phwdev->lif);
        }
        pciehsys_log("%3d %-16s %1d:%-7s    %c %4s\n",
                     pciehwdev_geth(phwdev),
                     pciehwdev_get_name(phwdev),
                     phwdev->port,
                     bdf_to_str(phwdev->bdf),
                     phwdev->intrc ? "ABCD"[phwdev->intpin] : ' ',
                     lifstr);
    }
}

static void
cmd_bar(int argc, char *argv[])
{
    pciehw_bar_dbg(argc, argv);
}

static void
cmd_pmt(int argc, char *argv[])
{
    pciehw_pmt_dbg(argc, argv);
}

static void
cmd_prt(int argc, char *argv[])
{
    pciehw_prt_dbg(argc, argv);
}

static void
cmd_romsk(int argc, char *argv[])
{
    pciehw_romsk_dbg(argc, argv);
}

static void
cmd_hdrt(int argc, char *argv[])
{
    pciehw_hdrt_dbg(argc, argv);
}

static void
cmd_portmap(int argc, char *argv[])
{
    pciehw_portmap_dbg(argc, argv);
}

static void
cmd_notify(int argc, char *argv[])
{
    pciehw_notify_dbg(argc, argv);
}

static void
cmd_indirect(int argc, char *argv[])
{
    pciehw_indirect_dbg(argc, argv);
}

static void
cmd_meminfo(int argc, char *argv[])
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    u_int64_t hwmempa = pal_mem_vtop(phwmem);
    const int w = 16;

    pciehsys_log("%-*s: 0x%08"PRIx64" size %lu\n", w, "physaddr",
                 hwmempa, sizeof(pciehw_mem_t));
    pciehsys_log("%-*s: 0x%08x\n", w, "magic", phwmem->magic);
    pciehsys_log("%-*s: %d\n", w, "version", phwmem->version);
    pciehsys_log("%-*s: 0x%x\n", w, "enabled_ports", phwmem->enabled_ports);
    pciehsys_log("%-*s: %d\n", w, "allocdev", phwmem->allocdev);
    pciehsys_log("%-*s: %d\n", w, "allocprt", phwmem->allocprt);
    pciehsys_log("%-*s: 0x%08"PRIx64" size %lu\n", w, "cfgcur",
                 hwmempa + offsetof(pciehw_mem_t, cfgcur),
                 sizeof(phwmem->cfgcur));
    pciehsys_log("%-*s: 0x%08"PRIx64" size %lu\n", w, "notify_area",
                 hwmempa + offsetof(pciehw_mem_t, notify_area),
                 sizeof(phwmem->notify_area));
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
    CMDENT(dev, "dev", ""),
    CMDENT(bar, "bar", ""),
    CMDENT(pmt, "pmt", ""),
    CMDENT(prt, "prt", ""),
    CMDENT(romsk, "romsk", ""),
    CMDENT(hdrt, "hdrt", ""),
    CMDENT(portmap, "portmap", ""),
    CMDENT(meminfo, "meminfo", ""),
    CMDENT(notify, "notify", ""),
    CMDENT(indirect, "indirect", ""),
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
pciehw_dbg(int argc, char *argv[])
{
    cmd_t *c;

    if (argc < 2) {
        pciehsys_log("Usage: pciehw <subcmd>\n");
        return;
    }

    c = cmd_lookup(cmdtab, argv[1]);
    if (c == NULL) {
        pciehsys_log("%s: %s not found\n", argv[0], argv[1]);
        return;
    }
    c->f(argc - 1, argv + 1);
}

int
pciehw_cfgrd(const u_int8_t port, const u_int16_t bdf,
             const u_int16_t offset, const u_int8_t size, u_int32_t *valp)
{
    pciehwdev_t *phwdev;

    phwdev = pciehwdev_by_bdf(port, bdf);
    if (phwdev == NULL) {
        return -ESRCH;
    }
    return pciehwdev_cfgrd(phwdev, offset, size, valp);
}

int
pciehw_cfgwr(const u_int8_t port, const u_int16_t bdf,
             const u_int16_t offset, const u_int8_t size, const u_int32_t val)
{
    pciehwdev_t *phwdev;

    phwdev = pciehwdev_by_bdf(port, bdf);
    if (phwdev == NULL) {
        return -ESRCH;
    }
    return pciehwdev_cfgwr(phwdev, offset, size, val);
}

static u_int64_t memval;
static u_int32_t ioval;

int
pciehw_memrd(const u_int8_t port,
             const u_int64_t addr, const u_int8_t size, u_int64_t *valp)
{
    *valp = memval;
    return 0;
}

int
pciehw_memwr(const u_int8_t port,
             const u_int64_t addr, const u_int8_t size, u_int64_t val)
{
    memval = val;
    return 0;
}

int
pciehw_iord(const u_int8_t port,
            const u_int32_t addr, const u_int8_t size, u_int32_t *valp)
{
    *valp = ioval;
    return 0;
}

int
pciehw_iowr(const u_int8_t port,
            const u_int32_t addr, const u_int8_t size, u_int32_t val)
{
    ioval = val;
    return 0;
}

int
pciehw_barsz(const u_int8_t port, const u_int16_t bdf, const int i)
{
    pciehwdev_t *phwdev;
    pciehwbar_t *phwbar;

    phwdev = pciehwdev_by_bdf(port, bdf);
    if (phwdev == NULL) {
        return 0;
    }
    phwbar = &phwdev->bar[i];
    return pciehw_bar_getsz(phwbar);
}

pciehw_params_t *
pciehw_get_params(void)
{
    pciehw_t *phw = pciehw_get();
    return &phw->hwparams;
}

int
pciehw_poll(void)
{
    pciehw_t *phw = pciehw_get();

    pciehw_indirect_poll(phw);
    pciehw_notify_poll(phw);
    return 0;
}

void *
pciehw_memset(void *s, int c, size_t n)
{
    if (((uintptr_t)s & 0x3) == 0 && (n & 0x3) == 0) {
        volatile u_int32_t *p;
        int i;

        c &= 0xff;
        c = ((c << 0) |
             (c << 8) |
             (c << 16) |
             (c << 24));
        for (p = s, i = 0; i < n >> 2; i++, p++) {
            *p = c;
        }
    } else {
        volatile u_int8_t *p;
        int i;

        for (p = s, i = 0; i < n; i++, p++) {
            *p = c;
        }
    }

    return s;
}

void *
pciehw_memcpy(void *dst, const void *src, size_t n)
{
    volatile u_int8_t *d = dst;
    const u_int8_t *s = src;
    int i;

    for (i = 0; i < n; i++) {
        *d++ = *s++;
    }
    return dst;
}
