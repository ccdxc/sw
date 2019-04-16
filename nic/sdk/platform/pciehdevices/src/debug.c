/*
 * Copyright (c) 2017-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/param.h>

#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/misc/include/misc.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"
#include "pciehdevices_impl.h"

#define PCI_DEVICE_ID_PENSANDO_DEBUG 0x8000

static int
debug_bars(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehbars_t *pbars;
    int i;

    pbars = pciehbars_new();

    for (i = 0; i < 3; i++) {
        const u_int64_t barsz = res->debug.bar[i].barsz;
        const u_int64_t barpa = res->debug.bar[i].barpa;
        u_int64_t sz_to_map = barsz;

        if (sz_to_map > 0) {
            pciehbar_t pbar;
            u_int64_t pa = barpa;
            u_int64_t baroff = 0;

            memset(&pbar, 0, sizeof(pbar));
            pbar.type = PCIEHBARTYPE_MEM64;
            pbar.size = roundup_power2(barsz);
            pbar.cfgidx = i * 2;
            pbar.prefetch = res->debug.bar[i].prefetch;

            while (sz_to_map > 0) {
                pciehbarreg_t preg;
                prt_t prt;
                /* max map with a single pmt/prt is 2g */
                const u_int64_t sz = MIN(sz_to_map, 0x80000000);

                memset(&preg, 0, sizeof(preg));
                preg.baroff = baroff;
                pmt_bar_enc(&preg.pmt,
                            res->port,
                            PMT_TYPE_MEM,
                            sz,
                            sz,     /* prtsize */
                            PMTF_RW);

                prt_res_enc(&prt, pa, sz, PRT_RESF_NONE);
                pciehbarreg_add_prt(&preg, &prt);
                pciehbar_add_reg(&pbar, &preg);

                sz_to_map -= sz;
                pa += sz;
                baroff += sz;
            }
            pciehbars_add_bar(pbars, &pbar);
        }
    }

    /* just in case we want to test rom bar on this device */
    add_common_rom_bar(pbars, res);

    pciehdev_set_bars(pdev, pbars);
    return 0;
}

static int
debug_cfg(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);
    u_int16_t vendorid, deviceid;
    u_int32_t classcode;

    vendorid = PCI_VENDOR_ID_PENSANDO;
    if (res->debug.vendorid) vendorid = res->debug.vendorid;

    deviceid = PCI_DEVICE_ID_PENSANDO_DEBUG;
    if (res->debug.deviceid) deviceid = res->debug.deviceid;

    classcode = 0xff0000; /* unclassified device */
    if (res->debug.classcode) classcode = res->debug.classcode;

    pciehcfg_setconf_vendorid(pcfg, vendorid);
    pciehcfg_setconf_deviceid(pcfg, deviceid);
    pciehcfg_setconf_classcode(pcfg, classcode);
    pciehcfg_setconf_nintrs(pcfg, res->intrc);
    pciehcfg_setconf_msix_tblbir(pcfg, pciehbars_get_msix_tblbir(pbars));
    pciehcfg_setconf_msix_tbloff(pcfg, pciehbars_get_msix_tbloff(pbars));
    pciehcfg_setconf_msix_pbabir(pcfg, pciehbars_get_msix_pbabir(pbars));
    pciehcfg_setconf_msix_pbaoff(pcfg, pciehbars_get_msix_pbaoff(pbars));
    pciehcfg_setconf_dsn(pcfg, res->dsn);

    pciehcfg_sethdr_type0(pcfg, pbars);
    pciehcfg_add_standard_caps(pcfg);
    pciehdev_set_cfg(pdev, pcfg);
    return 0;
}

static pciehdevice_t debug_device = {
    .name = "debug",
    .init_bars = debug_bars,
    .init_cfg  = debug_cfg,
};
PCIEHDEVICE_REGISTER(debug_device);
