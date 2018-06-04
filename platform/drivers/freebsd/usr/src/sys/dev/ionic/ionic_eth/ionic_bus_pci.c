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

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/pci.h>

#include "ionic.h"
#include "ionic_lif.h"
#include "ionic_bus.h"

// TODO move PCI_VENDOR_ID_PENSANDO to include/linux/pci_ids.h
#define PCI_VENDOR_ID_PENSANDO			0x1dd8

#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_PF	0x1002
#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_VF	0x1003
#define PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT	0x1004

/* Supported devices */
static const struct pci_device_id ionic_id_table[] = {
	{ PCI_VDEVICE(PENSANDO, PCI_DEVICE_ID_PENSANDO_IONIC_ETH_PF) },
	{ PCI_VDEVICE(PENSANDO, PCI_DEVICE_ID_PENSANDO_IONIC_ETH_VF) },
	{ PCI_VDEVICE(PENSANDO, PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT) },
	{ 0, }	/* end of table */
};
MODULE_DEVICE_TABLE(pci, ionic_id_table);

int ionic_bus_get_irq(struct ionic *ionic, unsigned int num)
{
	return ionic->pdev->dev.msix + num;
}

const char *ionic_bus_info(struct ionic *ionic)
{
	return pci_name(ionic->pdev);
}

int ionic_bus_alloc_irq_vectors(struct ionic *ionic, unsigned int nintrs)
{
	struct resource_list_entry *rle;
	int avail, ret;

	avail = pci_msix_count(ionic->pdev->dev.bsddev);
	dev_err(ionic->dev, "count nintrs %u avail %u\n", nintrs, avail);
	if (avail < nintrs)
		return -EINVAL;

	avail = nintrs;

	ret = -pci_alloc_msix(ionic->pdev->dev.bsddev, &avail);
	dev_err(ionic->dev, "try alloc nintrs %u avail %u ret %u\n", nintrs, avail, ret);
	if (ret)
		return ret;

	rle = linux_pci_get_rle(ionic->pdev, SYS_RES_IRQ, 1);
	ionic->pdev->dev.msix = rle->start;
	ionic->pdev->dev.msix_max = rle->start + avail;

	return avail;
}

void ionic_bus_free_irq_vectors(struct ionic *ionic)
{
	pci_release_msi(ionic->pdev->dev.bsddev);
}

static int ionic_map_bars(struct ionic *ionic)
{
	struct pci_dev *pdev = ionic->pdev;
	struct device *dev = ionic->dev;
	struct ionic_dev_bar *bars = ionic->bars;
	unsigned int i, j;

	ionic->num_bars = 0;
	for (i = 0, j = 0; i < IONIC_BARS_MAX; i++) {
		if (!(pci_resource_flags(pdev, i) & IORESOURCE_MEM))
			continue;
		bars[j].bus_addr = pci_resource_start(pdev, i);
		bars[j].len = pci_resource_len(pdev, i);
		bars[j].vaddr = ioremap(bars[j].bus_addr, bars[j].len);
		if (!bars[j].vaddr) {
			dev_err(dev, "Cannot memory-map BAR %d, aborting\n", j);
			return -ENODEV;
		}
		ionic->num_bars++;
		j++;
	}

	return 0;
}

static void ionic_unmap_bars(struct ionic *ionic)
{
	struct ionic_dev_bar *bars = ionic->bars;
	unsigned int i;

	for (i = 0; i < IONIC_BARS_MAX; i++)
		if (bars[i].vaddr)
			iounmap(bars[i].vaddr);
}

static int ionic_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct device *dev = &pdev->dev;
	struct ionic *ionic;
	int err;

	ionic = kzalloc(sizeof(*ionic), GFP_KERNEL);
	if (!ionic)
		return -ENOMEM;

	ionic->pdev = pdev;
	pci_set_drvdata(pdev, ionic);
	ionic->dev = dev;

	err = ionic_set_dma_mask(ionic);
	if (err) {
		dev_err(dev, "Cannot set DMA mask, aborting\n");
		return err;
	}

	/* Setup PCI device
	 */

	err = pci_enable_io(pdev->dev.bsddev, SYS_RES_MEMORY);
	if (err) {
		dev_err(dev, "Cannot enable PCI device, aborting\n");
		goto err_out_del_dev;
	}

	err = pci_request_regions(pdev, DRV_NAME);
	if (err) {
		dev_err(dev, "Cannot request PCI regions, aborting\n");
		goto err_out_disable_device;
	}

	pci_set_master(pdev);

	err = ionic_map_bars(ionic);
	if (err)
		goto err_out_unmap_bars;

	/* Discover ionic dev resources
	 */

	err = ionic_setup(ionic);
	if (err) {
		dev_err(dev, "Cannot setup device, aborting\n");
		goto err_out_unmap_bars;
	}

	err = ionic_reset(ionic);
	if (err) {
		dev_err(dev, "Cannot reset device, aborting\n");
		goto err_out_unmap_bars;
	}

	err = ionic_identify(ionic);
	if (err) {
		dev_err(dev, "Cannot identify device, aborting\n");
		goto err_out_unmap_bars;
	}

	dev_info(dev, "ASIC %s rev 0x%X serial num %s fw version %s\n",
		 ionic_dev_asic_name(ionic->ident->dev.asic_type),
		 ionic->ident->dev.asic_rev, ionic->ident->dev.serial_num,
		 ionic->ident->dev.fw_version);

	/* Allocate and init LIFs, creating a netdev per LIF
	 */

	err = ionic_lifs_size(ionic);
	if (err) {
		dev_err(dev, "Cannot size LIFs, aborting\n");
		goto err_out_forget_identity;
	}

	err = ionic_lifs_alloc(ionic);
	if (err) {
		dev_err(dev, "Cannot allocate LIFs, aborting\n");
		goto err_out_free_lifs;
	}

	err = ionic_lifs_init(ionic);
	if (err) {
		dev_err(dev, "Cannot init LIFs, aborting\n");
		goto err_out_deinit_lifs;
	}

	err = ionic_lifs_register(ionic);
	if (err) {
		dev_err(dev, "Cannot register LIFs, aborting\n");
		goto err_out_deinit_lifs;
	}

	return 0;

err_out_deinit_lifs:
	ionic_lifs_deinit(ionic);
err_out_free_lifs:
	ionic_lifs_free(ionic);
	ionic_bus_free_irq_vectors(ionic);
err_out_forget_identity:
	ionic_forget_identity(ionic);
err_out_unmap_bars:
	ionic_unmap_bars(ionic);
	pci_release_regions(pdev);
err_out_disable_device:
	pci_disable_device(pdev);
err_out_del_dev:
	pci_set_drvdata(pdev, NULL);

	return err;
}

static void ionic_remove(struct pci_dev *pdev)
{
	struct ionic *ionic = pci_get_drvdata(pdev);

	if (ionic) {
		ionic_lifs_unregister(ionic);
		ionic_lifs_deinit(ionic);
		ionic_lifs_free(ionic);
		ionic_bus_free_irq_vectors(ionic);
		ionic_forget_identity(ionic);
		ionic_unmap_bars(ionic);
		pci_release_regions(pdev);
		pci_disable_device(pdev);
		kfree(ionic);
	}
}

static struct pci_driver ionic_driver = {
	.name = DRV_NAME,
	.id_table = ionic_id_table,
	.probe = ionic_probe,
	.remove = ionic_remove,
};

int ionic_bus_register_driver(void)
{
	return pci_register_driver(&ionic_driver);
}

void ionic_bus_unregister_driver(void)
{
	pci_unregister_driver(&ionic_driver);
}
