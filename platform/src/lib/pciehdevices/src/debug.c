/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>

#include "pci_ids.h"
#include "misc.h"
#include "pciehost.h"
#include "pciehdevices.h"

#define PCI_DEVICE_ID_PENSANDO_DEBUG 0x8000

static void
init_bars(pciehbars_t *pbars, const pciehdevice_resources_t *pres)
{
    struct { u_int64_t sz, pa; } defaults[3] = {
        [0] = { 64 * 1024 * 1024, 0x80000000 },
    };
    int i;

    /*
     * Take bar config from env:
     *     DEBUG_BAR0_SZ
     *     DEBUG_BAR0_PA
     */

    for (i = 0; i < 3; i++) {
        char env_sz[] = "DEBUG_BARx_SZ";
        char env_pa[] = "DEBUG_BARx_PA";
        char *env;
        u_int64_t sz, pa;

        sz = defaults[i].sz;
        pa = defaults[i].pa;

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

            memset(&pbar, 0, sizeof(pbar));
            pbar.type = PCIEHBARTYPE_MEM64;
            {
                memset(&preg, 0, sizeof(preg));
                preg.regtype = PCIEHBARREGT_RES;
                preg.flags = PCIEHBARREGF_RW;
                preg.paddr = pa;
                preg.size = sz;
                pciehbar_add_reg(&pbar, &preg);
            }
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
