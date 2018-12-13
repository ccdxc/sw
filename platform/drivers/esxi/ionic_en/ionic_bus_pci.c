/*
 * Copyright 2017-2018 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

/*
 * ionic_bus_pci.c --
 *
 * Implement PCI bus related functions for ionic_en device
 */

#include "ionic.h"

#define PCI_VENDOR_ID_PENSANDO                  0x1dd8

#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_PF     0x1002
#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_VF     0x1003
#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT   0x1004


static const struct ionic_pci_device_entry dev_tbl[] = {
        {PCI_DEVICE_ID_PENSANDO_IONIC_ETH_PF, PENSANDO_IONIC_ETH_PF, "Pensando ionic PF"},
        {PCI_DEVICE_ID_PENSANDO_IONIC_ETH_VF, PENSANDO_IONIC_ETH_VF, "Pensando ionic VF"},
        {PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT, PENSANDO_IONIC_ETH_MGMT, "Pensando ionic Mgmt"}
};


/*
 ******************************************************************************
 *
 * ionic_pci_query --
 *
 *    Query PCI Parameters
 *
 *  Parameters:
 *     priv_data - IN (device private data)
 *
 *  Results:
 *     VMK_OK  - Success.
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_pci_query(struct ionic_en_priv_data *priv_data)             // IN
{
        VMK_ReturnStatus status;
        struct ionic_en_device *en_dev = &priv_data->ionic.en_dev;

        status = vmk_DeviceGetRegistrationData(en_dev->vmk_device,
                                               (vmk_AddrCookie *) &en_dev->pci_device);
        if (status != VMK_OK) {
                ionic_err("vmk_DeviceGetRegistrationData() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = vmk_PCIQueryDeviceID(en_dev->pci_device,
                                      &en_dev->pci_device_id);
        if (status != VMK_OK) {
                ionic_err("vmk_PCIQueryDeviceID() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = vmk_PCIQueryDeviceAddr(en_dev->pci_device,
                                        &en_dev->sbdf);
        if (status != VMK_OK) {
                ionic_err("vmk_PCIQueryDeviceAddr() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = vmk_PCIQueryIOResources(en_dev->pci_device,
                                         IONIC_BARS_MAX,
                                         &en_dev->pci_resources[0]);
        if (status != VMK_OK) {
                ionic_err("vmk_PCIQueryIOResources() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = ionic_pci_dev_entry_get(en_dev->pci_device_id.deviceID,
                                         dev_tbl,
                                         ARRAY_SIZE(dev_tbl),
                                         &en_dev->dev_entry);
        if (status != VMK_OK) {
                ionic_err("ionic_PciDevEntryGet() failed, status: %s",
                          vmk_StatusToString(status));
        } 

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_bars_check --
 *
 *    Check the existence of BARS
 *
 *  Parameters:
 *     priv_data   - IN (device private data)
 *     flags       - IN (device entry flags)
 *
 *  Results:
 *     Return VMK_OK if all needed BARs exist
 *     Return VMK_NOT_FOUND if one of the BARs was missing
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

static VMK_ReturnStatus
ionic_bars_check(struct ionic_en_priv_data *priv_data,            // IN
                 vmk_uint32 flags)                                // IN
{
	struct ionic_en_device *en_dev = &priv_data->ionic.en_dev;

	if (!(flags & PCI_DEVICE_ID_PENSANDO_IONIC_ETH_PF) &&
	    !(en_dev->pci_resources[IONIC_BAR0].flags &
	    VMK_PCI_BAR_FLAGS_MEM_64_BITS)) {
                ionic_err("Missing BAR 0, exiting...");
                return VMK_NOT_FOUND;
	}

        if (!(en_dev->pci_resources[IONIC_BAR1].flags &
	    VMK_PCI_BAR_FLAGS_MEM_64_BITS)) {
                ionic_err("Missing BAR 1, exiting...");
                return VMK_NOT_FOUND;
	}

        return VMK_OK;
}


/*
 ******************************************************************************
 *
 * ionic_bars_map --
 *
 *    Map bars
 *
 *  Parameters:
 *     priv_data - IN (device private data)
 *
 *  Results:
 *     VMK_OK  - Success.
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_bars_map(struct ionic_en_priv_data *priv_data)              // IN
{
        VMK_ReturnStatus status, status1;
        struct ionic_en_device *en_dev = &priv_data->ionic.en_dev;

        status = vmk_PCIMapIOResource(priv_data->module_id,
                                      en_dev->pci_device,
                                      IONIC_BAR0,
                                      NULL,
                                      &en_dev->bars[IONIC_BAR0]);
        if (status != VMK_OK) {
                ionic_err("vmk_PCIMapIOResource() failed for BAR 0, "
                          "status: %s", vmk_StatusToString(status));
                return status;
        }
	priv_data->ionic.num_bars++;
	priv_data->ionic.bars[0].vaddr = (void *) en_dev->bars[IONIC_BAR0];
	priv_data->ionic.bars[0].bus_addr = en_dev->sbdf.bus;
	priv_data->ionic.bars[0].len  = en_dev->pci_resources[IONIC_BAR0].size;
        priv_data->ionic.bars[0].res_index = 0;

        status = vmk_PCIMapIOResource(priv_data->module_id,
                                      en_dev->pci_device,
                                      IONIC_BAR1,
                                      NULL,
                                      &en_dev->bars[IONIC_BAR1]);
        if (status != VMK_OK) {
                ionic_err("vmk_PCIMapIOResource() failed for BAR 1, "
                          "status: %s", vmk_StatusToString(status));
                goto map_bar1_err;
        }
	priv_data->ionic.num_bars++;
	priv_data->ionic.bars[1].vaddr = (void *) en_dev->bars[IONIC_BAR1];
 	priv_data->ionic.bars[1].bus_addr = en_dev->sbdf.bus;
	priv_data->ionic.bars[1].len  = en_dev->pci_resources[IONIC_BAR1].size;
        priv_data->ionic.bars[1].res_index = 1;

        return status;

map_bar1_err:
        status1 = vmk_PCIUnmapIOResource(priv_data->module_id,
                                         en_dev->pci_device,
                                         IONIC_BAR0);
        if (status1 != VMK_OK) {
                ionic_err("vmk_PCIUnmapIOResource() failed with BAR 0, "
                          "status: %s", vmk_StatusToString(status));
        }
        
        return status;
}


/*
 ******************************************************************************
 *
 * ionic_bars_unmap --
 *
 *    Unmap bars
 *
 *  Parameters:
 *     priv_data - IN (device private data)
 *
 *  Results:
 *     None
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_bars_unmap(struct ionic_en_priv_data *priv_data)                 // IN
{
        VMK_ReturnStatus status;
        struct ionic_en_device *en_dev = &priv_data->ionic.en_dev;

        status = vmk_PCIUnmapIOResource(priv_data->module_id,
                                        en_dev->pci_device,
                                        IONIC_BAR0);
        if (status != VMK_OK) {
                ionic_err("vmk_PCIUnmapIOResource() failed with BAR 0, "
                          "status: %s", vmk_StatusToString(status));
        }
        
        status = vmk_PCIUnmapIOResource(priv_data->module_id,
                                        en_dev->pci_device,
                                        IONIC_BAR1);
        if (status != VMK_OK) {
                ionic_err("vmk_PCIUnmapIOResource() failed with BAR 1, "
                          "status: %s", vmk_StatusToString(status));
        }
}


/*
 ******************************************************************************
 *
 * ionic_pci_start --
 *
 *    Init and Map PCI
 *
 *  Parameters:
 *     priv_data - IN (device private data)
 *
 *  Results:
 *     VMK_ReturnStatus
 *     PCI related functionality is turned on
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

VMK_ReturnStatus
ionic_pci_start(struct ionic_en_priv_data *priv_data)             // IN
{
        VMK_ReturnStatus status;
        struct ionic_en_device *en_dev = &priv_data->ionic.en_dev;

        status = ionic_bars_check(priv_data,
                                  en_dev->dev_entry.flags);
        if (status != VMK_OK) {
                ionic_err("ionic_bars_check() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = ionic_bars_map(priv_data);
        if (status != VMK_OK) {
                ionic_err("ionic_bars_map() failed, status: %s",
                          vmk_StatusToString(status));
                return status;
        }

        status = ionic_pci_bus_master_enable(priv_data->module_id,
                                             en_dev->pci_device);
        if (status != VMK_OK) {
                ionic_err("ionic_pci_bus_master_enable() failed, status: %s",
                          vmk_StatusToString(status));
                goto bus_master_err;
        }

        return status;

bus_master_err:
        ionic_bars_unmap(priv_data);

        return status;
}


/*
 ******************************************************************************
 *
 * ionic_pci_stop --
 *
 *    Unmap and clean PCI resources
 *
 *  Parameters:
 *     priv_data - IN (device private data)
 *
 *  Results:
 *     PCI related functionality is turned off
 *
 *  Side-effects:
 *     None
 *
 ******************************************************************************
 */

void
ionic_pci_stop(struct ionic_en_priv_data *priv_data)              // IN
{
        struct ionic_en_device *en_dev = &priv_data->ionic.en_dev;

        ionic_pci_bus_master_disable(priv_data->module_id,
                                     en_dev->pci_device);
        ionic_bars_unmap(priv_data);
}
