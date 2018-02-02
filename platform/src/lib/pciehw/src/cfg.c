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

#include "bdf.h"
#include "pciehsys.h"
#include "pal.h"
#include "pcietlp.h"
#include "pciehost.h"
#include "cfgspace.h"
#include "pciehw.h"
#include "pciehw_impl.h"

int
pciehw_cfg_init(pciehw_t *phw)
{
    pciehw_romsk_init(phw);
    return 0;
}

static void
pciehw_set_cfghnd(pciehwdev_t *phwdev,
                  const u_int16_t reg, const pciehw_cfghnd_t hnd)
{
    phwdev->cfghnd[reg >> 2] = hnd;
}

static void
pciehw_cfg_set_handlers(pciehwdev_t *phwdev)
{
    cfgspace_t cs;
    pciehwbar_t *phwbar;
    int i, msixcap, cfgoff;

    pciehwdev_get_cfgspace(phwdev, &cs);

    /* Command/Status register */
    pciehw_set_cfghnd(phwdev, 0x4, PCIEHW_CFGHND_CMD);

    cfgoff = 0x10;
    for (phwbar = phwdev->bar, i = 0; i < PCIEHW_NBAR; i++, phwbar++) {
        if (phwbar->valid) {
            const int barlen = phwbar->type == PCIEHWBARTYPE_MEM64 ? 8 : 4;
            pciehw_set_cfghnd(phwdev, cfgoff, PCIEHW_CFGHND_BARS);
            if (barlen == 8) {
                pciehw_set_cfghnd(phwdev, cfgoff + 4, PCIEHW_CFGHND_BARS);
            }
            cfgoff += barlen;
        }
    }

    msixcap = cfgspace_findcap(&cs, 0x11); /* MSIX */
    if (msixcap) {
        /* MSIX message control */
        pciehw_set_cfghnd(phwdev, msixcap, PCIEHW_CFGHND_MSIX);
    }
}

int
pciehw_cfg_finalize(pciehdev_t *pdev)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    pciehwdev_t *phwdev = pciehdev_get_hwdev(pdev);
    pciehwdevh_t hwdevh = pciehwdev_geth(phwdev);
    pciehcfg_t *pcfg = pciehdev_get_cfg(pdev);
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
    pciehw_memcpy(phwmem->cfgcur[hwdevh], cs.cur, cfgsz); /* config space */
    pciehw_memcpy(phwmem->cfgrst[hwdevh], cs.cur, cfgsz); /* reset space */
    pciehw_memcpy(phwmem->cfgmsk[hwdevh], cs.msk, cfgsz); /* write mask */

    pciehw_cfg_set_handlers(phwdev);

    /*
     * Don't load the root into hardware.  root represents the
     * upstream port bridge which is provided by hardware so no
     * need to add to hw tables to virtualize.
     */
    if (pciehdev_get_parent(pdev) == NULL) {
        return 0;
    }

    if (pciehw_romsk_load(phw, phwdev) < 0) {
        pciehsys_error("%s: romsk_load failed\n", pciehdev_get_name(pdev));
        return -1;
    }
    if (pciehw_vfstride_load(phw, phwdev) < 0) {
        pciehsys_error("%s: vfstride_load failed\n", pciehdev_get_name(pdev));
        return -1;
    }
    if (pciehw_pmt_load_cfg(phwdev) < 0) {
        pciehsys_error("%s: pmt_load_cfg failed\n", pciehdev_get_name(pdev));
        return -1;
    }
    return 0;
}

/*
 * Detect these overlaps:
 *
 * start          count
 * v              v
 * +--------------+
 * +--------------+
 * ^              ^
 * tlpaddr        tpsize
 *
 * start          count
 * v              v
 * +--------------+
 *           +--------------+
 *           ^              ^
 *           tlpaddr        tpsize
 *
 *     start          count
 *     v              v
 *     +--------------+
 * +--------------+
 * ^              ^
 * tlpaddr        tpsize
 */
static int
stlp_overlap(const pcie_stlp_t *stlp,
             const u_int32_t start, const u_int32_t count)
{
    const u_int32_t tlpaddr = stlp->addr;
    const u_int32_t tlpsize = stlp->size;

    return tlpaddr < start + count && tlpaddr + tlpsize > start;
}

