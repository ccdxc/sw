/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/pci.h>

#include "sonic.h"
#include "sonic_lif.h"
#include "sonic_bus.h"
#include "sonic_debugfs.h"

// TODO move PCI_VENDOR_ID_PENSANDO to include/linux/pci_ids.h
#define PCI_VENDOR_ID_PENSANDO			0x1dd8

#define PCI_DEVICE_ID_PENSANDO_BRIDGE_UPSTREAM	0x1000
#define PCI_DEVICE_ID_PENSANDO_SONIC_PF		0x1007

/* Supported devices */
static const struct pci_device_id sonic_id_table[] = {
	{ PCI_VDEVICE(PENSANDO, PCI_DEVICE_ID_PENSANDO_SONIC_PF) },
	{ 0, }	/* end of table */
};
MODULE_DEVICE_TABLE(pci, sonic_id_table);

const char *sonic_bus_info(struct sonic *sonic)
{
	return pci_name(sonic->pdev);
}


#ifndef __FreeBSD__
int sonic_bus_get_irq(struct sonic *sonic, unsigned int num)
{
#ifdef HAVE_PCI_IRQ_API
        return pci_irq_vector(sonic->pdev, num);
#else
        return sonic->msix[num].vector;
#endif

}


int sonic_bus_alloc_irq_vectors(struct sonic *sonic, unsigned int nintrs)
{

#ifdef HAVE_PCI_IRQ_API
        return pci_alloc_irq_vectors(sonic->pdev, nintrs, nintrs,
                                     PCI_IRQ_MSIX);
#else
        int err;
        int i;

        if (sonic->msix)
                return -EBUSY;

        sonic->msix = devm_kzalloc(sonic->dev,
                                   sizeof(*sonic->msix) * nintrs, GFP_KERNEL);
        if (!sonic->msix)
                return -ENOMEM;
        for (i = 0; i < nintrs; i++)
                sonic->msix[i].entry = i;
        err = pci_enable_msix_exact(sonic->pdev, sonic->msix, nintrs);
        if (err < 0) {
                devm_kfree(sonic->dev, sonic->msix);
                sonic->msix = NULL;
                return err;
        }
        return nintrs;
#endif

}

void sonic_bus_free_irq_vectors(struct sonic *sonic)
{

#ifdef HAVE_PCI_IRQ_API
        pci_free_irq_vectors(sonic->pdev);
#else
        pci_disable_msix(sonic->pdev);
        devm_kfree(sonic->dev, sonic->msix);
        sonic->msix = NULL;
#endif
}
#else
int sonic_bus_get_irq(struct sonic *sonic, unsigned int num)
{
#if __FreeBSD_version >= 1200000
	return sonic->pdev->dev.irq_start + num;
#else
	return sonic->pdev->dev.msix + num;
#endif
}

int sonic_bus_alloc_irq_vectors(struct sonic *sonic, unsigned int nintrs)
{
	struct resource_list_entry *rle;
	int avail, ret;

	avail = pci_msix_count(sonic->pdev->dev.bsddev);
	OSAL_LOG_INFO("count nintrs %u avail %u", nintrs, avail);
	if (avail < nintrs)
		return -EINVAL;

	avail = nintrs;

	ret = -pci_alloc_msix(sonic->pdev->dev.bsddev, &avail);
	OSAL_LOG_INFO("try alloc nintrs %u avail %u ret %u", nintrs, avail, ret);
	if (ret)
		return ret;

	rle = linux_pci_get_rle(sonic->pdev, SYS_RES_IRQ, 1);
#if __FreeBSD_version >= 1200000
	sonic->pdev->dev.irq_start = rle->start;
	sonic->pdev->dev.irq_end = rle->start + avail;
#else
	sonic->pdev->dev.msix = rle->start;
	sonic->pdev->dev.msix_max = rle->start + avail;
#endif

	return avail;
}

