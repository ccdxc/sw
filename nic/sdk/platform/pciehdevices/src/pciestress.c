/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"
#include "pciehdevices_impl.h"

#define PCI_DEVICE_ID_PENSANDO_PCIESTRESS 0x8001

static int
pciestress_bars(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehbars_t *pbars;
    pciehbar_t pbar;
    pciehbarreg_t preg;
    u_int64_t sz = 0x1000;
    u_int64_t pa = 0x13f000000 + (sz * res->port);
    prt_t prt;

    pbars = pciehbars_new();

    /* mem64 bar */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    pbar.size = sz * 4; /* 4 pages with different properties */
    pbar.cfgidx = 0;

    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                res->port,
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
                res->port,
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

    pciehdev_set_bars(pdev, pbars);
    return 0;
}

static int
pciestress_cfg(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_PCIESTRESS);
    pciehcfg_setconf_classcode(pcfg, 0xff0000); /* unclassified device */
    pciehcfg_setconf_dsn(pcfg, res->dsn);

    pciehcfg_sethdr_type0(pcfg, pbars);
    pciehcfg_add_standard_caps(pcfg);
    pciehdev_set_cfg(pdev, pcfg);
    return 0;
}

static int
pciestress_initpf(pciehdev_t *pfdev, const pciehdev_res_t *pfres)
{
    if (pciestress_bars(pfdev, pfres) < 0) {
        return -1;
    }
    if (pciestress_cfg(pfdev, pfres) < 0) {
        return -1;
    }
    return 0;
}

static pciehdevice_t pciestress_device = {
    .type = PCIEHDEVICE_PCIESTRESS,
    .initpf = pciestress_initpf,
};
PCIEHDEVICE_REGISTER(pciestress_device);
