// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2017 - 2019 Pensando Systems, Inc */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/msi.h>
#include <linux/interrupt.h>

#include "ionic.h"
#include "ionic_lif.h"
#include "ionic_debugfs.h"

#define INTR_MSIXCFG_STRIDE     0x10
#define INTR_FWCFG_STRIDE       0x8

typedef union intr_fwcfg_u {
	struct {
		u_int32_t function_mask:1;
		u_int32_t rsrv:31;
		u_int32_t lif:11;
		u_int32_t port_id:3;
		u_int32_t local_int:1;
		u_int32_t legacy:1;
		u_int32_t int_pin:2;
		u_int32_t rsrv2:14;
	} __attribute__((packed));
	u_int32_t w[2];
} intr_fwcfg_t;


typedef struct intr_msixcfg_s {
	u_int64_t msgaddr;
	u_int32_t msgdata;
	u_int32_t vector_ctrl;
} __attribute__((packed)) intr_msixcfg_t;

uint8_t *msix_cfg_base_addr;
uint8_t *fwcfg_base_addr;

void *intr_msixcfg_addr(const int intr)
{
	return (msix_cfg_base_addr + (intr * INTR_MSIXCFG_STRIDE));
}

void *intr_fwcfg_addr(const int intr)
{
	return (fwcfg_base_addr + (intr * INTR_FWCFG_STRIDE));
}

void intr_fwcfg(const int intr, uint32_t lif)
{
	volatile void *pa = intr_fwcfg_addr(intr);
	u64 data = (u64)((u64)1 << 46)/*local_int=1*/ | ((u64)lif << 32);

	writeq(data, pa);
}

void intr_msixcfg(const int intr, const u_int64_t msgaddr,
		  const u_int32_t msgdata, const int vctrl)
{
	volatile void *pa = intr_msixcfg_addr(intr);

	writeq(msgaddr, (pa + offsetof(intr_msixcfg_t, msgaddr)));
	writel(msgdata, (pa + offsetof(intr_msixcfg_t, msgdata)));
	writel(vctrl, (pa + offsetof(intr_msixcfg_t, vector_ctrl)));
}

int ionic_bus_get_irq(struct ionic *ionic, unsigned int num)
{
	struct msi_desc *desc;
	int i = 0;

	for_each_msi_entry(desc, ionic->dev) {
		if (i == num) {
			pr_info("[i = %d] msi_entry: %d.%d\n",
				i, desc->platform.msi_index,
				desc->irq);

			return desc->irq;
		}
		i++;
	}

	return -1; //return error if user is asking more irqs than allocated
}

const char *ionic_bus_info(struct ionic *ionic)
{
	return ionic->pfdev->name;
}

static void mnic_set_msi_msg(struct msi_desc *desc, struct msi_msg *msg)
{
	uint32_t lif;
	int ret;
	struct device_node *mnic_node = NULL;

	pr_info("[%d] %x:%x %x\n", desc->platform.msi_index,
		msg->address_hi, msg->address_lo, msg->data);

	intr_msixcfg(desc->platform.msi_index,
		     (((uint64_t)msg->address_hi << 32) | msg->address_lo),
		     msg->data, 0/*vctrl*/);

	mnic_node = of_find_node_by_name(NULL, "mnic");
	if (!mnic_node)
		pr_err("Can't find device node \"mnic\" in device tree! Can not configure interrupts\n");

	ret = of_property_read_u32_index(mnic_node, "lif", 0, &lif);
	if (ret)
		pr_err("Failed to get lif property for \"mnic\"! Can not configure interrupts\n");

	intr_fwcfg(desc->platform.msi_index, lif);
}

int ionic_bus_alloc_irq_vectors(struct ionic *ionic, unsigned int nintrs)
{

	int err = 0;

	err = platform_msi_domain_alloc_irqs(ionic->dev, nintrs,
					     mnic_set_msi_msg);
	if (err)
		return err;

	return nintrs;
}

void ionic_bus_free_irq_vectors(struct ionic *ionic)
{
	platform_msi_domain_free_irqs(ionic->dev);
}

int ionic_mnic_dev_setup(struct ionic *ionic)
{
	struct ionic_dev *idev = &ionic->idev;
	unsigned int num_bars = ionic->num_bars;
	u32 sig;

	if (num_bars < 5)
		return -EFAULT;

	idev->dev_cmd = ionic->bars[0].vaddr;
	idev->dev_cmd_db = ionic->bars[1].vaddr;
	fwcfg_base_addr = ionic->bars[2].vaddr;
	idev->intr_ctrl = ionic->bars[3].vaddr;
	msix_cfg_base_addr = ionic->bars[4].vaddr;

#ifdef HAPS
	idev->ident = ionic->bars[0].vaddr + 0x800;
#endif

	sig = ioread32(&idev->dev_cmd->signature);
	if (sig != DEV_CMD_SIGNATURE)
		return -EFAULT;

	idev->db_pages = ionic->bars[5].vaddr;
	idev->phy_db_pages = ionic->bars[5].bus_addr;

#ifdef FAKE_ADMINQ
	spin_lock_init(&ionic->cmd_lock);
	INIT_LIST_HEAD(&ionic->cmd_list);
	INIT_WORK(&ionic->cmd_work, ionic_dev_cmd_work);
#endif

	return ionic_debugfs_add_dev_cmd(ionic);
}

