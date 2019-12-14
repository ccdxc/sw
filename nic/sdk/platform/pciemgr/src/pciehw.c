/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "platform/misc/include/misc.h"
#include "platform/misc/include/bdf.h"
#include "platform/pal/include/pal.h"
#include "platform/intrutils/include/intrutils.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "platform/cfgspace/include/cfgspace.h"
#include "platform/utils/mpart_rsvd.hpp"

#include "pciehw_impl.h"

#define MREGION_PCIEMGR_ADDR (MREGION_BASE_ADDR + MREGION_PCIEMGR_START_OFFSET)

static pciehw_t pciehw;

static pciehw_t *
pciehw_get(void)
{
    return &pciehw;
}

pciehw_mem_t *
pciehw_get_hwmem(void)
{
    pciehw_t *phw = pciehw_get();
    return phw->pciehwmem;
}

pciehw_shmem_t *
pciehw_get_shmem(void)
{
    pciehw_t *phw = pciehw_get();
    return phw->pcieshmem;
}

pciemgr_params_t *
pciehw_get_params(void)
{
    pciehw_t *phw = pciehw_get();
    return &phw->params;
}

pciehwdev_t *
pciehwdev_get(pciehwdevh_t hwdevh)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    return hwdevh > 0 && hwdevh < PCIEHW_NDEVS ? &pshmem->dev[hwdevh] : NULL;
}

pciehwdevh_t
pciehwdev_geth(const pciehwdev_t *phwdev)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    return phwdev ? phwdev - pshmem->dev : 0;
}

/*
 * Note this depends on all VFs being allocated contiguously,
 * which is true (at the moment).  If that assumption changes
 * we'll need another algorithm to find the VF from the PF.
 */
pciehwdev_t *
pciehwdev_getvf(pciehwdev_t *phwdev, const int vfidx)
{
    assert(vfidx >= 0 && vfidx < phwdev->totalvfs);
    return pciehwdev_get(phwdev->childh + vfidx);
}

const char *
pciehwdev_get_name(const pciehwdev_t *phwdev)
{
    return phwdev->name;
}

/*
 * Return a host bdf from a local bdf.
 */
u_int16_t
pciehw_hostbdf(const int port, const u_int16_t lbdf)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const pciehw_port_t *p = &pshmem->port[port];
    return bdf_make(bdf_to_bus(lbdf) + p->secbus,
                    bdf_to_dev(lbdf),
                    bdf_to_fnc(lbdf));
}

u_int16_t
pciehwdev_get_hostbdf(const pciehwdev_t *phwdev)
{
    return pciehw_hostbdf(phwdev->port, phwdev->bdf);
}

pciehwdev_t *
pciehwdev_find_by_name(const char *name)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehwdev_t *phwdev;
    int i;

    phwdev = &pshmem->dev[1];
    for (i = 1; i <= pshmem->allocdev; i++, phwdev++) {
        if (strcmp(name, pciehwdev_get_name(phwdev)) == 0) {
            return phwdev;
        }
    }
    return NULL;
}

void
pciehwdev_get_cfgspace(const pciehwdev_t *phwdev, cfgspace_t *cs)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem();
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);

    cs->cur = phwmem->cfgcur[hwdevh];
    cs->msk = pshmem->cfgmsk[hwdevh];
    cs->rst = pshmem->cfgrst[hwdevh];
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

static void
pciehw_db_init(void)
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
pciehw_hwinit(void)
{
    pciehw_intrhw_init();
    pciehw_db_init();
    pciehw_cfg_init();
    pciehw_bar_init();
    pciehw_vfstride_init();
    pciehw_tgt_port_init();
    pciehw_itr_port_init();
    pciehw_hdrt_init();
    pciehw_portmap_init();
    pciehw_notify_init();
    pciehw_indirect_init();
}

typedef void (*pciehw_cb_t)(pciehwdev_t *phwdev, void *cbarg);

