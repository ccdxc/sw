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
#include <linux/io.h>
#include <linux/platform_device.h>

#include "ionic.h"
#include "ionic_lif.h"
#include "ionic_debugfs.h"

int ionic_bus_get_irq(struct ionic *ionic, unsigned int num)
{
	return platform_get_irq(ionic->pfdev, num);;
}

const char *ionic_bus_info(struct ionic *ionic)
{
	return ionic->pfdev->name;
}

int ionic_bus_alloc_irq_vectors(struct ionic *ionic, unsigned int nintrs)
{
	return nintrs;
}

void ionic_bus_free_irq_vectors(struct ionic *ionic)
{
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

	/* Setup platform device
	 */

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
	ionic_debugfs_del_dev(ionic);
	platform_set_drvdata(pfdev, NULL);

	return err;
}

static int ionic_remove(struct platform_device *pfdev)
{
	struct ionic *ionic = platform_get_drvdata(pfdev);

	if (ionic) {
		ionic_debugfs_del_dev(ionic);
		ionic_lifs_unregister(ionic);
		ionic_lifs_deinit(ionic);
		ionic_lifs_free(ionic);
		ionic_bus_free_irq_vectors(ionic);
		ionic_forget_identity(ionic);
		ionic_unmap_bars(ionic);
	}

	return 0;
}

static struct platform_driver ionic_driver = {
	.probe = ionic_probe,
	.remove = ionic_remove,
	.driver = {
		.name = "ionic-mnic",
	},
};

#ifndef FOR_TESTING_MNIC
static struct resource ionic_resources[] = {
	{
		.start = 0xfe800000,
		.end   = 0xfe807fff,
		.flags = IORESOURCE_MEM,
		.name  = "ionic-mnic-BAR0",
	},
	{
		.start = 0xfe808000,
		.end   = 0xfe808fff,
		.flags = IORESOURCE_MEM,
		.name  = "ionic-mnic-BAR1",
	},
	{
		.start = 32,
		.end   = 32,
		.flags = IORESOURCE_IRQ,
		.name  = "ionic-mnic-irq0",
	},
	{
		.start = 33,
		.end   = 33,
		.flags = IORESOURCE_IRQ,
		.name  = "ionic-mnic-irq1",
	},
	{
		.start = 34,
		.end   = 34,
		.flags = IORESOURCE_IRQ,
		.name  = "ionic-mnic-irq2",
	},
};

static u64 ionic_dma_mask = DMA_BIT_MASK(64);

static struct platform_device ionic_device = {
	.name          = "ionic-mnic",
	.resource      = ionic_resources,
	.num_resources = ARRAY_SIZE(ionic_resources),
	.dev = {
		.dma_mask = &ionic_dma_mask,
		.coherent_dma_mask = DMA_BIT_MASK(64),
	},
};
#endif

int ionic_bus_register_driver(void)
{
#ifndef FOR_TESTING_MNIC
	int err;

	err = platform_device_register(&ionic_device);
	if (err)
		return err;
#endif
	return platform_driver_register(&ionic_driver);
}

void ionic_bus_unregister_driver(void)
{
	platform_driver_unregister(&ionic_driver);
#ifndef FOR_TESTING_MNIC
	platform_device_unregister(&ionic_device);
#endif
}
