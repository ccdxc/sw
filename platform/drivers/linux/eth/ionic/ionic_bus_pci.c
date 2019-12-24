// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/pci.h>

#include "ionic.h"
#include "ionic_bus.h"
#include "ionic_lif.h"
#include "ionic_debugfs.h"

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
#ifdef HAVE_PCI_IRQ_API
	return pci_irq_vector(ionic->pdev, num);
#else
	return ionic->msix[num].vector;
#endif
}

const char *ionic_bus_info(struct ionic *ionic)
{
	return pci_name(ionic->pdev);
}

int ionic_bus_alloc_irq_vectors(struct ionic *ionic, unsigned int nintrs)
{
#ifdef HAVE_PCI_IRQ_API
	return pci_alloc_irq_vectors(ionic->pdev, nintrs, nintrs,
				     PCI_IRQ_MSIX);
#else
	int err;
	int i;

	if (ionic->msix)
		return -EBUSY;

	ionic->msix = devm_kzalloc(ionic->dev,
				   sizeof(*ionic->msix) * nintrs, GFP_KERNEL);
	if (!ionic->msix)
		return -ENOMEM;
	for (i = 0; i < nintrs; i++)
		ionic->msix[i].entry = i;
	err = pci_enable_msix_exact(ionic->pdev, ionic->msix, nintrs);
	if (err < 0) {
		devm_kfree(ionic->dev, ionic->msix);
		ionic->msix = NULL;
		return err;
	}
	return nintrs;
#endif
}

void ionic_bus_free_irq_vectors(struct ionic *ionic)
{
#ifdef HAVE_PCI_IRQ_API
	pci_free_irq_vectors(ionic->pdev);
#else
	pci_disable_msix(ionic->pdev);
	devm_kfree(ionic->dev, ionic->msix);
	ionic->msix = NULL;
#endif
}

struct net_device *ionic_alloc_netdev(struct ionic *ionic)
{
	struct ionic_lif *lif;
	int nqueues;

	/* Create a netdev big enough to handle all the queues
	 * needed for lif0 and any macvlan slave lifs.
	 */
	nqueues = ionic->ntxqs_per_lif + (ionic->nlifs - 1);
	dev_dbg(ionic->dev, "nxqs=%d nlifs=%d nqueues=%d nintrs=%d\n",
		ionic->ntxqs_per_lif, ionic->nlifs,
		nqueues, ionic->nintrs);

	return alloc_etherdev_mqs(sizeof(*lif), nqueues, nqueues);
}

static int ionic_map_bars(struct ionic *ionic)
{
	struct pci_dev *pdev = ionic->pdev;
	struct device *dev = ionic->dev;
	struct ionic_dev_bar *bars;
	unsigned int i, j;

	bars = ionic->bars;
	ionic->num_bars = 0;

	for (i = 0, j = 0; i < IONIC_BARS_MAX; i++) {
		if (!(pci_resource_flags(pdev, i) & IORESOURCE_MEM))
			continue;
		bars[j].len = pci_resource_len(pdev, i);

		/* only map the whole bar 0 */
		if (j > 0) {
			bars[j].vaddr = NULL;
		} else {
			bars[j].vaddr = pci_iomap(pdev, i, bars[j].len);
			if (!bars[j].vaddr) {
				dev_err(dev,
					"Cannot memory-map BAR %d, aborting\n",
					i);
				return -ENODEV;
			}
		}

		bars[j].bus_addr = pci_resource_start(pdev, i);
		bars[j].res_index = i;
		ionic->num_bars++;
		j++;
	}

	ionic_debugfs_add_bars(ionic);

	return 0;
}

static void ionic_unmap_bars(struct ionic *ionic)
{
	struct ionic_dev_bar *bars = ionic->bars;
	unsigned int i;

	for (i = 0; i < IONIC_BARS_MAX; i++) {
		if (bars[i].vaddr) {
			iounmap(bars[i].vaddr);
			bars[i].bus_addr = 0;
			bars[i].vaddr = NULL;
			bars[i].len = 0;
		}
	}
}

void __iomem *ionic_bus_map_dbpage(struct ionic *ionic, int page_num)
{
#ifdef HAVE_PCI_IOMAP_RANGE
	return pci_iomap_range(ionic->pdev,
			       ionic->bars[IONIC_PCI_BAR_DBELL].res_index,
			       (u64)page_num << PAGE_SHIFT, PAGE_SIZE);
#else
	int bar = ionic->bars[IONIC_PCI_BAR_DBELL].res_index;
	phys_addr_t start = pci_resource_start(ionic->pdev, bar);
	phys_addr_t offset = start + ((phys_addr_t)page_num << PAGE_SHIFT);

	return ioremap(offset, PAGE_SIZE);
#endif /* HAVE_PCI_IOMAP_RANGE */
}