void
pciehw_foreach(int port, pciehw_cb_t cb, void *cbarg)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehwdevh_t phwdevh;
    pciehwdev_t *phwdev;

    /*
     * XXX Use better data structures for per-port searches.
     */
    phwdev = &pshmem->dev[1];
    for (phwdevh = 1; phwdevh <= pshmem->allocdev; phwdevh++, phwdev++) {
        if (phwdev->port == port) {
            cb(phwdev, cbarg);
        }
    }
}

struct hostup_args {
    int gen;
    int width;
};

static void
pciehw_hostup(pciehwdev_t *phwdev, void *arg)
{
    struct hostup_args *a = arg;
    cfgspace_t cs;

    pciehwdev_get_cfgspace(phwdev, &cs);
    cfgspace_update(&cs, a->gen, a->width);
}

void
pciehw_event_hostup(const int port, const int gen, const int width)
{
    struct hostup_args a = { .gen = gen, .width = width };

    pciehw_foreach(port, pciehw_hostup, &a);
}

void
pciehw_event_hostdn(const int port)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehwdev_t *phwroot = pciehwdev_get(pshmem->rooth[port]);
    pciehw_port_t *p = &pshmem->port[port];

    if (phwroot) {
        pciehw_reset_bus(phwroot, p->secbus);
    }
}

void
pciehw_event_buschg(const int port, const u_int8_t secbus)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehw_port_t *p = &pshmem->port[port];

    p->secbus = secbus;
}

static int
pciehw_memmap_hwmem(const pciemgr_initmode_t initmode)
{
    pciehw_t *phw = pciehw_get();
    const char *pciehw_addr_env = getenv("PCIEHW_ADDR");
    u_int64_t pciehw_pa = roundup(MREGION_PCIEMGR_ADDR, 1024*1024);
    pciehw_mem_t *pciehwmem;

    if (pciehw_addr_env) {
        pciehw_pa = strtoull(pciehw_addr_env, NULL, 0);
        pciesys_loginfo("$PCIEHW_ADDR override 0x%"PRIx64"\n", pciehw_pa);
    }

    assert((MREGION_PCIEMGR_ADDR + MREGION_PCIEMGR_SIZE -
           pciehw_pa) >= sizeof(pciehw_mem_t));

    pciehwmem = pal_mem_map(pciehw_pa, sizeof(pciehw_mem_t), MATTR_UNCACHED);
    if (pciehwmem == NULL) {
        pciesys_logerror("failed to map hwmem at 0x%"PRIx64" size %ld\n",
                         pciehw_pa, sizeof(pciehw_mem_t));
        return -1;
    }
    phw->pciehwmem = pciehwmem;
    return 0;
}

static void
pciehw_memunmap_hwmem(void)
{
    pciehw_t *phw = pciehw_get();

    if (phw->pciehwmem) {
        pal_mem_unmap(phw->pciehwmem);
        phw->pciehwmem = NULL;
    }
}

static int
pciehw_memmap_shmem(const pciemgr_initmode_t initmode)
{
    pciehw_t *phw = pciehw_get();
    char *env = getenv("PCIEMGR_DATA");
    char path[PATH_MAX];
    int oflags;
    pciehw_shmem_t *pcieshmem;

    /*
     * Find the default path, either from $PCIEMGR_DATA
     * or /var/run/pciemgr_data on ARM,
     * or $HOME/.pciemgr_data on x86_64.
     */
    if (env) {
        strncpy0(path, env, sizeof(path));
        pciesys_loginfo("PCIEMGR_DATA override %s\n", path);
    } else {
#ifdef __aarch64__
        strncpy(path, "/var/run/pciemgr_data", sizeof(path));
#else
        env = getenv("HOME");
        snprintf(path, sizeof(path), "%s/.pciemgr_data", env ? env : "");
#endif
    }

    /*
     * If FORCE_INIT, just rm the datafile and we'll behave as if
     * there was no saved state.
     */
    if (initmode == FORCE_INIT) {
        (void)unlink(path);
    }

    oflags = O_RDWR;
    if (initmode == INHERIT_OK || initmode == FORCE_INIT) {
        oflags |= O_CREAT;
    }
    pcieshmem = mapfile(path, sizeof(pciehw_shmem_t), oflags);
    if (pcieshmem == NULL) {
        pciesys_logerror("%s: %s\n", path, strerror(errno));
        return -1;
    }
    phw->pcieshmem = pcieshmem;
    return 0;
}

