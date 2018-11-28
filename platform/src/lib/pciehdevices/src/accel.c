/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/param.h>

#include "nic/include/storage_seq_common.h"
#include "platform/include/common/pci_ids.h"
#include "platform/src/lib/pciemgrutils/include/pciemgrutils.h"

#include "pciehdevices.h"
#include "pciehdevices_impl.h"

static void
initialize_bars(pciehdev_t *pdev, const pciehdevice_resources_t *pres)
{
    const u_int8_t upd[8] = {
        [STORAGE_SEQ_QTYPE_SQ] =
            PRT_UPD_SCHED_SET | PRT_UPD_PICI_PISET | PRT_UPD_PID_CHECK,
        [STORAGE_SEQ_QTYPE_ADMIN] =
            PRT_UPD_SCHED_SET | PRT_UPD_PICI_PISET | PRT_UPD_PID_CHECK,
    };
    pciehbars_t *pbars;

    pbars = pciehbars_new();

    add_common_resource_bar(pbars, pres);
    add_common_doorbell_bar(pbars, pres, upd);
    add_common_cmb_bar(pbars, pres);

    pciehdev_set_bars(pdev, pbars);
}

static void
initialize_cfg(pciehdev_t *pdev, const pciehdevice_resources_t *pres)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_ACCEL);
    pciehcfg_setconf_classcode(pcfg, 0x120000); /* processing accelerator */
    pciehcfg_setconf_nintrs(pcfg, pres->intrc);
    pciehcfg_setconf_msix_tblbir(pcfg, pciehbars_get_msix_tblbir(pbars));
    pciehcfg_setconf_msix_tbloff(pcfg, pciehbars_get_msix_tbloff(pbars));
    pciehcfg_setconf_msix_pbabir(pcfg, pciehbars_get_msix_pbabir(pbars));
    pciehcfg_setconf_msix_pbaoff(pcfg, pciehbars_get_msix_pbaoff(pbars));
    pciehcfg_setconf_dsn(pcfg, pres->dsn);
    pciehcfg_setconf_fnn(pcfg, pres->fnn);

    pciehcfg_sethdr_type0(pcfg, pbars);
    pciehcfg_add_standard_caps(pcfg);

    pciehdev_set_cfg(pdev, pcfg);
}

pciehdev_t *
pciehdev_accel_new(const char *name, const pciehdevice_resources_t *pres)
{
    pciehdev_t *pdev = pciehdev_new(name, pres);
    initialize_bars(pdev, pres);
    initialize_cfg(pdev, pres);
    return pdev;
}
