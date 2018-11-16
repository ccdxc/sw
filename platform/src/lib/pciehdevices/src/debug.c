/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "platform/include/common/pci_ids.h"
#include "platform/src/lib/misc/include/misc.h"
#include "platform/src/lib/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"

#define PCI_DEVICE_ID_PENSANDO_DEBUG 0x8000

static void
init_bars(pciehbars_t *pbars, const pciehdevice_resources_t *pres)
{
    int i;

    /*
     * Default bar config from pres->debugbar[],
     * Can override from these in env:
     *     DEBUG_BARx_SZ
     *     DEBUG_BARx_PA
     */

    for (i = 0; i < 3; i++) {
        char env_sz[] = "DEBUG_BARx_SZ";
        char env_pa[] = "DEBUG_BARx_PA";
        char *env;
        u_int64_t sz, pa;

        sz = pres->debugbar[i].barsz;
        pa = pres->debugbar[i].barpa;

        /* could override bar params from $env */
        env_sz[9] = '0' + i;
        env_pa[9] = '0' + i;
        if ((env = getenv(env_sz)) != NULL) {
            sz = strtoull_ext(env);
        }
        if ((env = getenv(env_pa)) != NULL) {
            pa = strtoull_ext(env);
        }

        if (sz) {
            pciehbarreg_t preg;
            pciehbar_t pbar;
            prt_t prt;

            memset(&pbar, 0, sizeof(pbar));
            pbar.type = PCIEHBARTYPE_MEM64;
            pbar.size = roundup_power2(sz);
            pbar.cfgidx = i * 2;

            memset(&preg, 0, sizeof(preg));
            pmt_bar_enc(&preg.pmt,
                        pres->port,
                        PMT_TYPE_MEM,
                        pbar.size,
                        sz,     /* prtsize */
                        PMT_BARF_RW);

            prt_res_enc(&prt, pa, sz, PRT_RESF_NONE);
            pciehbarreg_add_prt(&preg, &prt);
            pciehbar_add_reg(&pbar, &preg);
            pciehbars_add_bar(pbars, &pbar);
        }
    }
}

static void
init_cfg(pciehcfg_t *pcfg, pciehbars_t *pbars,
         const pciehdevice_resources_t *pres)
{
    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_DEBUG);
    pciehcfg_setconf_classcode(pcfg, 0xff0000); /* unclassified device */
    pciehcfg_setconf_nintrs(pcfg, pres->intrc);
    pciehcfg_setconf_msix_tblbir(pcfg, pciehbars_get_msix_tblbir(pbars));
    pciehcfg_setconf_msix_tbloff(pcfg, pciehbars_get_msix_tbloff(pbars));
    pciehcfg_setconf_msix_pbabir(pcfg, pciehbars_get_msix_pbabir(pbars));
    pciehcfg_setconf_msix_pbaoff(pcfg, pciehbars_get_msix_pbaoff(pbars));
    pciehcfg_setconf_dsn(pcfg, pres->dsn);
    pciehcfg_setconf_fnn(pcfg, pres->fnn);

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
pciehdev_debug_new(const char *name, const pciehdevice_resources_t *pres)
{
    pciehdev_t *pdev = pciehdev_new(name, pres);
    initialize_bars(pdev, pres);
    initialize_cfg(pdev, pres);
    return pdev;
}