static void
pciehw_memunmap_shmem(void)
{
    pciehw_t *phw = pciehw_get();

    if (phw->pcieshmem) {
        munmap(phw->pcieshmem, sizeof(pciehw_shmem_t));
        phw->pcieshmem = NULL;
    }
}

static int
pciehw_memmap(const pciemgr_initmode_t initmode)
{
    int r;

    r = pciehw_memmap_hwmem(initmode);
    if (r < 0) {
        return r;
    }
    r = pciehw_memmap_shmem(initmode);
    if (r < 0) {
        pciehw_memunmap_hwmem();
        return r;
    }
    return 0;
}

static void
pciehw_memunmap(void)
{
    pciehw_memunmap_hwmem();
    pciehw_memunmap_shmem();
}

static void
pciehw_meminit_hwmem(void)
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem();

    pciehw_memset(phwmem, 0, sizeof(*phwmem));
    phwmem->version = PCIEHW_VERSION;
    phwmem->magic = PCIEHW_MAGIC;
}

static void
pciehw_meminit_shmem(void)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();

    memset(pshmem, 0, sizeof(*pshmem));
    pshmem->version = PCIEHW_VERSION;
    pshmem->magic = PCIEHW_MAGIC;
}

static void
pciehw_meminit(void)
{
    pciehw_meminit_hwmem();
    pciehw_meminit_shmem();
}

static int
pciehw_memopen(const pciemgr_initmode_t initmode)
{
    pciehw_mem_t *phwmem;
    pciehw_shmem_t *pshmem;
    int r;

    r = pciehw_memmap(initmode);
    if (r < 0) {
        return r;
    }

    phwmem = pciehw_get_hwmem();
    pshmem = pciehw_get_shmem();

    if (pshmem->magic == 0 && pshmem->version == 0) {
        /* Uninitialized state, initialize here (unless INHERIT_ONLY) */
        if (initmode == INHERIT_ONLY) {
            /*
             * This case is not reached.
             * If INHERIT_ONLY and the mmap'd file is not there
             * we'll fail at pciehw_memmap() above, but we'll handled
             * it here for completeness.
             */
            pciesys_logerror("memopen: inherit_only but no shmem\n");
            pciehw_memunmap();
            return -1;
        }
        pciehw_meminit();
#ifdef __aarch64__
        pciesys_loginfo("memopen: init magic %x/%x version %d/%d\n",
                        phwmem->magic, pshmem->magic,
                        phwmem->version, pshmem->version);
#endif
    } else if (initmode == FORCE_INIT) {
        pciehw_meminit();
#ifdef __aarch64__
        pciesys_loginfo("memopen: force init magic %x/%x version %d/%d\n",
                        phwmem->magic, pshmem->magic,
                        phwmem->version, pshmem->version);
#endif
    } else if (phwmem->magic != PCIEHW_MAGIC ||
               pshmem->magic != PCIEHW_MAGIC ||
               phwmem->version != PCIEHW_VERSION ||
               pshmem->version != PCIEHW_VERSION) {
        if (initmode == INHERIT_ONLY) {
            pciesys_logerror("memopen: bad magic %x/%x (want %x) "
                             "version %d/%d (want %d)\n",
                             phwmem->magic, pshmem->magic, PCIEHW_MAGIC,
                             phwmem->version, pshmem->version, PCIEHW_VERSION);
            pciehw_memunmap();
            return -1;
        }
        pciesys_logwarn("memopen: reinit bad magic %x/%x (want %x) "
                        "version %d/%d (want %d)\n",
                        phwmem->magic, pshmem->magic, PCIEHW_MAGIC,
                        phwmem->version, pshmem->version, PCIEHW_VERSION);
        pciehw_meminit();
    } else {
        /* don't log anything here, pcieutil uses this mode */
    }
    return 0;
}