void ionic_bus_unmap_dbpage(struct ionic *ionic, void __iomem *page)
{
	iounmap(page);
}

phys_addr_t ionic_bus_phys_dbpage(struct ionic *ionic, int page_num)
{
	return ionic->bars[IONIC_PCI_BAR_DBELL].bus_addr +
		((phys_addr_t)page_num << PAGE_SHIFT);
}

static int ionic_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct device *dev = &pdev->dev;
	struct ionic *ionic;
	int err;

	ionic = ionic_devlink_alloc(dev);
	if (!ionic)
		return -ENOMEM;

	ionic->pdev = pdev;
	ionic->dev = dev;
	pci_set_drvdata(pdev, ionic);
	mutex_init(&ionic->dev_cmd_lock);

	ionic->is_mgmt_nic =
		ent->device == PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT;
	ionic->pfdev = NULL;

	err = ionic_set_dma_mask(ionic);
	if (err) {
		dev_err(dev, "Cannot set DMA mask: %d, aborting\n", err);
		goto err_out_clear_drvdata;
	}

	ionic_debugfs_add_dev(ionic);

	/* Setup PCI device */
	err = pci_enable_device_mem(pdev);
	if (err) {
		dev_err(dev, "Cannot enable PCI device: %d, aborting\n", err);
		goto err_out_debugfs_del_dev;
	}

	err = pci_request_regions(pdev, IONIC_DRV_NAME);
	if (err) {
		dev_err(dev, "Cannot request PCI regions: %d, aborting\n", err);
		goto err_out_pci_disable_device;
	}

	pci_set_master(pdev);

	err = ionic_map_bars(ionic);
	if (err)
		goto err_out_pci_clear_master;

	/* Configure the device */
	err = ionic_setup(ionic);
	if (err) {
		dev_err(dev, "Cannot setup device: %d, aborting\n", err);
		goto err_out_unmap_bars;
	}

	err = ionic_identify(ionic);
	if (err) {
		dev_err(dev, "Cannot identify device: %d, aborting\n", err);
		goto err_out_teardown;
	}

	err = ionic_init(ionic);
	if (err) {
		dev_err(dev, "Cannot init device: %d, aborting\n", err);
		goto err_out_teardown;
	}

	/* Configure the ports */
	err = ionic_port_identify(ionic);
	if (err) {
		dev_err(dev, "Cannot identify port: %d, aborting\n", err);
		goto err_out_reset;
	}

	err = ionic_port_init(ionic);
	if (err) {
		dev_err(dev, "Cannot init port: %d, aborting\n", err);
		goto err_out_reset;
	}

	/* Configure LIFs */
	err = ionic_lif_identify(ionic, IONIC_LIF_TYPE_CLASSIC,
				 &ionic->ident.lif);
	if (err) {
		dev_err(dev, "Cannot identify LIFs: %d, aborting\n", err);
		goto err_out_port_reset;
	}

	err = ionic_lifs_size(ionic);
	if (err) {
		dev_err(dev, "Cannot size LIFs: %d, aborting\n", err);
		goto err_out_port_reset;
	}

	err = ionic_lifs_alloc(ionic);
	if (err) {
		dev_err(dev, "Cannot allocate LIFs: %d, aborting\n", err);
		goto err_out_free_irqs;
	}

	err = ionic_lifs_init(ionic);
	if (err) {
		dev_err(dev, "Cannot init LIFs: %d, aborting\n", err);
		goto err_out_free_lifs;
	}

	if (ionic->neth_eqs) {
		err = ionic_eqs_alloc(ionic);
		if (err) {
			dev_err(dev, "Cannot allocate EQs: %d\n", err);
			ionic->neth_eqs = 0;
		} else {
			err = ionic_eqs_init(ionic);
			if (err) {
				dev_err(dev, "Cannot init EQs: %d\n", err);
				ionic_eqs_free(ionic);
				ionic->neth_eqs = 0;
			}
		}
	}

	err = ionic_lifs_init_queues(ionic);
	if (err) {
		dev_err(dev, "Cannot init LIF queues: %d, aborting\n", err);
		goto err_out_free_eqs;
	}

	err = ionic_lifs_register(ionic);
	if (err) {
		dev_err(dev, "Cannot register LIFs: %d, aborting\n", err);
		goto err_out_deinit_eqs;
	}

	err = ionic_devlink_register(ionic);
	if (err)
		dev_err(dev, "Cannot register devlink: %d\n", err);

	return 0;

