/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <sys/types.h>

#include "pci_ids.h"
#include "pciehdev.h"
#include "pciehcfg.h"

static void
bridgeup_initialize_cfg(pciehdev_t *pdev)
{
    pciehcfg_t *pcfg = pciehcfg_new();

    pciehcfg_setconf_deviceid(pcfg, PCI_DEVICE_ID_PENSANDO_BRUP);
    pciehcfg_setconf_classcode(pcfg, 0x060400); /* PCI_CLASS_BRIDGE_PCI */
    pciehcfg_setconf_bridgeup(pcfg, 1);

    pciehcfg_sethdr_type1(pcfg, NULL);
    pciehcfg_add_standard_caps(pcfg);

    pciehdev_set_cfg(pdev, pcfg);
}

pciehdev_t *
pciehdev_bridgeup_new(void)
{
    pciehdev_t *pdev = pciehdev_new("bridgeup", 0);
    bridgeup_initialize_cfg(pdev);
    return pdev;
}
