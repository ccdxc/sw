/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"
#include "pciehdevices_impl.h"

#define PCI_DEVICE_ID_PENSANDO_RCDEV 0x8888

static int
rcdev_bars(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehbars_t *pbars;
    pciehbar_t pbar;
    pciehbarreg_t preg;
    prt_t prt;
    const u_int64_t sz = 0x00100000; /* 1MB */
    const u_int64_t pa = 0; /* pa will come later from bar addr */

    pbars = pciehbars_new();

    /* bar mem64 */
    memset(&pbar, 0, sizeof(pbar));
    pbar.type = PCIEHBARTYPE_MEM64;
    pbar.size = sz;
    pbar.cfgidx = 0;

    memset(&preg, 0, sizeof(preg));
    pmt_bar_enc(&preg.pmt,
                res->port,
                PMT_TYPE_MEM,
                pbar.size,
                pbar.size, /* prtsize */
                PMTF_RW);

    prt_res_enc(&prt, pa, sz, PRT_RESF_NONE);
    pciehbarreg_add_prt(&preg, &prt);
    pciehbar_add_reg(&pbar, &preg);
    pciehbars_add_bar(pbars, &pbar);

    pciehdev_set_bars(pdev, pbars);
    return 0;
}

static int
rcdev_cfg(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_RCDEV);
    pciehcfg_setconf_classcode(pcfg, 0xff0000); /* unclassified device */

    pciehcfg_sethdr_type0(pcfg, pbars);
    pciehdev_set_cfg(pdev, pcfg);
    return 0;
}

static pciehdevice_t rcdev_device = {
    .name = "rcdev",
    .init_bars = rcdev_bars,
    .init_cfg  = rcdev_cfg,
};
PCIEHDEVICE_REGISTER(rcdev_device);