err_out_deinit_eqs:
	ionic_eqs_deinit(ionic);
err_out_free_eqs:
	ionic_eqs_free(ionic);
	ionic_lifs_deinit(ionic);
err_out_free_lifs:
	ionic_lifs_free(ionic);
err_out_free_irqs:
	ionic_bus_free_irq_vectors(ionic);
err_out_port_reset:
	ionic_port_reset(ionic);
err_out_reset:
	ionic_reset(ionic);
err_out_teardown:
	ionic_dev_teardown(ionic);
err_out_unmap_bars:
	ionic_unmap_bars(ionic);
	pci_release_regions(pdev);
err_out_pci_clear_master:
	pci_clear_master(pdev);
err_out_pci_disable_device:
	pci_disable_device(pdev);
err_out_debugfs_del_dev:
	ionic_debugfs_del_dev(ionic);
err_out_clear_drvdata:
	mutex_destroy(&ionic->dev_cmd_lock);
	ionic_devlink_free(ionic);
	pci_set_drvdata(pdev, NULL);

	return err;
}

static void ionic_remove(struct pci_dev *pdev)
{
	struct ionic *ionic = pci_get_drvdata(pdev);

	if (!ionic)
		return;

	ionic_devlink_unregister(ionic);
	ionic_lifs_unregister(ionic);
	ionic_eqs_deinit(ionic);
	ionic_eqs_free(ionic);
	ionic_lifs_deinit(ionic);
	ionic_lifs_free(ionic);
	ionic_bus_free_irq_vectors(ionic);
	ionic_port_reset(ionic);
	ionic_reset(ionic);
	ionic_dev_teardown(ionic);
	ionic_unmap_bars(ionic);
	pci_release_regions(pdev);
	pci_clear_master(pdev);
	pci_disable_sriov(pdev);
	pci_disable_device(pdev);
	ionic_debugfs_del_dev(ionic);
	mutex_destroy(&ionic->dev_cmd_lock);
	ionic_devlink_free(ionic);
}

static int ionic_sriov_configure(struct pci_dev *pdev, int num_vfs)
{
	struct ionic *ionic = pci_get_drvdata(pdev);
	struct device *dev = ionic->dev;
	unsigned int size;
	int i, err = 0;

	if (!ionic_is_pf(ionic))
		return -ENODEV;

	if (num_vfs > 0) {
		err = pci_enable_sriov(pdev, num_vfs);
		if (err) {
			dev_err(&pdev->dev, "Cannot enable SRIOV, err=%d\n",
				err);
			return err;
		}

		size = sizeof(struct ionic_vf *) * num_vfs;
		ionic->vf = kzalloc(size, GFP_KERNEL);
		if (!ionic->vf) {
			pci_disable_sriov(pdev);
			return -ENOMEM;
		}

		for (i = 0; i < num_vfs; i++) {
			struct ionic_vf *v;

			v = kzalloc(sizeof(*v), GFP_KERNEL);
			if (!v) {
				err = -ENOMEM;
				num_vfs = 0;
				goto remove_vfs;
			}

			v->stats_pa = dma_map_single(dev, &v->stats,
						     sizeof(struct lif_stats),
						     DMA_FROM_DEVICE);
			if (dma_mapping_error(dev, v->stats_pa)) {
				err = -ENODEV;
				kfree(v);
				ionic->vf[i] = NULL;
				num_vfs = 0;
				goto remove_vfs;
			}

			ionic->vf[i] = v;
			ionic->num_vfs++;
		}

		return num_vfs;
	}

remove_vfs:
	if (num_vfs == 0) {
		if (err)
			dev_err(&pdev->dev, "Enable SRIOV failed: %d\n", err);

		pci_disable_sriov(pdev);

		for (i = 0; i < ionic->num_vfs; i++) {
			dma_unmap_single(dev, ionic->vf[i]->stats_pa,
					 sizeof(struct lif_stats),
					 DMA_FROM_DEVICE);
			kfree(ionic->vf[i]);
		}

		kfree(ionic->vf);
		ionic->vf = NULL;
		ionic->num_vfs = 0;
	}

	return err;
}

static struct pci_driver ionic_driver = {
	.name = IONIC_DRV_NAME,
	.id_table = ionic_id_table,
	.probe = ionic_probe,
	.remove = ionic_remove,
	.sriov_configure = ionic_sriov_configure,
};

int ionic_bus_register_driver(void)
{
	return pci_register_driver(&ionic_driver);
}

void ionic_bus_unregister_driver(void)
{
	pci_unregister_driver(&ionic_driver);
}