static void
pciehw_memclose(void)
{
    pciehw_memunmap();
}

static void
pciehw_set_initmode(void)
{
    pciehw_t *phw = pciehw_get();
    char *env = getenv("PCIEHW_INITMODE");

    if (env) {
        if (strcmp(env, "inherit_only") == 0) {
            phw->params.initmode = INHERIT_ONLY;
        } else if (strcmp(env, "inherit_ok") == 0) {
            phw->params.initmode = INHERIT_OK;
        } else if (strcmp(env, "force_init") == 0) {
            phw->params.initmode = FORCE_INIT;
        }
        pciesys_loginfo("PCIEHW_INITMODE override %d\n", phw->params.initmode);
    }
}

int
pciehw_open(pciemgr_params_t *params)
{
    pciehw_t *phw = pciehw_get();
    pciehw_shmem_t *pshmem;
    int r;

    pciehw_struct_size_checks();

    /* already open */
    if (phw->open) {
        phw->clients++;
        return 0;
    }
    if (params) {
        phw->params = *params;
    }

    pciehw_set_initmode();

    r = pciehw_memopen(phw->params.initmode);
    if (r < 0) return r;

    pshmem = pciehw_get_shmem();

    if (phw->params.initmode == FORCE_INIT || !pshmem->hwinit) {
        pciehw_hwinit();
        pshmem->hwinit = 1;
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
        if (--phw->clients == 0) {
            pciehw_notify_disable_all_ports();
            pciehw_indirect_disable_all_ports();
            pciehw_memclose();
            phw->open = 0;
        }
    }
}

static pciehwdev_t *
pciehwdev_alloc(pciehdev_t *pdev)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    const char *name = pciehdev_get_name(pdev);
    pciehwdev_t *phwdev;

    assert(pshmem->allocdev < PCIEHW_NDEVS - 1);
    phwdev = &pshmem->dev[++pshmem->allocdev];
    memset(phwdev, 0, sizeof(*phwdev));
    strncpy0(phwdev->name, name, sizeof(phwdev->name));
    pciehdev_set_hwdev(pdev, phwdev);
    return phwdev;
}

