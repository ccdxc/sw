/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/param.h>

#include "storage/storage_seq_common.h"
#include "platform/pciehdevices/include/pci_ids.h"
#include "platform/misc/include/misc.h"
#include "platform/intrutils/include/intrutils.h"
#include "platform/pciemgrutils/include/pciemgrutils.h"
#include "pciehdevices.h"
#include "pciehdevices_impl.h"

static int
accel_bars(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    const u_int8_t upd[8] = {
        [STORAGE_SEQ_QTYPE_SQ] =
            PRT_UPD_SCHED_SET | PRT_UPD_PICI_PISET | PRT_UPD_PID_CHECK,
        [STORAGE_SEQ_QTYPE_NOTIFY] =
            PRT_UPD_SCHED_SET | PRT_UPD_PICI_PISET | PRT_UPD_PID_CHECK,
        [STORAGE_SEQ_QTYPE_ADMIN] =
            PRT_UPD_SCHED_SET | PRT_UPD_PICI_PISET | PRT_UPD_PID_CHECK,
    };
    pciehbars_t *pbars;

    pbars = pciehbars_new();

    add_common_resource_bar(pbars, res);
    add_common_doorbell_bar(pbars, res, upd);
    add_common_cmb_bar(pbars, res);
    add_common_rom_bar(pbars, res);

    pciehdev_set_bars(pdev, pbars);
    return 0;
}

static int
accel_cfg(pciehdev_t *pdev, const pciehdev_res_t *res)
{
    pciehcfg_t *pcfg = pciehcfg_new();
    pciehbars_t *pbars = pciehdev_get_bars(pdev);

    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_ACCEL);
    pciehcfg_setconf_classcode(pcfg, 0x120000); /* processing accelerator */
    pciehcfg_setconf_nintrs(pcfg, res->intrc);
    pciehcfg_setconf_msix_tblbir(pcfg, pciehbars_get_msix_tblbir(pbars));
    pciehcfg_setconf_msix_tbloff(pcfg, pciehbars_get_msix_tbloff(pbars));
    pciehcfg_setconf_msix_pbabir(pcfg, pciehbars_get_msix_pbabir(pbars));
    pciehcfg_setconf_msix_pbaoff(pcfg, pciehbars_get_msix_pbaoff(pbars));
    pciehcfg_setconf_dsn(pcfg, res->dsn);

    pciehcfg_sethdr_type0(pcfg, pbars);
    pciehcfg_add_standard_vpd(pcfg);
    pciehcfg_add_standard_caps(pcfg);

    pciehdev_set_cfg(pdev, pcfg);
    return 0;
}

static int
accel_initpf(pciehdev_t *pfdev, const pciehdev_res_t *pfres)
{
    if (accel_bars(pfdev, pfres) < 0) {
        return -1;
    }
    if (accel_cfg(pfdev, pfres) < 0) {
        return -1;
    }
    return 0;
}

static pciehdevice_t accel_device = {
    .type = PCIEHDEVICE_ACCEL,
    .initpf = accel_initpf,
};
PCIEHDEVICE_REGISTER(accel_device);