static int ionic_map_bars(struct ionic *ionic)
{
	struct platform_device *pfdev = ionic->pfdev;
	struct device *dev = ionic->dev;
	struct ionic_dev_bar *bars = ionic->bars;
	struct resource *res;
	unsigned int i, j;
	void *base;

	ionic->num_bars = 0;
	for (i = 0, j = 0; i < IONIC_BARS_MAX; i++) {
		res = platform_get_resource(pfdev, IORESOURCE_MEM, i);
		if (!res)
			continue;
		base = devm_ioremap_resource(dev, res);
		if (IS_ERR(base)) {
			dev_err(dev, "Cannot memory-map BAR %d, aborting\n", j);
			return -ENODEV;
		}
		bars[j].len = res->end - res->start + 1;
		bars[j].vaddr = base;
		bars[j].bus_addr = res->start;
		ionic->num_bars++;
		j++;
	}

	return ionic_debugfs_add_bars(ionic);
}

static void ionic_unmap_bars(struct ionic *ionic)
{
	struct device *dev = ionic->dev;
	struct ionic_dev_bar *bars = ionic->bars;
	unsigned int i;

	for (i = 0; i < IONIC_BARS_MAX; i++)
		if (bars[i].vaddr)
			devm_iounmap(dev, bars[i].vaddr);
}

void __iomem *ionic_bus_map_dbpage(struct ionic *ionic, int page_num)
{
	return ionic->bars[1].vaddr + (page_num << PAGE_SHIFT);
}

void ionic_bus_unmap_dbpage(struct ionic *ionic, void __iomem *page)
{
}

phys_addr_t ionic_bus_phys_dbpage(struct ionic *ionic, int page_num)
{
	return 0;
}

static int ionic_probe(struct platform_device *pfdev)
{
	struct device *dev = &pfdev->dev;
	struct ionic *ionic;
	int err;

	ionic = devm_kzalloc(dev, sizeof(*ionic), GFP_KERNEL);
	if (!ionic)
		return -ENOMEM;

	ionic->pfdev = pfdev;
	platform_set_drvdata(pfdev, ionic);
	ionic->dev = dev;

	ionic->is_mgmt_nic = true;

	err = ionic_set_dma_mask(ionic);
	if (err) {
		dev_err(dev, "Cannot set DMA mask, aborting\n");
		return err;
	}

	err = ionic_debugfs_add_dev(ionic);
	if (err) {
		dev_err(dev, "Cannot add device debugfs, aborting\n");
		return err;
	}

	/* Setup platform device */

	err = ionic_map_bars(ionic);
	if (err)
		goto err_out_unmap_bars;

	/* Discover ionic dev resources */

	err = ionic_mnic_dev_setup(ionic);
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

	dev_info(dev, "ASIC %s rev 0x%X serial num %s fw version %s txqs = %d rxqs = %d adminqs = %d nintrs = %d\n",
		 ionic_dev_asic_name(ionic->ident->dev.asic_type),
		 ionic->ident->dev.asic_rev,
		 ionic->ident->dev.serial_num,
		 ionic->ident->dev.fw_version,
		 ionic->ident->dev.tx_qtype.qid_count,
		 ionic->ident->dev.rx_qtype.qid_count,
		 ionic->ident->dev.admin_qtype.qid_count,
		 ionic->ident->dev.nintrs);

	/* Allocate and init LIFs, creating a netdev per LIF */
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
		goto err_out_unregister_lifs;
	}

	return 0;

err_out_unregister_lifs:
	ionic_lifs_unregister(ionic);
err_out_deinit_lifs:
	ionic_lifs_deinit(ionic);
err_out_free_lifs:
	ionic_lifs_free(ionic);
	ionic_bus_free_irq_vectors(ionic);
err_out_forget_identity:
	ionic_forget_identity(ionic);
err_out_unmap_bars:
	ionic_unmap_bars(ionic);
	ionic_debugfs_del_dev(ionic);
	platform_set_drvdata(pfdev, NULL);

	return err;
}

static int ionic_remove(struct platform_device *pfdev)
{
	struct ionic *ionic = platform_get_drvdata(pfdev);

	if (ionic) {
		ionic_lifs_unregister(ionic);
		ionic_lifs_deinit(ionic);
		ionic_lifs_free(ionic);
		ionic_bus_free_irq_vectors(ionic);
		ionic_forget_identity(ionic);
		ionic_unmap_bars(ionic);
		ionic_debugfs_del_dev(ionic);
	}

	return 0;
}

static const struct of_device_id mnic_of_match[] = {
		{.compatible = "pensando,ionic-mnic"},
			{/* end of table */}
};

static struct platform_driver ionic_driver = {
	.probe = ionic_probe,
	.remove = ionic_remove,
	.driver = {
		.name = "ionic-mnic",
		.owner = THIS_MODULE,
		.of_match_table = mnic_of_match,
	},
};

int ionic_bus_register_driver(void)
{
	return platform_driver_register(&ionic_driver);
}

void ionic_bus_unregister_driver(void)
{
	platform_driver_unregister(&ionic_driver);
}
