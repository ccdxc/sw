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

#include "sonic.h"
#include "sonic_lif.h"
#include "sonic_debugfs.h"

// TODO move PCI_VENDOR_ID_PENSANDO to include/linux/pci_ids.h
#define PCI_VENDOR_ID_PENSANDO			0x1dd8

#define PCI_DEVICE_ID_PENSANDO_SONIC_PF	0x1007

/* Supported devices */
static const struct pci_device_id sonic_id_table[] = {
	{ PCI_VDEVICE(PENSANDO, PCI_DEVICE_ID_PENSANDO_SONIC_PF) },
	{ 0, }	/* end of table */
};
MODULE_DEVICE_TABLE(pci, sonic_id_table);

int sonic_bus_get_irq(struct sonic *sonic, unsigned int num)
{
	return pci_irq_vector(sonic->pdev, num);
}

const char *sonic_bus_info(struct sonic *sonic)
{
	return pci_name(sonic->pdev);
}

int sonic_bus_alloc_irq_vectors(struct sonic *sonic, unsigned int nintrs)
{
	return pci_alloc_irq_vectors(sonic->pdev, nintrs, nintrs,
				     PCI_IRQ_MSIX);
}

void sonic_bus_free_irq_vectors(struct sonic *sonic)
{
	pci_free_irq_vectors(sonic->pdev);
}

static int sonic_map_bars(struct sonic *sonic)
{
	struct pci_dev *pdev = sonic->pdev;
	struct device *dev = sonic->dev;
	struct sonic_dev_bar *bars = sonic->bars;
	unsigned int i, j;

	sonic->num_bars = 0;
	for (i = 0, j = 0; i < SONIC_BARS_MAX; i++) {
		if (!(pci_resource_flags(pdev, i) & IORESOURCE_MEM))
			continue;
		bars[j].len = pci_resource_len(pdev, i);
		bars[j].vaddr = pci_iomap(pdev, i, bars[j].len);
		if (!bars[j].vaddr) {
			dev_err(dev, "Cannot memory-map BAR %d, aborting\n", j);
			return -ENODEV;
		}
		bars[j].bus_addr = pci_resource_start(pdev, i);
		sonic->num_bars++;
		j++;
	}

	return sonic_debugfs_add_bars(sonic);
}

static void sonic_unmap_bars(struct sonic *sonic)
{
	struct sonic_dev_bar *bars = sonic->bars;
	unsigned int i;

	for (i = 0; i < SONIC_BARS_MAX; i++)
		if (bars[i].vaddr)
			iounmap(bars[i].vaddr);
}

static int sonic_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct device *dev = &pdev->dev;
	struct sonic *sonic;
	int err;

	sonic = devm_kzalloc(dev, sizeof(*sonic), GFP_KERNEL);
	if (!sonic)
		return -ENOMEM;

	sonic->pdev = pdev;
	pci_set_drvdata(pdev, sonic);
	sonic->dev = dev;

	err = sonic_set_dma_mask(sonic);
	if (err) {
		dev_err(dev, "Cannot set DMA mask, aborting\n");
		return err;
	}

	err = sonic_debugfs_add_dev(sonic);
	if (err) {
		dev_err(dev, "Cannot add device debugfs, aborting\n");
		return err;
	}

	/* Setup PCI device
	 */

	err = pci_enable_device_mem(pdev);
	if (err) {
		dev_err(dev, "Cannot enable PCI device, aborting\n");
		goto err_out_debugfs_del_dev;
	}

	err = pci_request_regions(pdev, DRV_NAME);
	if (err) {
		dev_err(dev, "Cannot request PCI regions, aborting\n");
		goto err_out_disable_device;
	}

	pci_set_master(pdev);

	err = sonic_map_bars(sonic);
	if (err)
		goto err_out_unmap_bars;

	/* Discover sonic dev resources
	 */

	err = sonic_setup(sonic);
	if (err) {
		dev_err(dev, "Cannot setup device, aborting\n");
		goto err_out_unmap_bars;
	}

	err = sonic_reset(sonic);
	if (err) {
		dev_err(dev, "Cannot reset device, aborting\n");
		goto err_out_unmap_bars;
	}

	err = sonic_identify(sonic);
	if (err) {
		dev_err(dev, "Cannot identify device, aborting\n");
		goto err_out_unmap_bars;
	}

	dev_info(dev, "ASIC %s rev 0x%X serial num %s fw version %s\n",
		 sonic_dev_asic_name(sonic->ident->dev.asic_type),
		 sonic->ident->dev.asic_rev, sonic->ident->dev.serial_num,
		 sonic->ident->dev.fw_version);

	/* Allocate and init LIFs, creating a netdev per LIF
	 */

	err = sonic_lifs_size(sonic);
	if (err) {
		dev_err(dev, "Cannot size LIFs, aborting, err=%d\n", err);
		goto err_out_forget_identity;
	}

	err = sonic_lifs_alloc(sonic);
	if (err) {
		dev_err(dev, "Cannot allocate LIFs, aborting, err=%d\n", err);
		goto err_out_free_lifs;
	}

	err = sonic_lifs_init(sonic);
	if (err) {
		dev_err(dev, "Cannot init LIFs, aborting, err=%d\n", err);
		goto err_out_deinit_lifs;
	}

	err = sonic_lifs_register(sonic);
	if (err) {
		dev_err(dev, "Cannot register LIFs, aborting, err=%d\n", err);
		goto err_out_deinit_lifs;
	}

	return 0;

err_out_deinit_lifs:
	sonic_lifs_deinit(sonic);
err_out_free_lifs:
	sonic_lifs_free(sonic);
	sonic_bus_free_irq_vectors(sonic);
err_out_forget_identity:
	sonic_forget_identity(sonic);
err_out_unmap_bars:
	sonic_unmap_bars(sonic);
	pci_release_regions(pdev);
err_out_disable_device:
	pci_disable_device(pdev);
err_out_debugfs_del_dev:
	sonic_debugfs_del_dev(sonic);
	pci_set_drvdata(pdev, NULL);

	return err;
}

static void sonic_remove(struct pci_dev *pdev)
{
	struct sonic *sonic = pci_get_drvdata(pdev);

	if (sonic) {
		sonic_reset(sonic);
		sonic_debugfs_del_dev(sonic);
		sonic_lifs_unregister(sonic);
		sonic_lifs_deinit(sonic);
		sonic_lifs_free(sonic);
		sonic_bus_free_irq_vectors(sonic);
		sonic_forget_identity(sonic);
		sonic_unmap_bars(sonic);
		pci_release_regions(pdev);
		pci_disable_sriov(pdev);
		pci_disable_device(pdev);
	}
}

static int sonic_sriov_configure(struct pci_dev *pdev, int numvfs)
{
	int err;

	if (numvfs > 0) {
		err = pci_enable_sriov(pdev, numvfs);
		if (err) {
			dev_err(&pdev->dev, "Cannot enable SRIOV, err=%d\n",
				err);
			return err;
		}
	}

	if (numvfs == 0)
		pci_disable_sriov(pdev);

	return numvfs;
}

static struct pci_driver sonic_driver = {
	.name = DRV_NAME,
	.id_table = sonic_id_table,
	.probe = sonic_probe,
	.remove = sonic_remove,
	.sriov_configure = sonic_sriov_configure,
};

int sonic_bus_register_driver(void)
{
	return pci_register_driver(&sonic_driver);
}

void sonic_bus_unregister_driver(void)
{
	pci_unregister_driver(&sonic_driver);
}