void
pciehw_cfgrd_notify(pciehwdev_t *phwdev,
                    const pcie_stlp_t *stlp,
                    const pciehw_spmt_t *spmt)
{
}

static u_int32_t
cfg_bar32(cfgspace_t *cs, const u_int32_t cfgoff)
{
    u_int32_t baraddr;

    cfgspace_read(cs, cfgoff, 4, &baraddr);
    return baraddr;
}

static u_int64_t
cfg_bar64(cfgspace_t *cs, const u_int32_t cfgoff)
{
    u_int32_t barlo, barhi;

    cfgspace_read(cs, cfgoff + 0, 4, &barlo);
    cfgspace_read(cs, cfgoff + 4, 4, &barhi);

    return ((u_int64_t)barhi << 32) | barlo;
}

static u_int64_t
cfg_baraddr(cfgspace_t *cs, const u_int32_t cfgoff, const u_int32_t barlen)
{
    u_int64_t baraddr;

    if (barlen == 8) {
        baraddr = cfg_bar64(cs, cfgoff);
    } else {
        baraddr = cfg_bar32(cs, cfgoff);
    }
    return baraddr;
}

static void
pciehw_cfg_mem_enable(pciehwdev_t *phwdev, const int on)
{
    pciehwbar_t *phwbar;
    int i;

    for (phwbar = phwdev->bar, i = 0; i < PCIEHW_NBAR; i++, phwbar++) {
        if (phwbar->valid &&
            (phwbar->type == PCIEHWBARTYPE_MEM ||
             phwbar->type == PCIEHWBARTYPE_MEM64)) {
            pciehw_bar_enable(phwbar, on);
        }
    }
}

static void
pciehw_cfg_io_enable(pciehwdev_t *phwdev, const int on)
{
    pciehwbar_t *phwbar;
    int i;

    for (phwbar = phwdev->bar, i = 0; i < PCIEHW_NBAR; i++, phwbar++) {
        if (phwbar->valid && phwbar->type == PCIEHWBARTYPE_IO) {
            pciehw_bar_enable(phwbar, on);
        }
    }
}

static void
pciehw_cfgwr_cmd(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    cfgspace_t cs;
    u_int32_t cmd;

    pciehwdev_get_cfgspace(phwdev, &cs);
    cfgspace_read(&cs, 0x4, 2, &cmd);
    pciehw_cfg_io_enable(phwdev, (cmd & 0x1) != 0);
    pciehw_cfg_mem_enable(phwdev, (cmd & 0x2) != 0);
#if 0
    /* XXX */
    pciehw_cfg_busmaster_enable(phwdev, (cmd & 0x4) != 0);
#endif
}

static void
pciehw_cfgwr_bars(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    pciehwbar_t *phwbar;
    cfgspace_t cs;
    int i, cfgoff;

    pciehwdev_get_cfgspace(phwdev, &cs);

    cfgoff = 0x10;
    for (phwbar = phwdev->bar, i = 0; i < PCIEHW_NBAR; i++, phwbar++) {
        if (phwbar->valid) {
            const int barlen = phwbar->type == PCIEHWBARTYPE_MEM64 ? 8 : 4;
            if (stlp_overlap(stlp, cfgoff, barlen)) {
                u_int64_t baraddr = cfg_baraddr(&cs, cfgoff, barlen);
                if (phwbar->type == PCIEHWBARTYPE_IO) {
                    baraddr &= ~0x3ULL;
                } else {
                    baraddr &= ~0xfULL;
                }
                pciehw_bar_setaddr(phwbar, baraddr);
            }
            cfgoff += barlen;
        }
    }
}

void
pciehw_cfgwr_notify(pciehwdev_t *phwdev,
                    const pcie_stlp_t *stlp,
                    const pciehw_spmt_t *spmt)
{
    if (stlp_overlap(stlp, 0x4, 0x2)) {
        pciehw_cfgwr_cmd(phwdev, stlp);
    }
    if (stlp_overlap(stlp, 0x10, 0x18)) {
        pciehw_cfgwr_bars(phwdev, stlp);
    }
}
