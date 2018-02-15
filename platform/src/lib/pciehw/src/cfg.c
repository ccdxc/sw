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
#include <linux/pci_regs.h>

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
    const u_int16_t regdw = reg >> 2;
    assert(regdw < PCIEHW_CFGHNDSZ);
    phwdev->cfghnd[regdw] = hnd;
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

    msixcap = cfgspace_findcap(&cs, PCI_CAP_ID_MSIX);
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
 * regaddr        regsize
 * v              v
 * +--------------+
 * +--------------+
 * ^              ^
 * tlpaddr        tpsize
 *
 * regaddr        regsize
 * v              v
 * +--------------+
 *           +--------------+
 *           ^              ^
 *           tlpaddr        tpsize
 *
 *     regaddr        regsize
 *     v              v
 *     +--------------+
 * +--------------+
 * ^              ^
 * tlpaddr        tpsize
 */
static int
stlp_overlap(const pcie_stlp_t *stlp,
             const u_int32_t regaddr, const u_int32_t regsize)
{
    const u_int32_t tlpaddr = stlp->addr;
    const u_int32_t tlpsize = stlp->size;

    return tlpaddr < regaddr + regsize && tlpaddr + tlpsize > regaddr;
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
    u_int32_t baraddr = cfgspace_readd(cs, cfgoff);
    return baraddr;
}

static u_int64_t
cfg_bar64(cfgspace_t *cs, const u_int32_t cfgoff)
{
    u_int32_t barlo, barhi;

    barlo = cfgspace_readd(cs, cfgoff + 0);
    barhi = cfgspace_readd(cs, cfgoff + 4);

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
    u_int16_t cmd, msixcap, msixctl;

    pciehwdev_get_cfgspace(phwdev, &cs);
    cmd = cfgspace_readw(&cs, PCI_COMMAND);
    msixcap = cfgspace_findcap(&cs, PCI_CAP_ID_MSIX);
    if (msixcap) {
        msixctl = cfgspace_readw(&cs, msixcap + PCI_MSIX_FLAGS);
    } else {
        msixctl = 0;
    }

    /* bar control */
    pciehw_cfg_io_enable(phwdev, (cmd & 0x1) != 0);
    pciehw_cfg_mem_enable(phwdev, (cmd & 0x2) != 0);

    /* intx_disable */
    if ((msixctl & PCI_MSIX_FLAGS_ENABLE) == 0) {
        const int legacy = 1;
        const int fmask = (cmd & PCI_COMMAND_INTX_DISABLE) != 0;
        pciehw_intr_config(phwdev, legacy, fmask);
    }

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

static void
pciehw_cfgwr_msix(pciehwdev_t *phwdev, const pcie_stlp_t *stlp)
{
    const u_int16_t reg = stlp->addr;
    const u_int16_t regdw = reg >> 2;
    cfgspace_t cs;
    u_int16_t msixctl, cmd;
    int msix_en, msix_mask, fmask, legacy;

    pciehwdev_get_cfgspace(phwdev, &cs);
    msixctl = cfgspace_readw(&cs, (regdw << 2) + 2);
    msix_en = (msixctl & PCI_MSIX_FLAGS_ENABLE) != 0;
    msix_mask = (msixctl & PCI_MSIX_FLAGS_MASKALL) != 0;

    if (msix_en) {
        /* msix mode */
        legacy = 0;
        fmask = msix_mask;
    } else {
        /* intx mode */
        legacy = 1;
        cmd = cfgspace_readw(&cs, PCI_COMMAND);
        fmask = (cmd & PCI_COMMAND_INTX_DISABLE) != 0;
    }

    pciehw_intr_config(phwdev, legacy, fmask);
}

void
pciehw_cfgwr_notify(pciehwdev_t *phwdev,
                    const pcie_stlp_t *stlp,
                    const pciehw_spmt_t *spmt)
{
    const u_int16_t reg = stlp->addr;
    const u_int16_t regdw = reg >> 2;
    pciehw_cfghnd_t hnd = PCIEHW_CFGHND_NONE;

    if (regdw < PCIEHW_CFGHNDSZ) {
        hnd = phwdev->cfghnd[regdw];
    }
    switch (hnd) {
    case PCIEHW_CFGHND_NONE:
        break;
    case PCIEHW_CFGHND_CMD:
        pciehw_cfgwr_cmd(phwdev, stlp);
        break;
    case PCIEHW_CFGHND_BARS:
        pciehw_cfgwr_bars(phwdev, stlp);
        break;
    case PCIEHW_CFGHND_MSIX:
        pciehw_cfgwr_msix(phwdev, stlp);
        break;
    }
}

void
pciehw_cfgrd_indirect(indirect_entry_t *ientry, const pcie_stlp_t *stlp)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    const u_int32_t pmti = ientry->info.pmti;
    const pciehw_spmt_t *spmt = &phwmem->spmt[pmti];
    const pciehwdevh_t hwdevh = spmt->owner;
    pciehwdev_t *phwdev = pciehwdev_get(hwdevh);
    u_int32_t val;

    if (pciehwdev_cfgrd(phwdev, stlp->addr, stlp->size, &val) < 0) {
        ientry->cpl = PCIECPL_CA;
    } else {
        ientry->data[0] = val;
    }
    pciehw_indirect_complete(ientry);

    pciehw_cfgrd_notify(phwdev, stlp, spmt);
}

void
pciehw_cfgwr_indirect(indirect_entry_t *ientry, const pcie_stlp_t *stlp)
{
    pciehw_t *phw = pciehw_get();
    pciehw_mem_t *phwmem = pciehw_get_hwmem(phw);
    const u_int32_t pmti = ientry->info.pmti;
    const pciehw_spmt_t *spmt = &phwmem->spmt[pmti];
    const pciehwdevh_t hwdevh = spmt->owner;
    pciehwdev_t *phwdev = pciehwdev_get(hwdevh);

    if (pciehwdev_cfgwr(phwdev, stlp->addr, stlp->size, stlp->data) < 0) {
        ientry->cpl = PCIECPL_CA;
    }
    pciehw_indirect_complete(ientry);

    pciehw_cfgwr_notify(phwdev, stlp, spmt);
}
