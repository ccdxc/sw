/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "platform/include/common/pci_ids.h"
#include "nic/sdk/platform/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"
#include "pciehdevices_impl.h"

#define PCI_DEVICE_ID_PENSANDO_PCIESTRESS 0x8001

static void
init_bars(pciehbars_t *pbars, const pciehdevice_resources_t *pres)
{
    pciehbarreg_t preg;
    pciehbar_t pbar;
    u_int64_t sz = 0x1000;
    u_int64_t pa = 0x13f000000 + (sz * pres->port);
    prt_t prt;

    /* mem64 bar */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    pbar.size = sz * 4; /* 4 pages with different properties */
    pbar.cfgidx = 0;

    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                pres->port,
                PMT_TYPE_MEM,
                pbar.size,
                sz,     /* prtsize */
                PMTF_RW);
    pmt_bar_setr_prt(&preg.pmt, 12, 2);

    /* direct access */
    prt_res_enc(&prt, pa, 0x1000, PRT_RESF_PMVDIS);
    pciehbarreg_add_prt(&preg, &prt);
    /* notify access */
    prt_res_enc(&prt, pa, 0x1000, PRT_RESF_NOTIFY);
    pciehbarreg_add_prt(&preg, &prt);
    /* indirect access */
    prt_res_enc(&prt, pa, 0x1000, PRT_RESF_INDIRECT);
    pciehbarreg_add_prt(&preg, &prt);
    /* pad prt */
    prt_res_enc(&prt, 0, 0, PRT_RESF_NONE);
    pciehbarreg_add_prt(&preg, &prt);

    pciehbar_add_reg(&pbar, &preg);
    pciehbars_add_bar(pbars, &pbar);

    /* io bar */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_IO;
    pbar.size = 0x80;
    pbar.cfgidx = 2;

    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                pres->port,
                PMT_TYPE_IO,
                pbar.size,
                0x20, /* prtsize */
                PMTF_RW);
    pmt_bar_setr_prt(&preg.pmt, 5, 2);

    /* direct access */
    prt_res_enc(&prt, pa, 0x20, PRT_RESF_PMVDIS);
    pciehbarreg_add_prt(&preg, &prt);
    /* notify access */
    prt_res_enc(&prt, pa, 0x20, PRT_RESF_NOTIFY);
    pciehbarreg_add_prt(&preg, &prt);
    /* indirect access */
    prt_res_enc(&prt, pa, 0x20, PRT_RESF_INDIRECT);
    pciehbarreg_add_prt(&preg, &prt);
    /* pad prt */
    prt_res_enc(&prt, 0, 0, PRT_RESF_NONE);
    pciehbarreg_add_prt(&preg, &prt);

    pciehbar_add_reg(&pbar, &preg);
    pciehbars_add_bar(pbars, &pbar);

    /* just in case we want to test rom bar on this device */
    add_common_rom_bar(pbars, pres);
}

static void
init_cfg(pciehcfg_t *pcfg, pciehbars_t *pbars,
         const pciehdevice_resources_t *pres)
{
    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_PCIESTRESS);
    pciehcfg_setconf_classcode(pcfg, 0xff0000); /* unclassified device */
    pciehcfg_setconf_dsn(pcfg, pres->dsn);

    pciehcfg_sethdr_type0(pcfg, pbars);
    pciehcfg_add_standard_caps(pcfg);
}

static void
initialize_bars(pciehdev_t *pdev, const pciehdevice_resources_t *pres)
{
    pciehbars_t *pbars;

    pbars = pciehbars_new();
    init_bars(pbars, pres);
    pciehdev_set_bars(pdev, pbars);
}

static void
initialize_cfg(pciehdev_t *pdev, const pciehdevice_resources_t *pres)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    init_cfg(pcfg, pbars, pres);
    pciehdev_set_cfg(pdev, pcfg);
}

pciehdev_t *
pciehdev_pciestress_new(const char *name, const pciehdevice_resources_t *pres)
{
    pciehdev_t *pdev = pciehdev_new(name, pres);
    initialize_bars(pdev, pres);
    initialize_cfg(pdev, pres);
    return pdev;
}
