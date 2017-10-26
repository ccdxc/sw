/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "pciehost.h"
#include "cfgspace.h"
#include "pciehw.h"
#include "pciehw_impl.h"
#include "pciehsys.h"
#include "bdf.h"

static pciehw_t pciehw;

static pciehw_t *
pciehw_get(void)
{
    return &pciehw;
}

static pciehw_mem_t *
pciehw_get_hwmem(pciehw_t *phw)
{
    return phw->pciehwmem;
}

static pciehwdev_t *
pciehwdev_get(pciehwdevh_t hwdevh)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    return hwdevh ? &phwmem->dev[hwdevh] : NULL;
}

static pciehwdevh_t
pciehwdev_geth(pciehwdev_t *phwdev)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    return phwdev ? phwdev - phwmem->dev : 0;
}

static void
pciehwdev_get_cfgspace(pciehwdev_t *phwdev, cfgspace_t *cs)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);

    cs->cur = phwmem->cfgcur[hwdevh];
    cs->msk = phwmem->cfgmsk[hwdevh];
    cs->size = PCIEHW_CFGSZ;
}

static int
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

static int
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
pciehw_initmem(pciehw_mem_t *phwmem)
{
    memset(phwmem, 0, sizeof(*phwmem));
    phwmem->version = PCIEHW_VERSION;
    phwmem->magic = PCIEHW_MAGIC;
    return 0;
}

int
pciehw_open(pciehw_params_t *hwparams)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem;
    int inithw, r;

    /* already open */
    if (phw->flags & PCIEHWF_OPEN) {
        phw->clients++;
        return 0;
    }
    if (hwparams) {
        phw->hwparams = *hwparams;
    }
    inithw = phw->hwparams.inithw;
    if ((r = pciehw_openmem(phw, inithw)) < 0) {
        return r;
    }

    /* Is mem initialized? */
    phwmem = pciehw_get_hwmem(phw);
    if (phwmem->magic != PCIEHW_MAGIC) {
        if (!inithw) {
            pciehw_closemem(phw);
            return -EINVAL;
        }
        pciehw_initmem(phwmem);
    }

    /* Do we understand this version? */
    if (phwmem->version != PCIEHW_VERSION) {
        pciehw_closemem(phw);
        return -ENOEXEC;
    }

    phw->flags |= PCIEHWF_OPEN;
    phw->clients++;
    return 0;
}

void
pciehw_close(void)
{
    pciehw_t *phw = pciehw_get();

    if (phw->flags & PCIEHWF_OPEN) {
        if (--phw->clients) {
            pciehw_closemem(phw);
            phw->flags &= ~PCIEHWF_OPEN;
        }
    }
}

static pciehwdev_t *
pciehwdev_alloc(pciehdev_t *pdev)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdev_t *phwdev;

    assert(phwmem->allocdev < PCIEHW_NDEVS - 1);
    phwdev = &phwmem->dev[++phwmem->allocdev];
    memset(phwdev, 0, sizeof(*phwdev));
    phwdev->pdev = pdev;
    pciehdev_set_hwdev(pdev, phwdev);
    return phwdev;
}

void
pciehw_initialize_topology(void)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);

    if (phwmem) {
        phwmem->allocdev = 0;
        phwmem->rooth = 0;
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
pciehwdev_by_bdf(const u_int16_t bdf)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdev_t *phwdev, *phwdev_found;
    pciehwdevh_t phwdevh;

    if (phwmem == NULL) {
        return NULL;
    }
    phwdev_found = NULL;
    for (phwdevh = phwmem->rooth; phwdevh; phwdevh = phwdev->peerh) {
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
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdev_t *phwdev = pciehwdev_alloc(pdev);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    pciehcfg_t *pcfg = pciehdev_get_cfg(pdev);
    pciehdev_t *parent, *peer, *child;
    cfgspace_t cs;
    u_int16_t cfgsz;

    /*
     * Init config space contents from device config space.
     * Init config reset contents from device config space.
     * Init config write mask from device config space.
     */
    pciehcfg_get_cfgspace(pcfg, &cs);
    cfgsz = cfgspace_size(&cs);
    assert(cfgsz <= PCIEHW_CFGSZ);
    memcpy(phwmem->cfgcur[hwdevh], cs.cur, cfgsz); /* config space */
    memcpy(phwmem->cfgrst[hwdevh], cs.cur, cfgsz); /* reset space */
    memcpy(phwmem->cfgmsk[hwdevh], cs.msk, cfgsz); /* write mask */

    phwdev->bdf = pciehdev_get_bdf(pdev);
    parent = pciehdev_get_parent(pdev);
    phwdev->parenth = parent ? pciehwdev_geth(pciehdev_get_hwdev(parent)) : 0;

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
fake_bios_scan(int bus, int *nextbus)
{
    int dev, bdf;
    u_int32_t val;

    for (dev = 0; dev < 32; dev++) {
        bdf = bdf_make(bus, dev, 0);

        /* read vendor/device id to be sure a device exists at bdf */
        if (pciehw_cfgrd(bdf, 0, 4, &val) < 0) {
            continue;
        }
        /* read header type register */
        if (pciehw_cfgrd(bdf, 0xe, 1, &val) < 0) {
            continue;
        }
        /* bridge header type? */
        if ((val & 0x7f) == 1) {
            /* set secondary bus number */
            int secbus = (*nextbus)++;
            pciehw_cfgwr(bdf, 0x19, 1, secbus);
            fake_bios_scan(secbus, nextbus); /* scan secondary bus */
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
    phwmem->rooth = pciehwdev_geth(phwroot);

    if (phwparams->fake_bios_scan) {
        int nextbus = 1;
        fake_bios_scan(0, &nextbus);
    }
}

int
pciehw_cfgrd(const u_int16_t bdf,
             const u_int16_t offset, const u_int8_t size, u_int32_t *valp)
{
    pciehwdev_t *phwdev;

    phwdev = pciehwdev_by_bdf(bdf);
    if (phwdev == NULL) {
        return -ESRCH;
    }
    return pciehwdev_cfgrd(phwdev, offset, size, valp);
}

int
pciehw_cfgwr(const u_int16_t bdf,
             const u_int16_t offset, const u_int8_t size, const u_int32_t val)
{
    pciehwdev_t *phwdev;

    phwdev = pciehwdev_by_bdf(bdf);
    if (phwdev == NULL) {
        return -ESRCH;
    }
    return pciehwdev_cfgwr(phwdev, offset, size, val);
}

static u_int64_t memval;
static u_int32_t ioval;

int
pciehw_memrd(const u_int64_t addr, const u_int8_t size, u_int64_t *valp)
{
    *valp = memval;
    return 0;
}

int
pciehw_memwr(const u_int64_t addr, const u_int8_t size, u_int64_t val)
{
    memval = val;
    return 0;
}

int
pciehw_iord(const u_int32_t addr, const u_int8_t size, u_int32_t *valp)
{
    *valp = ioval;
    return 0;
}

int
pciehw_iowr(const u_int32_t addr, const u_int8_t size, u_int32_t val)
{
    ioval = val;
    return 0;
}

pciehw_params_t *
pciehw_get_params(void)
{
    pciehw_t *phw = pciehw_get();
    return &phw->hwparams;
}