void sonic_bus_free_irq_vectors(struct sonic *sonic)
{
	pci_release_msi(sonic->pdev->dev.bsddev);
}
#endif
static int sonic_map_bars(struct sonic *sonic)
{
	struct pci_dev *pdev = sonic->pdev;
	struct sonic_dev_bar *bars = sonic->bars;
	unsigned int i, j;

	sonic->num_bars = 0;
	for (i = 0, j = 0; i < SONIC_BARS_MAX; i++) {
		if (!(pci_resource_flags(pdev, i) & IORESOURCE_MEM))
			continue;
		bars[j].bus_addr = pci_resource_start(pdev, i);
		bars[j].len = pci_resource_len(pdev, i);
#ifndef __FreeBSD__
		bars[j].vaddr = pci_iomap(pdev, i, bars[j].len);
#else
		bars[j].vaddr = ioremap(bars[j].bus_addr, bars[j].len);
#endif
		if (!bars[j].vaddr) {
			OSAL_LOG_ERROR("Cannot memory-map BAR %d, aborting",
					j);
			return -ENODEV;
		}
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

#ifdef __FreeBSD__
/*
 * Enable ExtTag on a device.
 * Probably similar to ONTAP function.
 */
static void pcie_enable_extended_tag_capability(device_t dev)
{
	uint32_t cap_off, cap, ctrl;

	pci_find_cap(dev, PCIY_EXPRESS, &cap_off);
	if (cap_off == 0) {
		device_printf(dev, "Couldn't find PCI express capability\n");
		return;
	}
	cap = pci_read_config(dev, cap_off + PCIER_DEVICE_CAP, 2);
	ctrl = pci_read_config(dev, cap_off + PCIER_DEVICE_CTL, 2);

	if ((cap & PCIEM_CAP_EXT_TAG_FIELD) && ((ctrl & PCIEM_CTL_EXT_TAG_FIELD) == 0)) {
		device_printf(dev, "ExtTag  is disabled, enabling it.\n");
		ctrl |= PCIEM_CTL_EXT_TAG_FIELD;
		pci_write_config(dev, cap_off + PCIER_DEVICE_CTL, PCIEM_CTL_EXT_TAG_FIELD, 2);
	}
}
/*
 * This is the workaround till BIOS fixes enabling Extended Tag.
 */
static void sonic_pci_exttag_workaround(struct device *dev)
{
	device_t upstream;

	/*
	 * First enable extend tag on the device itself.
	 */
	pcie_enable_extended_tag_capability(dev->bsddev);

	/*
	 * XXX: This assumes system has only one Naples card.
	 */
	upstream = pci_find_device(PCI_VENDOR_ID_PENSANDO,
		PCI_DEVICE_ID_PENSANDO_BRIDGE_UPSTREAM);

	/* In case you are running in VM. */
	if (upstream == NULL) {
		device_printf(dev->bsddev,
			"Couldn't find Pensando upstream bridge, skipping ExtTag\n");
		return;
	}

	pcie_enable_extended_tag_capability(upstream);
}
#endif

static int sonic_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct device *dev = &pdev->dev;
	struct sonic *sonic;
	int err;

	sonic = kzalloc(sizeof(*sonic), GFP_KERNEL);
	if (!sonic)
		return -ENOMEM;

	sonic->pdev = pdev;
	pci_set_drvdata(pdev, sonic);
	sonic->dev = dev;

#ifdef __FreeBSD__
	sonic_pci_exttag_workaround(dev);
#endif

	err = sonic_set_dma_mask(sonic);
	if (err) {
		OSAL_LOG_ERROR("Cannot set DMA mask, aborting");
		return err;
	}

	err = sonic_debugfs_add_dev(sonic);
	if (err) {
		OSAL_LOG_ERROR("Cannot add device debugfs, aborting");
		return err;
	}

	/* Setup PCI device
	 */
#ifndef __FreeBSD__
	err = pci_enable_device_mem(pdev);
#endif
	if (err) {
		OSAL_LOG_ERROR("Cannot enable PCI device, aborting");
		goto err_out_debugfs_del_dev;
	}

	err = pci_request_regions(pdev, DRV_NAME);
	if (err) {
		OSAL_LOG_ERROR("Cannot request PCI regions, aborting");
		goto err_out_disable_device;
	}

	pci_set_master(pdev);

	err = sonic_map_bars(sonic);
	if (err)
		goto err_out_pci_clear_master;

	/* Discover sonic dev resources
	 */

	err = sonic_setup(sonic);
	if (err) {
		OSAL_LOG_ERROR("Cannot setup device, aborting");
		goto err_out_unmap_bars;
	}

	err = sonic_reset(sonic);
	if (err) {
		OSAL_LOG_ERROR("Cannot reset device, aborting");
		goto err_out_teardown;
	}

	err = sonic_identify(sonic);
	if (err) {
		OSAL_LOG_ERROR("Cannot identify device, aborting");
		goto err_out_teardown;
	}

	OSAL_LOG_INFO("ASIC %s rev 0x%X serial num %s fw status %u fw version %s",
		 sonic_dev_asic_name(sonic->ident.info.asic_type),
		 sonic->ident.info.asic_rev, sonic->ident.info.serial_num,
		 sonic->ident.info.fw_status, sonic->ident.info.fw_version);

	/* Allocate and init LIFs, creating a netdev per LIF
	 */

	err = sonic_lifs_size(sonic);
	if (err) {
		OSAL_LOG_ERROR("Cannot size LIFs, aborting, err=%d", err);
		goto err_out_teardown;
	}

	err = sonic_lifs_alloc(sonic);
	if (err) {
		OSAL_LOG_ERROR("Cannot allocate LIFs, aborting, err=%d", err);
		goto err_out_free_irqs;
	}

	err = sonic_lifs_init(sonic);
	if (err) {
		OSAL_LOG_ERROR("Cannot init LIFs, aborting, err=%d", err);
		goto err_out_free_lifs;
	}

	err = sonic_lifs_register(sonic);
	if (err) {
		OSAL_LOG_ERROR("Cannot register LIFs, aborting, err=%d", err);
		goto err_out_deinit_lifs;
	}

	return 0;

err_out_deinit_lifs:
	sonic_lifs_deinit(sonic);
err_out_free_lifs:
	sonic_lifs_free(sonic);
err_out_free_irqs:
	sonic_bus_free_irq_vectors(sonic);
err_out_teardown:
	sonic_dev_teardown(sonic);
err_out_unmap_bars:
	sonic_unmap_bars(sonic);
	pci_release_regions(pdev);
err_out_pci_clear_master:
	pci_clear_master(pdev);
err_out_disable_device:
#ifndef __FreeBSD__
	pci_disable_device(pdev);
#endif
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
		sonic_dev_teardown(sonic);
		sonic_unmap_bars(sonic);
		pci_release_regions(pdev);
#ifndef __FreeBSD__
		pci_disable_sriov(pdev);
		pci_disable_device(pdev);
#endif
		pci_clear_master(pdev);
	}
}

#ifndef __FreeBSD__
static int sonic_sriov_configure(struct pci_dev *pdev, int numvfs)
{
	int err;

	if (numvfs > 0) {
		err = pci_enable_sriov(pdev, numvfs);
		if (err) {
			OSAL_LOG_ERROR("Cannot enable SRIOV, err=%d",
				err);
			return err;
		}
	}

	if (numvfs == 0)
		pci_disable_sriov(pdev);

	return numvfs;
}
#endif

static struct pci_driver sonic_driver = {
	.name = DRV_NAME,
	.id_table = sonic_id_table,
	.probe = sonic_probe,
	.remove = sonic_remove,
#ifndef __FreeBSD__
	.sriov_configure = sonic_sriov_configure,
#endif
};

int sonic_bus_register_driver(void)
{
	return pci_register_driver(&sonic_driver);
}

void sonic_bus_unregister_driver(void)
{
	pci_unregister_driver(&sonic_driver);
}