void
pciehw_initialize_topology(const u_int8_t port)
{
    pciehw_t *phw = pciehw_get();
    pciehw_shmem_t *pshmem = pciehw_get_shmem();

    if (pshmem) {
        /* XXX these are global, not per-port */
        if (phw->params.initmode != INHERIT_ONLY) {
            pshmem->allocdev = 0;
            pshmem->allocpmt = 0;
            pshmem->allocprt = 0;
            pshmem->rooth[port] = 0;
        }
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

        /*
         * If our parent is a bridge, then our bus is the
         * bridge's secondary bus.
         *
         * If not a bridge, we are a VF and our bus is
         * our parent's bus.
         */
        if (pciehwdev_cfgrd(parent_hwdev, 0xe, 1, &hdrt) == 0 &&
            (hdrt & 0x7f) == 1) {
            pciehwdev_cfgrd(parent_hwdev, 0x19, 1, &sec_bus);
            bus = sec_bus;
        } else {
            bus = bdf_to_bus(parent_hwdev->bdf);
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
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehwdev_t *phwdev, *phwdev_found;
    pciehwdevh_t phwdevh;

    if (pshmem == NULL) {
        return NULL;
    }
    phwdev_found = NULL;
    for (phwdevh = pshmem->rooth[port]; phwdevh; phwdevh = phwdev->peerh) {
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
    pciehwdev_t *phwdev = pciehwdev_alloc(pdev);
    pciehdev_t *parent, *peer, *child;

    phwdev->pf = pciehdev_is_pf(pdev);
    phwdev->vf = pciehdev_is_vf(pdev);
    phwdev->totalvfs = pciehdev_get_totalvfs(pdev);
    phwdev->vfidx = pciehdev_get_vfidx(pdev);
    phwdev->bdf = pciehdev_get_bdf(pdev);
    phwdev->port = pciehdev_get_port(pdev);
    phwdev->lifb = pciehdev_get_lifb(pdev);
    phwdev->lifc = pciehdev_get_lifc(pdev);
    phwdev->intrb = pciehdev_get_intrb(pdev);
    phwdev->intrc = pciehdev_get_intrc(pdev);
    phwdev->intrdmask = pciehdev_get_intrm(pdev);

    parent = pciehdev_get_parent(pdev);
    phwdev->parenth = parent ? pciehwdev_geth(pciehdev_get_hwdev(parent)) : 0;

    /*
     * Don't load the root into hardware.  root represents the
     * upstream port bridge which is provided by hardware so no
     * need to add to hw tables to virtualize.
     */
    if (parent) {
        /* bar first, then cfg below */
        pciehw_bars_finalize(pdev);

        /*
         * We'll load hdrt when the link comes up.
         * pciehw_hdrt_load(phwdev->lifb, phwdev->lifc, phwdev->bdf);
         */
        pciehw_portmap_load(phwdev->lifb, phwdev->lifc, phwdev->port);
    }

    /* cfg after bar above */
    pciehw_cfg_finalize(pdev);

    /* init_intr() now that we have a parent link and cfg */
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
        /* VF has vendor/devicid 0xffffffff */
        if (val == 0xffffffff) {
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
    pciemgr_params_t *params = &phw->params;
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehwdev_t *phwroot = proot ? pciehw_finalize_dev(proot) : NULL;
    const u_int8_t port = proot ? pciehdev_get_port(proot) : 0;

    pciehw_cfg_finalize_done(phwroot);
    pshmem->rooth[port] = pciehwdev_geth(phwroot);
    if (params->fake_bios_scan) {
        int nextbus = 1;
        fake_bios_scan(port, 0, &nextbus);
    }
}

/******************************************************************
 * debug
 */

#define DEVF_ALL        0x01

static void
dev_show_detailed(const pciehwdev_t *phwdev, const int flags)
{
    const int w = 20;
    char lifstr[16] = { '\0' };
    char intrsstr[16] = { '\0' };

    if (phwdev->lifc == 1) {
        snprintf(lifstr, sizeof(lifstr), "%d", phwdev->lifb);
    } else if (phwdev->lifc > 1) {
        snprintf(lifstr, sizeof(lifstr), "%d-%d",
                 phwdev->lifb,
                 phwdev->lifb + phwdev->lifc - 1);
    }
    if (phwdev->intrc) {
        snprintf(intrsstr, sizeof(intrsstr), "%d-%d",
                 phwdev->intrb,
                 phwdev->intrb + phwdev->intrc - 1);
    }

    pciesys_loginfo("%-*s %s\n", w, "name", pciehwdev_get_name(phwdev));
    pciesys_loginfo("%-*s %d\n", w, "hdl", pciehwdev_geth(phwdev));
    pciesys_loginfo("%-*s %d\n", w, "port", phwdev->port);
    pciesys_loginfo("%-*s %s\n", w, "bdf", bdf_to_str(phwdev->bdf));
    pciesys_loginfo("%-*s %s\n", w, "lif", lifstr);
    pciesys_loginfo("%-*s %s\n", w, "intrs", intrsstr);
    pciesys_loginfo("%-*s %d\n", w, "intrdmask", phwdev->intrdmask);
    pciesys_loginfo("%-*s %c\n", w, "intpin",
                    !phwdev->vf && phwdev->intrc ?
                    "ABCD"[phwdev->intpin] : ' ');
    pciesys_loginfo("%-*s %d\n", w, "pf", phwdev->pf);
    pciesys_loginfo("%-*s %d\n", w, "vf", phwdev->vf);
    pciesys_loginfo("%-*s %d\n", w, "vfidx", phwdev->vfidx);
    pciesys_loginfo("%-*s %d\n", w, "totalvfs", phwdev->totalvfs);
    pciesys_loginfo("%-*s %d\n", w, "numvfs", phwdev->numvfs);
    pciesys_loginfo("%-*s %d\n", w, "enabledvfs", phwdev->enabledvfs);
    pciesys_loginfo("%-*s 0x%04x\n", w, "sriovctrl", phwdev->sriovctrl);
    pciesys_loginfo("%-*s %d\n", w, "parenth", phwdev->parenth);
    pciesys_loginfo("%-*s %d\n", w, "childh", phwdev->childh);
    pciesys_loginfo("%-*s %d\n", w, "peerh", phwdev->peerh);
}

static void
dev_show1(const pciehwdev_t *phwdev, const int flags)
{
    char lifstr[16] = { '\0' };
    char intrsstr[16] = { '\0' };

    if (phwdev->lifc == 1) {
        snprintf(lifstr, sizeof(lifstr), "%d", phwdev->lifb);
    } else if (phwdev->lifc > 1) {
        snprintf(lifstr, sizeof(lifstr), "%d-%d",
                 phwdev->lifb,
                 phwdev->lifb + phwdev->lifc - 1);
    }
    if (phwdev->intrc) {
        snprintf(intrsstr, sizeof(intrsstr), "%d-%d",
                 phwdev->intrb,
                 phwdev->intrb + phwdev->intrc - 1);
    }
    pciesys_loginfo("%-3d %-9s %-16s %1d:%-7s %c    %-5s\n",
                    pciehwdev_geth(phwdev),
                    lifstr,
                    pciehwdev_get_name(phwdev),
                    phwdev->port,
                    bdf_to_str(pciehwdev_get_hostbdf(phwdev)),
                    !phwdev->vf && phwdev->intrc ?
                        "ABCD"[phwdev->intpin] : ' ',
                    intrsstr);
}

static void
dev_show_all(const int flags)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehwdev_t *phwdev;
    int i;

    phwdev = &pshmem->dev[1];
    for (i = 1; i <= pshmem->allocdev; i++, phwdev++) {
        cfgspace_t cs;

        pciehwdev_get_cfgspace(phwdev, &cs);
        if (flags & DEVF_ALL || cfgspace_get_headertype(&cs) == 0) {
            dev_show1(phwdev, flags);
        }
    }
}

void
pciehw_dev_show(int argc, char *argv[])
{
    int opt, flags = 0;
    char *name = NULL;

    optind = 0;
    while ((opt = getopt(argc, argv, "ad:")) != -1) {
        switch (opt) {
        case 'a': flags |= DEVF_ALL; break;
        case 'd': name = optarg; break;
        case '?': return;
        }
    }

    if (name) {
        pciehwdev_t *phwdev = pciehwdev_find_by_name(name);
        if (phwdev != NULL) {
            dev_show_detailed(phwdev, flags);
        } else {
            pciesys_loginfo("%s: device not found\n", name);
        }
    } else {
        pciesys_loginfo("%-3s %-9s %-16s %-9s %-4s %-5s\n",
                        "hdl", "lif", "name", "p:bb:dd.f", "intx", "intrs");
        dev_show_all(flags);
    }
}

static void
devintr_show1(const pciehwdev_t *phwdev, const int flags)
{
    int devidx, hwintr;
    intr_state_t intrst;
    char intx[8];

    hwintr = phwdev->intrb;
    for (devidx = 0; devidx < phwdev->intrc; devidx++, hwintr++) {

        intr_state_get(hwintr, &intrst);
        snprintf(intx, sizeof(intx), "int%c", "ABCD"[intrst.fwcfg_legacy_pin]);
        pciesys_loginfo("%-16s %-6d %-6d %c%c%c%c %-5s "
                        "0x%08" PRIx64 " 0x%-5x %d\n",
                        pciehwdev_get_name(phwdev),
                        devidx,
                        hwintr,
                        intrst.fwcfg_function_mask   ? 'f' : '-',
                        intrst.msixcfg_vector_ctrl   ? 'v' : '-',
                        intrst.drvcfg_mask           ? 'd' : '-',
                        intrst.drvcfg_mask_on_assert ? 'a' : '-',
                        intrst.fwcfg_local_int       ? "msixl" :
                        intrst.fwcfg_legacy_int      ? intx : "msix",
                        (uint64_t)intrst.msixcfg_msg_addr_51_2 << 2,
                        intrst.msixcfg_msg_data,
                        intrst.drvcfg_int_credits);
    }
}

static void
devintr_show_all(const int flags)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehwdev_t *phwdev;
    int i;

    phwdev = &pshmem->dev[1];
    for (i = 1; i <= pshmem->allocdev; i++, phwdev++) {
        devintr_show1(phwdev, flags);
    }
}

void
pciehw_devintr_show(int argc, char *argv[])
{
    pciehwdev_t *phwdev = NULL;
    int opt, flags = 0;

    optind = 0;
    while ((opt = getopt(argc, argv, "d:")) != -1) {
        switch (opt) {
        case 'd':
            phwdev = pciehwdev_find_by_name(optarg);
            if (phwdev == NULL) {
                pciesys_logerror("%s: device not found\n", optarg);
                return;
            }
            break;
        case '?':
            return;
        }
    }

    pciesys_loginfo("%-16s %-6s %-6s %-4s %-5s "
                    "%-10s %-7s %s\n",
                    "name", "devidx", "hwintr", "mask", "mode",
                    "msgaddr", "msgdata", "int_credits");
    if (phwdev != NULL) {
        devintr_show1(phwdev, flags);
    } else {
        devintr_show_all(flags);
    }
}
static void
cmd_dev(int argc, char *argv[])
{
    pciehw_dev_show(argc, argv);
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

pciemgr_stats_t *
pciehw_stats_get(const int port)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehw_port_t *p;

    if (port < 0 || port >= PCIEHW_NPORTS) {
        return NULL;
    }
    p = &pshmem->port[port];
    return &p->stats;
}

void
pciehw_stats_show(const int port, const unsigned int flags)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehw_port_t *p;
    const int w = 20;

    if (port < 0 || port >= PCIEHW_NPORTS) {
        pciesys_logerror("port %d out of range\n", port);
        return;
    }
    p = &pshmem->port[port];

#define PCIEMGR_STATS_DEF(S) \
    if (flags & PMGRSF_ALL || p->stats.S) \
        pciesys_loginfo("%-*s %" PRIi64 "\n", w, #S, p->stats.S);

#include "../include/pciemgr_stats_defs.h"
}

void
pciehw_stats_clear(const int port, const unsigned int flags)
{
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    pciehw_port_t *p;

    if (port < 0 || port >= PCIEHW_NPORTS) {
        pciesys_logerror("port %d out of range\n", port);
        return;
    }
    p = &pshmem->port[port];

#define PCIEMGR_STATS_DEF(S) \
    if (p->stats.S) p->stats.S = 0;

#include "../include/pciemgr_stats_defs.h"
}

static void
cmd_stats(int argc, char *argv[])
{
    int opt, port;
    unsigned int flags;

    port = 0;
    flags = PMGRSF_NONE;
    optind = 0;
    while ((opt = getopt(argc, argv, "ap:")) != -1) {
        switch (opt) {
        case 'a': flags |= PMGRSF_ALL; break;
        case 'p': port = strtoul(optarg, NULL, 0); break;
        case '?': return;
        }
    }
    pciehw_stats_show(port, flags);
}

static void
cmd_meminfo(int argc, char *argv[])
{
    pciehw_mem_t *phwmem = pciehw_get_hwmem();
    pciehw_shmem_t *pshmem = pciehw_get_shmem();
    u_int64_t hwmempa = pal_mem_vtop(phwmem);
    const int w = 24;

#define PSZ(t) \
    pciesys_loginfo("%-*s: %lu\n", w, "sizeof "#t, sizeof(t));

    /* pciehw_mem_t info */
    PSZ(pciehw_mem_t);

    pciesys_loginfo("%-*s: 0x%08" PRIx64 "\n", w, "physaddr", hwmempa);
    pciesys_loginfo("%-*s: 0x%08"PRIx64" size %lu\n", w, "notify_area",
                    hwmempa + offsetof(pciehw_mem_t, notify_area),
                    sizeof(phwmem->notify_area));
    pciesys_loginfo("%-*s: 0x%08"PRIx64" size %lu\n", w, "cfgcur",
                    hwmempa + offsetof(pciehw_mem_t, cfgcur),
                    sizeof(phwmem->cfgcur));
    pciesys_loginfo("%-*s: 0x%08x\n", w, "magic", phwmem->magic);
    pciesys_loginfo("%-*s: %d\n", w, "version", phwmem->version);

    /* pciehw_shmem_t info */
    PSZ(pciehw_shmem_t);
    PSZ(pciehwdev_t);
    PSZ(pciehwbar_t);
    PSZ(pciehw_port_t);
    PSZ(pciehw_spmt_t);
    PSZ(pciehw_sprt_t);
    PSZ(pciehw_sromsk_t);
    PSZ(pciemgr_stats_t);

    pciesys_loginfo("%-*s: %p\n", w, "pshmem->dev", pshmem->dev);
    pciesys_loginfo("%-*s: %d\n", w, "allocdev", pshmem->allocdev);
    pciesys_loginfo("%-*s: %d\n", w, "allocprt", pshmem->allocprt);
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
    CMDENT(stats, "stats", ""),
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
        pciesys_loginfo("Usage: pciehw <subcmd>\n");
        return;
    }

    c = cmd_lookup(cmdtab, argv[1]);
    if (c == NULL) {
        pciesys_logerror("%s: %s not found\n", argv[0], argv[1]);
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

unsigned long long
pciehw_barsz(const u_int8_t port, const u_int16_t bdf, const int i)
{
    pciehwdev_t *phwdev;
    pciehwbar_t *phwbar;

    phwdev = pciehwdev_by_bdf(port, bdf);
    if (phwdev == NULL) {
        return 0;
    }
    phwbar = &phwdev->bar[i];
    return pciehw_bar_getsize(phwbar);
}

/*
 * Called by lspci, this function might be called to read any
 * arbitrary byte(s) of vpd.  The underlying worker function expects
 * dword-aligned access only since that is what the pcie spec allows.
 * We have to account for unaligned head and tail here.
 */
int
pciehw_read_vpd(const u_int8_t port, const u_int16_t bdf,
                const u_int16_t addr, void *buf, const size_t len)
{
    const u_int16_t aligned_addr = addr & ~0x3;
    const size_t aligned_len = roundup(addr + len, 4) - aligned_addr;
    u_int8_t vpddata[PCIEHW_VPDSZ];
    pciehwdev_t *phwdev;
    u_int32_t *vp;
    int i;

    phwdev = pciehwdev_by_bdf(port, bdf);
    if (phwdev == NULL) {
        return 0;
    }
    if (addr + len >= PCIEHW_VPDSZ) {
        return 0;
    }

    vp = (u_int32_t *)&vpddata[aligned_addr];
    for (i = aligned_addr; i < aligned_addr + aligned_len; i += 4, vp++) {
        *vp = pciehw_vpd_read(phwdev, i);
    }
    memcpy(buf, &vpddata[addr], len);
    return 1;
}

void *
pciehw_memset(void *s, int c, size_t n)
{
#ifdef DEVICEMEM_WAR
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
#else
    return memset(s, c, n);
#endif
}

void *
pciehw_memcpy(void *dst, const void *src, size_t n)
{
#ifdef DEVICEMEM_WAR
    volatile u_int8_t *d = dst;
    const u_int8_t *s = src;
    int i;

    for (i = 0; i < n; i++) {
        *d++ = *s++;
    }
    return dst;
#else
    return memcpy(dst, src, n);
#endif
}
